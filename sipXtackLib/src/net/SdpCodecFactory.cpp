//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//  G.729 enabling controls.  Currently only on VxWorks and Windows
//////////////////////////////////////////////////////////////////////////////

#undef PLATFORM_SUPPORTS_G729

#if (defined(_VXWORKS) || defined(WIN32)) /* [ */
#define PLATFORM_SUPPORTS_G729
#endif /* if (defined(_VXWORKS) || defined(WIN32)) ] */

//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <string.h>

// APPLICATION INCLUDES
#include <utl/UtlDListIterator.h>
#include <os/OsWriteLock.h>
#include <os/OsReadLock.h>
#include <net/SdpCodecFactory.h>
#include <sdp/SdpCodec.h>
#include <net/NameValueTokenizer.h>

#define VERBOSE_CODEC_FACTORY
#undef VERBOSE_CODEC_FACTORY

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
SdpCodecFactory* SdpCodecFactory::spInstance = NULL;
OsBSem SdpCodecFactory::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

SdpCodecFactory* SdpCodecFactory::getSdpCodecFactory()
{
   // If the object already exists, then use it
   if (spInstance == NULL)
   {

       // If the object does not yet exist, then acquire
       // the lock to ensure that only one instance of the object is 
       // created
       sLock.acquire();
       if (spInstance == NULL)
           spInstance = new SdpCodecFactory();

       sLock.release();

   }

   return spInstance;
}


// Constructor
SdpCodecFactory::SdpCodecFactory(int numCodecs, SdpCodec* codecs[]) :
    mReadWriteMutex(OsRWMutex::Q_FIFO)
{
   mCodecCPULimit = SdpCodec::SDP_CODEC_CPU_HIGH ;
   addCodecs(numCodecs, codecs);
}

// Copy constructor
SdpCodecFactory::SdpCodecFactory(const SdpCodecFactory& rSdpCodecFactory) :
    mReadWriteMutex(OsRWMutex::Q_FIFO)
{
    *this = rSdpCodecFactory;
}

// Destructor
SdpCodecFactory::~SdpCodecFactory()
{
    mCodecs.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SdpCodecFactory& 
SdpCodecFactory::operator=(const SdpCodecFactory& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

    OsReadLock thatLock(((SdpCodecFactory&)rhs).mReadWriteMutex);
    OsWriteLock thisLock(mReadWriteMutex);
    mCodecs.destroyAll();
    UtlDListIterator iterator(((SdpCodecFactory&)rhs).mCodecs);
    const SdpCodec* codecFound;

    while((codecFound = (SdpCodec*) iterator()))
    {
        mCodecs.insert(new SdpCodec(*codecFound));
    }

    mCodecCPULimit = rhs.mCodecCPULimit;
    
   return *this;
}

void SdpCodecFactory::addCodecs(int numCodecs, SdpCodec* codecs[])
{
   OsWriteLock lock(mReadWriteMutex);
   for(int index = 0; index < numCodecs; index++)
   {
       addCodecNoLock(*(codecs[index]));
   }
}

void SdpCodecFactory::addCodec(SdpCodec& newCodec)
{
    OsWriteLock lock(mReadWriteMutex);
    addCodecNoLock(newCodec);
}

void SdpCodecFactory::bindPayloadTypes()
{
    int unusedDynamicPayloadId = 
        SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC + 1;
    SdpCodec* codecWithoutPayloadId = NULL;
    UtlString prevSubmimeType = "none";
    UtlString actualSubmimeType;

    // Find a codec which does not have its payload type set
    // Cheat a little and make the codec writable
    while((codecWithoutPayloadId = (SdpCodec*) getCodecByType(-1)))
    {
        // Find an unused dynamic payload type id
        while(getCodecByType(unusedDynamicPayloadId))
        {
            // Assuming all codecs with same submime type are stored
            // sequentially, only increment payload id for different submime types
            codecWithoutPayloadId->getEncodingName(actualSubmimeType);
            if (prevSubmimeType.compareTo(actualSubmimeType, UtlString::ignoreCase) != 0)
            {
                // Increment payload id for new submime type
                unusedDynamicPayloadId++;
                prevSubmimeType = actualSubmimeType;
            }
            else
            {
                // Just break if we have the same submime type
                break;
            }
        }

        codecWithoutPayloadId->setCodecPayloadFormat(unusedDynamicPayloadId);
    }

#ifdef VERBOSE_CODEC_FACTORY /* [ */
    {
        int count, i;
        SdpCodec** codecs;
        UtlString codecData;
        getCodecs(count, codecs);
        for (i=0; i<count; i++) {
            codecs[i]->toString(codecData);
            osPrintf("\n  SDP Codec Factory[%d]:\n %s", i, codecData.data());
        }
    }
#endif /* VERBOSE_CODEC_FACTORY ] */
}

void SdpCodecFactory::copyPayloadType(SdpCodec& codec)
{
    SdpCodec* codecFound = NULL;
    OsWriteLock lock(mReadWriteMutex);
    UtlDListIterator iterator(mCodecs);
    int newPayloadType;

    while((codecFound = (SdpCodec*) iterator()) != NULL)
    {
        if(codecFound->isSameDefinition(codec))
        {
            newPayloadType = codec.getCodecPayloadFormat();
            codecFound->setCodecPayloadFormat(newPayloadType);
        }
    }
}

void SdpCodecFactory::copyPayloadTypes(int numCodecs, 
                                       SdpCodec* codecArray[])
{
    int index;
    for(index = 0; index < numCodecs; index++)
    {
        copyPayloadType(*(codecArray[index]));
    }
}

void SdpCodecFactory::clearCodecs(void)
{
    OsWriteLock lock(mReadWriteMutex);
    mCodecs.destroyAll();
}

SdpCodec::SdpCodecTypes SdpCodecFactory::getCodecType(const char* pCodecName)
{
    SdpCodec::SdpCodecTypes retType = SdpCodec::SDP_CODEC_UNKNOWN;
    UtlString compareString(pCodecName);

    compareString.toUpper();

    if (strcmp(compareString,"TELEPHONE-EVENT") == 0 ||
       strcmp(compareString,"AUDIO/TELEPHONE-EVENT") == 0 || 
       strcmp(compareString,"128") == 0 ||
       strcmp(compareString,"AVT-TONES") == 0 ||
       strcmp(compareString,"AVT") == 0)
        retType = SdpCodec::SDP_CODEC_TONES;
    else
    if (strcmp(compareString,"PCMU") == 0 ||
       strcmp(compareString,"G711U") == 0 || 
       strcmp(compareString,"0") == 0 ||
       strcmp(compareString,"258") == 0)
        retType = SdpCodec::SDP_CODEC_GIPS_PCMU;
    else
    if (strcmp(compareString,"PCMA") == 0 ||
       strcmp(compareString,"G711A") == 0 || 
       strcmp(compareString,"8") == 0 ||
       strcmp(compareString,"257") == 0)
        retType = SdpCodec::SDP_CODEC_GIPS_PCMA;
    else
    if (strcmp(compareString,"EG711U") == 0 ||
       strcmp(compareString,"260") == 0)
        retType = SdpCodec::SDP_CODEC_GIPS_IPCMU;
    else
    if (strcmp(compareString,"EG711A") == 0 ||
       strcmp(compareString,"259") == 0)
        retType = SdpCodec::SDP_CODEC_GIPS_IPCMA;
    else
    if (strcmp(compareString,"IPCMWB") == 0)
        retType = SdpCodec::SDP_CODEC_GIPS_IPCMWB;
    else
    if (strcmp(compareString,"G729") == 0 ||
       strcmp(compareString,"G729A") == 0)
        retType = SdpCodec::SDP_CODEC_G729A;
    else
    if (strcmp(compareString,"G729AB") == 0 ||
       strcmp(compareString,"G729B") == 0)
        retType = SdpCodec::SDP_CODEC_G729AB;
    else
    if (strcmp(compareString,"G723") == 0)
        retType = SdpCodec::SDP_CODEC_G723;
    else
    if (strcmp(compareString,"G729A-FOR-CISCO-7960") == 0)
        retType = SdpCodec::SDP_CODEC_G729ACISCO7960;
    else
    if (strcmp(compareString,"ILBC") == 0)
        retType = SdpCodec::SDP_CODEC_ILBC;
    else
    if (strcmp(compareString,"GSM") == 0)
        retType = SdpCodec::SDP_CODEC_GSM;
   else
    if (strcmp(compareString,"ISAC") == 0)
        retType = SdpCodec::SDP_CODEC_GIPS_ISAC;
#ifdef HAVE_SPEEX // [
   else 
      if (strcmp(compareString,"SPEEX") == 0)
         retType = SdpCodec::SDP_CODEC_SPEEX;
   else 
      if (strcmp(compareString,"SPEEX_5") == 0)
         retType = SdpCodec::SDP_CODEC_SPEEX_5;
   else 
      if (strcmp(compareString,"SPEEX_15") == 0)
         retType = SdpCodec::SDP_CODEC_SPEEX_15;
   else 
      if (strcmp(compareString,"SPEEX_24") == 0)
         retType = SdpCodec::SDP_CODEC_SPEEX_24;
#endif // HAVE_SPEEX ]
#ifdef HAVE_GSM // [
   else 
      if (strcmp(compareString,"GSM") == 0)
         retType = SdpCodec::SDP_CODEC_GSM;
#endif // HAVE_GSM ]
   else
    if (strcmp(compareString,"VP71-CIF") == 0)
        retType = SdpCodec::SDP_CODEC_VP71_CIF;
   else
    if (strcmp(compareString,"VP71-QCIF") == 0)
        retType = SdpCodec::SDP_CODEC_VP71_QCIF;
   else
    if (strcmp(compareString,"VP71-SQCIF") == 0)
        retType = SdpCodec::SDP_CODEC_VP71_SQCIF;
   else
    if (strcmp(compareString,"VP71-QVGA") == 0)
        retType = SdpCodec::SDP_CODEC_VP71_QVGA;
   else
    if (strcmp(compareString,"IYUV-CIF") == 0)
        retType = SdpCodec::SDP_CODEC_IYUV_CIF;
   else
    if (strcmp(compareString,"IYUV-QCIF") == 0)
        retType = SdpCodec::SDP_CODEC_IYUV_QCIF;
   else
    if (strcmp(compareString,"IYUV-SQCIF") == 0)
        retType = SdpCodec::SDP_CODEC_IYUV_SQCIF;
   else
    if (strcmp(compareString,"IYUV-QVGA") == 0)
        retType = SdpCodec::SDP_CODEC_IYUV_QVGA;
   else
    if (strcmp(compareString,"I420-CIF") == 0)
        retType = SdpCodec::SDP_CODEC_I420_CIF;
   else
    if (strcmp(compareString,"I420-QCIF") == 0)
        retType = SdpCodec::SDP_CODEC_I420_QCIF;
   else
    if (strcmp(compareString,"I420-SQCIF") == 0)
        retType = SdpCodec::SDP_CODEC_I420_SQCIF;
   else
    if (strcmp(compareString,"I420-QVGA") == 0)
        retType = SdpCodec::SDP_CODEC_I420_QVGA;
   else
    if (strcmp(compareString,"H263-CIF") == 0)
        retType = SdpCodec::SDP_CODEC_H263_CIF;
   else
    if (strcmp(compareString,"H263-QCIF") == 0)
        retType = SdpCodec::SDP_CODEC_H263_QCIF;
   else
    if (strcmp(compareString,"H263-SQCIF") == 0)
        retType = SdpCodec::SDP_CODEC_H263_SQCIF;
   else
    if (strcmp(compareString,"H263-QVGA") == 0)
        retType = SdpCodec::SDP_CODEC_H263_QVGA;
   else
    if (strcmp(compareString,"RGB24-CIF") == 0)
        retType = SdpCodec::SDP_CODEC_RGB24_CIF;
   else
    if (strcmp(compareString,"RGB24-QCIF") == 0)
        retType = SdpCodec::SDP_CODEC_RGB24_QCIF;
   else
    if (strcmp(compareString,"RGB24-SQCIF") == 0)
        retType = SdpCodec::SDP_CODEC_RGB24_SQCIF;
   else
    if (strcmp(compareString,"RGB24-QVGA") == 0)
        retType = SdpCodec::SDP_CODEC_RGB24_QVGA;
    else
       retType = SdpCodec::SDP_CODEC_UNKNOWN;
    return retType;
}

#ifndef _VXWORKS /* [ */
//left this in to maintain backward compatibility

//////////////////////////////////////////////////////////////////////////////
//  WHAT?? WHY??
// While correcting compiler warnings I stumbled across something seriously
// stupid in src/net/SdpCodecFactory.cpp.  It is proof positive that too
// many cooks spoil the broth.
//
// This is NONSENSE.  It was left in due to laziness.  There is NOTHING
// platform-dependent in this code.  The original implementation was
// just plain broken, and at some point I fixed it.  Then, someone put
// it back in, rather than fixing the code that called it.
//
// The only reason it is "platform-dependent" is that this is called from
// the main program startup code, which is typically different for each
// APPLICATION; in this case, the phone (VxWorks) and the media server
// (Linus/posix) are the applications that each needed to be modified to
// call the corrected version of this routine.
//
// In fact, this routine was fixed to allow this file to be completely
// independent of the application (or "platform"), as the actual action
// of this code was now controlled by the caller, which is where any
// such variation should be isolated.
//
// Further complicating this whole thing is the fact that softphone needed
// to do something different that did the others to support the free vs.
// paid version, which differed by which codecs were supported (exactly
// the purpose of this entire class).
//
// But, instead of fixing the call to this routine to use the correct
// implementation, the decision was made to RESURRECT THE OLD BROKEN ONE.
//
// Anyway, this should be fixed in the main program of the softphone and
// the mediaserver, and then this bogus method must be deleted.
//////////////////////////////////////////////////////////////////////////////

int SdpCodecFactory::buildSdpCodecFactory(UtlString &codecList)
{
   UtlString oneCodec;
   int numRejected = 0;
   int codecStringIndex = 0;
   SdpCodec::SdpCodecTypes codecs[1];
   SdpCodec::SdpCodecTypes internalCodecId;
   NameValueTokenizer::getSubField(codecList,codecStringIndex ,
                                        ", \n\r\t", &oneCodec);
 
   while(!oneCodec.isNull())
   {
       
       internalCodecId = SdpCodecFactory::getCodecType(oneCodec.data());
       if (internalCodecId != SdpCodec::SDP_CODEC_UNKNOWN)
       {
           codecs[0] = internalCodecId;
           numRejected += buildSdpCodecFactory(1,codecs);
       }

    
       codecStringIndex++;
       NameValueTokenizer::getSubField(codecList, codecStringIndex,
                                     ", \n\r\t", &oneCodec);

   }

   return numRejected;
}
#endif/* !_VXWORKS ] */

int SdpCodecFactory::buildSdpCodecFactory(int codecCount, SdpCodec::SdpCodecTypes codecTypes[])
{

   UtlString codecMediaType("Unknown");
   UtlString codecEncodingName("Unknown");
   UtlBoolean codecValid = TRUE;
   int numRejected = 0;

   int numCodecs = 0;

   for (int loop = 0;loop < codecCount;loop++)
   {
      codecMediaType = "Unknown";
      codecEncodingName = "Unknown";
      codecValid = TRUE;

      switch(codecTypes[loop])
      {
#if 0 /* [ */

      case SdpCodec::SDP_CODEC_L16_8K: // "132"

  // NOTE:  Do not use until bug #1218 is fixed!
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_L16_8K,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_AUDIO,
                            "pcm",
                            8000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

#endif /* ] */

      case SdpCodec::SDP_CODEC_TONES:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_TONES,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_DTMF_TONES,
                            8000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_LOW);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
            // osPrintf("Codec is SDP_CODEC_TONES ");
         }
         break;

#ifdef PLATFORM_SUPPORTS_G729 /* [ */
      case SdpCodec::SDP_CODEC_G729A:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_G729A,
                            SdpCodec::SDP_CODEC_G729,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_G729A,
                            8000,
                            10000,
                            1,
                            "annexb=no",
                            SdpCodec::SDP_CODEC_CPU_HIGH,
                            SDP_CODEC_BANDWIDTH_LOW);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
            // osPrintf("Codec is SDP_CODEC_G729(A) ");
         }
         break;
      case SdpCodec::SDP_CODEC_G729AB:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_G729AB,
                            SdpCodec::SDP_CODEC_G729,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_G729AB,
                            8000,
                            10000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_HIGH,
                            SDP_CODEC_BANDWIDTH_LOW);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
            // osPrintf("Codec is SDP_CODEC_G729(B) ");
         }
         break;
      case SdpCodec::SDP_CODEC_G729ACISCO7960:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_G729A,
                            SdpCodec::SDP_CODEC_G729,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_G729ACISCO7960,
                            8000,
                            10000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_HIGH,
                            SDP_CODEC_BANDWIDTH_LOW);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
            // osPrintf("Codec is SDP_CODEC_G729(A) ");
         }
         break;
#endif /* PLATFORM_SUPPORTS_G729 ] */

      case SdpCodec::SDP_CODEC_GIPS_PCMA:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_GIPS_PCMA,
                            SdpCodec::SDP_CODEC_PCMA,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_PCMA,
                            8000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_NORMAL);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
            // osPrintf("Codec is SDP_CODEC_GIPS_PCMA ");
         }
         break;

      case SdpCodec::SDP_CODEC_GIPS_PCMU:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_GIPS_PCMU,
                            SdpCodec::SDP_CODEC_PCMU,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_PCMU,
                            8000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_NORMAL);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
            // osPrintf("Codec is SDP_CODEC_GIPS_PCMU ");
         }
         break;

//////////////////////////////////////////////////////////////////////////
// The enhanced codecs are only supported on the Xpressa

      case SdpCodec::SDP_CODEC_GIPS_IPCMA:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_GIPS_IPCMA,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_IPCMA,
                            8000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_NORMAL);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
            // osPrintf("Codec is SDP_CODEC_GIPS_IPCMA ");
         }
         break;

      case SdpCodec::SDP_CODEC_GIPS_IPCMU:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_GIPS_IPCMU,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_IPCMU,
                            8000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_NORMAL);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
            // osPrintf("Codec is SDP_CODEC_GIPS_IPCMU ");
         }
         break;

      case SdpCodec::SDP_CODEC_ILBC:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_ILBC,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_ILBC,
                            8000,
                            30000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_HIGH,
                            SDP_CODEC_BANDWIDTH_LOW);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
            // osPrintf("Codec is SDP_CODEC_ILBC ");
         }
         break;

      case SdpCodec::SDP_CODEC_GIPS_ISAC:
        {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_GIPS_ISAC,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_ISAC,
                            16000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_HIGH,
                            SDP_CODEC_BANDWIDTH_VARIABLE);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;
#ifdef HAVE_GSM // [
      case SdpCodec::SDP_CODEC_GSM:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_GSM,
                            SdpCodec::SDP_CODEC_GSM,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_GSM,
                            8000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_HIGH,
                            SDP_CODEC_BANDWIDTH_LOW);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;
#endif // HAVE_GSM ]
      case SdpCodec::SDP_CODEC_G723:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_G723,
                            SdpCodec::SDP_CODEC_G723,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_G723,
                            8000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_HIGH,
                            SDP_CODEC_BANDWIDTH_LOW); 
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_GIPS_IPCMWB:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_GIPS_IPCMWB,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_AUDIO,
                            MIME_SUBTYPE_IPCMWB,
                            16000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
            // osPrintf("Codec is SDP_CODEC_GIPS_IPCMWB ");
         }
         break;

#ifdef HAVE_SPEEX // [

      case SdpCodec::SDP_CODEC_SPEEX:
          {
              SdpCodec aCodec(SdpCodec::SDP_CODEC_SPEEX,
                  SdpCodec::SDP_CODEC_SPEEX,
                  MIME_TYPE_AUDIO,
                  MIME_SUBTYPE_SPEEX,
                  8000,
                  20000,
                  1,
                  "mode=3",
                  SdpCodec::SDP_CODEC_CPU_LOW,
                  SDP_CODEC_BANDWIDTH_LOW);
              addCodec(aCodec);
              aCodec.getMediaType(codecMediaType);
              aCodec.getEncodingName(codecEncodingName);
          }
        break;
        case SdpCodec::SDP_CODEC_SPEEX_5:
          {
              SdpCodec aCodec(SdpCodec::SDP_CODEC_SPEEX_5,
                  SdpCodec::SDP_CODEC_SPEEX_5,
                  MIME_TYPE_AUDIO,
                  MIME_SUBTYPE_SPEEX,
                  8000,
                  20000,
                  1,
                  "mode=2",
                  SdpCodec::SDP_CODEC_CPU_LOW,
                  SDP_CODEC_BANDWIDTH_LOW);
              addCodec(aCodec);
              aCodec.getMediaType(codecMediaType);
              aCodec.getEncodingName(codecEncodingName);
          }
        break;
        case SdpCodec::SDP_CODEC_SPEEX_15:
          {
              SdpCodec aCodec(SdpCodec::SDP_CODEC_SPEEX_15,
                  SdpCodec::SDP_CODEC_SPEEX_15,
                  MIME_TYPE_AUDIO,
                  MIME_SUBTYPE_SPEEX,
                  8000,
                  20000,
                  1,
                  "mode=5",
                  SdpCodec::SDP_CODEC_CPU_LOW,
                  SDP_CODEC_BANDWIDTH_NORMAL);
              addCodec(aCodec);
              aCodec.getMediaType(codecMediaType);
              aCodec.getEncodingName(codecEncodingName);
          }
        break;
        case SdpCodec::SDP_CODEC_SPEEX_24:
          {
              SdpCodec aCodec(SdpCodec::SDP_CODEC_SPEEX_24,
                  SdpCodec::SDP_CODEC_SPEEX_24,
                  MIME_TYPE_AUDIO,
                  MIME_SUBTYPE_SPEEX,
                  8000,
                  20000,
                  1,
                  "mode=7",
                  SdpCodec::SDP_CODEC_CPU_LOW,
                  SDP_CODEC_BANDWIDTH_NORMAL);
              addCodec(aCodec);
              aCodec.getMediaType(codecMediaType);
              aCodec.getEncodingName(codecEncodingName);
          }
        break;

#endif // HAVE_SPEEX ]

      case SdpCodec::SDP_CODEC_VP71_CIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_VP71_CIF,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_VP71,
                            90000,
                            20000,
                            1,
                            "size=CIF/QCIF/SQCIF",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_NORMAL,
                            SDP_VIDEO_FORMAT_CIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_VP71_QCIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_VP71_QCIF,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_VP71,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_NORMAL,
                            SDP_VIDEO_FORMAT_QCIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_VP71_SQCIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_VP71_SQCIF,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_VP71,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_NORMAL,
                            SDP_VIDEO_FORMAT_SQCIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_VP71_QVGA:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_VP71_QVGA,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_VP71,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_NORMAL,
                            SDP_VIDEO_FORMAT_QVGA);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_IYUV_CIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_IYUV_CIF,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_IYUV,
                            90000,
                            20000,
                            1,
                            "size=CIF/QCIF/SQCIF",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_CIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_IYUV_QCIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_IYUV_QCIF,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_IYUV,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_QCIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_IYUV_SQCIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_IYUV_SQCIF,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_IYUV,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_SQCIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_IYUV_QVGA:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_IYUV_QVGA,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_IYUV,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_QVGA);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_I420_CIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_I420_CIF,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_I420,
                            90000,
                            20000,
                            1,
                            "size=CIF/QCIF/SQCIF",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_CIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_I420_QCIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_I420_QCIF,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_I420,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_QCIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_I420_SQCIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_I420_SQCIF,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_I420,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_SQCIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_I420_QVGA:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_I420_QVGA,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_I420,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_QVGA);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_RGB24_CIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_RGB24_CIF,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_RGB24,
                            90000,
                            20000,
                            1,
                            "size=CIF/QCIF/SQCIF",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_CIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_RGB24_QCIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_RGB24_QCIF,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_RGB24,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_QCIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_RGB24_SQCIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_RGB24_SQCIF,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_RGB24,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_SQCIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_H263_CIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_H263_CIF,
                            SdpCodec::SDP_CODEC_H263,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_H263,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_NORMAL,
                            SDP_VIDEO_FORMAT_CIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_H263_QCIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_H263_QCIF,
                            SdpCodec::SDP_CODEC_H263,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_H263,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_NORMAL,
                            SDP_VIDEO_FORMAT_QCIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_H263_SQCIF:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_H263_SQCIF,
                            SdpCodec::SDP_CODEC_H263,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_H263,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_NORMAL,
                            SDP_VIDEO_FORMAT_SQCIF);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_H263_QVGA:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_H263_QVGA,
                            SdpCodec::SDP_CODEC_H263,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_H263,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_QVGA);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

      case SdpCodec::SDP_CODEC_RGB24_QVGA:
         {
            SdpCodec aCodec(SdpCodec::SDP_CODEC_RGB24_QVGA,
                            SdpCodec::SDP_CODEC_UNKNOWN,
                            MIME_TYPE_VIDEO,
                            MIME_SUBTYPE_RGB24,
                            90000,
                            20000,
                            1,
                            "",
                            SdpCodec::SDP_CODEC_CPU_LOW,
                            SDP_CODEC_BANDWIDTH_HIGH,
                            SDP_VIDEO_FORMAT_QVGA);
            addCodec(aCodec);
            aCodec.getMediaType(codecMediaType);
            aCodec.getEncodingName(codecEncodingName);
         }
         break;

//////////////////////////////////////////////////////////////////////////

      default:
         osPrintf("\n\n  **** Unsupported codec: %d **** \n\n",
            codecTypes[loop]);
         codecValid = FALSE;
         numRejected++;
      }

      if (codecValid) {
         numCodecs++;
#ifdef VERBOSE_CODEC_FACTORY /* [ */
         osPrintf("Using %d codecs: %d %s/%s\n", mCodecs.entries(),
                         codecTypes[loop],
                         codecMediaType.data(), codecEncodingName.data());
#endif /* ] */
      // } else {
         // osPrintf("Invalid codec selection: %d\n", internalCodecId);
      }

   }

   return numRejected;
}

// Limits the advertised codec by CPU limit level.
void SdpCodecFactory::setCodecCPULimit(int iLimit)
{
   mCodecCPULimit = iLimit ;
}
     


/* ============================ ACCESSORS ================================= */

const SdpCodec* SdpCodecFactory::getCodec(SdpCodec::SdpCodecTypes internalCodecId)
{
    UtlInt codecToMatch(internalCodecId);
    OsReadLock lock(mReadWriteMutex);
    const SdpCodec* codecFound = (SdpCodec*) mCodecs.find(&codecToMatch);

#ifdef TEST_PRINT
    if(codecFound)
    {
        UtlString codecDump;
        codecFound->toString(codecDump);
        osPrintf("SdpCodecFactory::getCodec found:\n%s",
            codecDump.data());
    }
#endif

    // Filter the codec based on CPU limit
    if ((codecFound != NULL) && (codecFound->getCPUCost() > mCodecCPULimit))
    {
        codecFound = NULL ; 
    }

    return(codecFound);
}

const SdpCodec* SdpCodecFactory::getCodecByType(int payloadTypeId)
{
    const SdpCodec* codecFound = NULL;

    OsReadLock lock(mReadWriteMutex);
    UtlDListIterator iterator(mCodecs);

    while((codecFound = (SdpCodec*) iterator()))
    {
        // If the format type matches
        if((codecFound->getCodecPayloadFormat() == payloadTypeId) && 
           (codecFound->getCPUCost() <= mCodecCPULimit))
        {
            // we found a match
            break;
        }
    }

    return(codecFound);
}

const SdpCodec* SdpCodecFactory::getCodec(const char* mimeType, 
                                          const char* mimeSubType)
{
    const SdpCodec* codecFound = NULL;
    UtlString foundMimeType;
    UtlString foundMimeSubType;
    UtlString mimeTypeString(mimeType ? mimeType : "");
    mimeTypeString.toLower();
    UtlString mimeSubTypeString(mimeSubType ? mimeSubType : "");
    mimeSubTypeString.toLower();
    OsReadLock lock(mReadWriteMutex);
    UtlDListIterator iterator(mCodecs);

    while((codecFound = (SdpCodec*) iterator()))
    {
        // If the mime type matches
        codecFound->getMediaType(foundMimeType);
        if(foundMimeType.compareTo(mimeTypeString, UtlString::ignoreCase) == 0)
        {
            // and if the mime subtype matches
            codecFound->getEncodingName(foundMimeSubType);
            if((foundMimeSubType.compareTo(mimeSubTypeString, UtlString::ignoreCase) == 0) &&
               (codecFound->getCPUCost() <= mCodecCPULimit))
            {
                // we found a match
                break;
            }
        }
    }

#ifdef TEST_PRINT
    if(codecFound)
    {
        UtlString codecDump;
        codecFound->toString(codecDump);
        osPrintf("SdpCodecFactory::getCodec found:\n%s",
            codecDump.data());
    }
#endif

    return(codecFound);
}

int SdpCodecFactory::getCodecCount()
{
    OsReadLock lock(mReadWriteMutex);
    SdpCodec* codecFound = NULL;
    
    // Find all codecs, where the CPU cost is tolerable.
    int iCount = 0;    
    UtlDListIterator iterator(mCodecs);
    while((codecFound = (SdpCodec*) iterator()))
    {
        if (codecFound->getCPUCost() <= mCodecCPULimit)
        {
            iCount++;
        }        
    }

    return iCount;
}

int SdpCodecFactory::getCodecCount(const char* mimetype)
{
    OsReadLock lock(mReadWriteMutex);
    SdpCodec* codecFound = NULL;
    UtlString foundMimeType;
    
    // Find all codecs, where the CPU cost is tolerable.
    int iCount = 0;    
    UtlDListIterator iterator(mCodecs);
    while((codecFound = (SdpCodec*) iterator()))
    {
        codecFound->getMediaType(foundMimeType);
        if (codecFound->getCPUCost() <= mCodecCPULimit && 
                foundMimeType.compareTo(mimetype, UtlString::ignoreCase) == 0)
        {
            iCount++;
        }        
    }

    return iCount;
}


void SdpCodecFactory::getCodecs(int& numCodecs, 
                                SdpCodec**& codecArray)
{
    const SdpCodec* codecFound = NULL;
    OsReadLock lock(mReadWriteMutex);
    int arrayMaximum = mCodecs.entries();
    codecArray = new SdpCodec*[arrayMaximum];
    UtlDListIterator iterator(mCodecs);
    int index = 0;

    while(index < arrayMaximum &&
          (codecFound = (SdpCodec*) iterator()) != NULL)
    {
        if (codecFound->getCPUCost() <= mCodecCPULimit)
        {
            codecArray[index] = new SdpCodec(*codecFound);
            index++;
        }
    }

    numCodecs = index;
}

void SdpCodecFactory::getCodecs(int& numCodecs, 
                                SdpCodec**& codecArray,
                                const char* mimeType)
{
    const SdpCodec* codecFound = NULL;
    OsReadLock lock(mReadWriteMutex);
    int arrayMaximum = mCodecs.entries();
    codecArray = new SdpCodec*[arrayMaximum];
    UtlDListIterator iterator(mCodecs);
    UtlString sMimeType;
    int index = 0;

    while(index < arrayMaximum &&
          (codecFound = (SdpCodec*) iterator()) != NULL)
    {
        codecFound->getMediaType(sMimeType);
        if (codecFound->getCPUCost() <= mCodecCPULimit && 
                sMimeType.compareTo(mimeType, UtlString::ignoreCase) == 0)
        {
            codecArray[index] = new SdpCodec(*codecFound);
            index++;
        }
    }

    numCodecs = index;
}

void SdpCodecFactory::getCodecs(int& numCodecs, 
                                SdpCodec**& codecArray,
                                const char* mimeType,
                                const char* subMimeType)
{
    const SdpCodec* codecFound = NULL;
    OsReadLock lock(mReadWriteMutex);
    int arrayMaximum = mCodecs.entries();
    codecArray = new SdpCodec*[arrayMaximum];
    UtlDListIterator iterator(mCodecs);
    UtlString sMimeType;
    UtlString sSubMimeType;
    int index = 0;

    while(index < arrayMaximum &&
          (codecFound = (SdpCodec*) iterator()) != NULL)
    {
        codecFound->getMediaType(sMimeType);
        codecFound->getEncodingName(sSubMimeType);
        if (    codecFound->getCPUCost() <= mCodecCPULimit && 
                sMimeType.compareTo(mimeType, UtlString::ignoreCase) == 0 && 
                sSubMimeType.compareTo(subMimeType, UtlString::ignoreCase) == 0)
        {
            codecArray[index] = new SdpCodec(*codecFound);
            index++;
        }
    }

    numCodecs = index;
}


void SdpCodecFactory::toString(UtlString& serializedFactory)
{
    serializedFactory.remove(0);
    const SdpCodec* codecFound = NULL;
    UtlDListIterator iterator(mCodecs);
    int index = 0;

    while((codecFound = (SdpCodec*) iterator()) != NULL)
    {
        UtlString codecString;
        char codecLabel[256];
        sprintf(codecLabel, "Codec[%d] cost=%d\n", index, codecFound->getCPUCost());
        serializedFactory.append(codecLabel);
        codecFound->toString(codecString);
        serializedFactory.append(codecString);
        serializedFactory.append("\n");
        index++;
    }
        
}

// Gets the codec CPU limit level
int SdpCodecFactory::getCodecCPULimit()
{
   return mCodecCPULimit;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void SdpCodecFactory::addCodecNoLock(SdpCodec& newCodec)
{
    mCodecs.insert(new SdpCodec(newCodec));

#ifdef TEST_PRINT
    UtlString codecDump;
    newCodec.toString(codecDump);
    osPrintf("SdpCodecFactory::addCodec adding:\n%s",
        codecDump.data());
#endif
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

