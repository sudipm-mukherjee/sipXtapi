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

#include "mp/MpCodecInfo.h"

MpCodecInfo::MpCodecInfo(  const char*    codecVersion,
                           unsigned       samplingRate,
                           unsigned       numBitsPerSample,
                           unsigned       numChannels,
                           unsigned       interleaveBlockSize,
                           unsigned       bitRate,
                           unsigned       minPacketBits,
                           unsigned       avgPacketBits,
                           unsigned       maxPacketBits,
                           unsigned       numSamplesPerFrame,
                           unsigned       preCodecJitterBufferSize,
                           UtlBoolean     signalingCodec,
                           UtlBoolean     doesVadCng)
 : mCodecVersion(codecVersion),
   mSamplingRate(samplingRate),
   mNumBitsPerSample(numBitsPerSample),
   mNumSamplesPerFrame( numSamplesPerFrame ),
   mNumChannels(numChannels),
   mInterleaveBlockSize(interleaveBlockSize),
   mBitRate(bitRate),
   mMinPacketBits(minPacketBits),
   mAvgPacketBits(avgPacketBits),
   mMaxPacketBits(maxPacketBits),
   mPreCodecJitterBufferSize(preCodecJitterBufferSize),
   mIsSignalingCodec(signalingCodec),
   mDoesVadCng(doesVadCng)
{
}

MpCodecInfo::~MpCodecInfo()
{
}

/* ============================ MANIPULATORS ============================== */

MpCodecInfo& MpCodecInfo::operator=(const MpCodecInfo& rMpCodecInfo)
{
   if (&rMpCodecInfo == this)
   {
      return *this;
   }

   mCodecVersion=rMpCodecInfo.mCodecVersion;
   mSamplingRate=rMpCodecInfo.mSamplingRate;
   mNumBitsPerSample=rMpCodecInfo.mNumBitsPerSample;
   mNumSamplesPerFrame=rMpCodecInfo.mNumSamplesPerFrame;
   mNumChannels=rMpCodecInfo.mNumChannels;
   mInterleaveBlockSize=rMpCodecInfo.mInterleaveBlockSize;
   mBitRate=rMpCodecInfo.mBitRate;
   mMinPacketBits=rMpCodecInfo.mMinPacketBits;
   mAvgPacketBits=rMpCodecInfo.mAvgPacketBits;
   mMaxPacketBits=rMpCodecInfo.mMaxPacketBits;
   mPreCodecJitterBufferSize=rMpCodecInfo.mPreCodecJitterBufferSize;
   mDoesVadCng=rMpCodecInfo.mDoesVadCng;

   return *this;
}

/* ============================ ACCESSORS ================================= */

UtlString MpCodecInfo::getCodecVersion(void) const
{
//Returns a string identifying the codec version
   return(mCodecVersion);
}

unsigned MpCodecInfo::getSamplingRate(void) const
{
//Returns the sampling rate for the PCM data expected by the codec
   return(mSamplingRate);
}

unsigned MpCodecInfo::getNumBitsPerSample(void) const
{
//Returns the PCM data sample size (in bits)
   return(mNumBitsPerSample);
}

unsigned MpCodecInfo::getNumSamplesPerFrame(void) const
{
//Returns the number of PCM samples per input frame for this codec
   return(mNumSamplesPerFrame);
}

unsigned MpCodecInfo::getNumChannels(void) const
{
//Returns the number of channels supported by the codec
   return(mNumChannels);
}

unsigned MpCodecInfo::getInterleaveBlockSize(void) const
{
//Returns the size of the interleave block (in samples)
// This value is not meaningful if the number of channels for the
// codec is equal to 1.
   return(mInterleaveBlockSize);
}

unsigned MpCodecInfo::getBitRate(void) const
{
//Returns the bit rate for this codec (in bits per second)
// If the codec is variable rate, then the average expected bit rate
// should be returned.
   return(mBitRate);
}

unsigned MpCodecInfo::getMinPacketBits(void) const
{
//Returns the minimum number of bits in an encoded frame
   return(mMinPacketBits);
}

unsigned MpCodecInfo::getAvgPacketBits(void) const
{
//Returns the average number of bits in an encoded frame
   return(mAvgPacketBits);
}

unsigned MpCodecInfo::getMaxPacketBits(void) const
{
//Returns the maximum number of bits in an encoded frame
   return(mMaxPacketBits);
}

unsigned MpCodecInfo::getPreCodecJitterBufferSize(void) const
{
//Returns the maximum number of bits in an encoded frame
   return(mPreCodecJitterBufferSize);
}

/* ============================ INQUIRY =================================== */


UtlBoolean MpCodecInfo::isSignalingCodec (void) const
{
//Returns TRUE if codec is used for signaling; otherwise returns FALSE
   return(mIsSignalingCodec);
}


UtlBoolean MpCodecInfo::doesVadCng (void) const
{
//Returns TRUE if codec does its own VAD and CNG; otherwise returns FALSE
   return(mDoesVadCng);
}

