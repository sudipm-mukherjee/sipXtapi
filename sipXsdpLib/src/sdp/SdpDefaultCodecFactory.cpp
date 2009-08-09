// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <sdp/SdpDefaultCodecFactory.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// LOCAL TYPES DECLARATIONS
struct MpMimeInfoMapElement
{
   SdpCodec::SdpCodecTypes codecType;
   const char* mimeSubtype;
   unsigned    sampleRate;
   unsigned    numChannels;
   const char* fmtp;
};

struct MpCodecNamesMapElement
{
   SdpCodec::SdpCodecTypes codecType;
   const char* codecName;
};

// STATIC VARIABLE INITIALIZATIONS
static MpMimeInfoMapElement sgMimeInfoMap[] =
{
   { SdpCodec::SDP_CODEC_PCMA,          "pcma",    8000, 1,  NULL  },
   { SdpCodec::SDP_CODEC_PCMU,          "pcmu",    8000, 1,  NULL  },
   { SdpCodec::SDP_CODEC_GSM,           "gsm",     8000, 1,  NULL  },
   { SdpCodec::SDP_CODEC_SPEEX,         "speex",   8000, 1,  ""        },
   { SdpCodec::SDP_CODEC_SPEEX_5,       "speex",   8000, 1,  "mode=2"  },
   { SdpCodec::SDP_CODEC_SPEEX_15,      "speex",   8000, 1,  "mode=5"  },
   { SdpCodec::SDP_CODEC_SPEEX_24,      "speex",   8000, 1,  "mode=7"  },
   { SdpCodec::SDP_CODEC_SPEEX_WB,      "speex",  16000, 1,  ""        },
   { SdpCodec::SDP_CODEC_SPEEX_WB_5,    "speex",  16000, 1,  "mode=1"  },
   { SdpCodec::SDP_CODEC_SPEEX_WB_21,   "speex",  16000, 1,  "mode=6"  },
   { SdpCodec::SDP_CODEC_SPEEX_WB_42,   "speex",  16000, 1,  "mode=10"  },
   { SdpCodec::SDP_CODEC_SPEEX_UWB,     "speex",  32000, 1,  ""        },
   { SdpCodec::SDP_CODEC_SPEEX_UWB_8,   "speex",  32000, 1,  "mode=1"  },
   { SdpCodec::SDP_CODEC_SPEEX_UWB_22,  "speex",  32000, 1,  "mode=6"  },
   { SdpCodec::SDP_CODEC_SPEEX_UWB_44,  "speex",  32000, 1,  "mode=10"  },
   { SdpCodec::SDP_CODEC_ILBC,          "ilbc",    8000, 1,  "mode=30" },
   { SdpCodec::SDP_CODEC_ILBC,          "ilbc",    8000, 1,  ""        },
   { SdpCodec::SDP_CODEC_AMR_ALIGNED,   "amr",     8000, 1,  "octet-align=1" },
   { SdpCodec::SDP_CODEC_AMR,           "amr",     8000, 1,  "octet-align=0" },
   { SdpCodec::SDP_CODEC_AMR,           "amr",     8000, 1,  ""              },
   { SdpCodec::SDP_CODEC_AMRWB_ALIGNED, "amr-wb", 16000, 1,  "octet-align=1" },
   { SdpCodec::SDP_CODEC_AMRWB,         "amr-wb", 16000, 1,  "octet-align=0" },
   { SdpCodec::SDP_CODEC_AMRWB,         "amr-wb", 16000, 1,  ""              },
   { SdpCodec::SDP_CODEC_G726_16,       "g726-16", 8000, 1,  NULL },
   { SdpCodec::SDP_CODEC_G726_24,       "g726-24", 8000, 1,  NULL },
   { SdpCodec::SDP_CODEC_G726_32,       "g726-32", 8000, 1,  NULL },
   { SdpCodec::SDP_CODEC_G726_40,       "g726-40", 8000, 1,  NULL },
   { SdpCodec::SDP_CODEC_G729A,         "g729a",   8000, 1,  "annexb=no" },
   { SdpCodec::SDP_CODEC_G729A,         "g729",    8000, 1,  "annexb=no" },
   { SdpCodec::SDP_CODEC_G722,          "g722",   16000, 1,  NULL },
   { SdpCodec::SDP_CODEC_TONES,         "telephone-event", 8000, 1,   NULL  }
};
#define SIZEOF_MIME_INFO_MAP     \
   (sizeof(sgMimeInfoMap) / sizeof(sgMimeInfoMap[0]))

static MpCodecNamesMapElement sgCodecNamesMap[] =
{
   { SdpCodec::SDP_CODEC_TONES,           "TELEPHONE-EVENT" },
   { SdpCodec::SDP_CODEC_TONES,           "AUDIO/TELEPHONE-EVENT" },
   { SdpCodec::SDP_CODEC_TONES,           "AVT-TONES" },
   { SdpCodec::SDP_CODEC_TONES,           "AVT" },
   { SdpCodec::SDP_CODEC_PCMU,            "PCMU" },
   { SdpCodec::SDP_CODEC_PCMU,            "G711U" },
   { SdpCodec::SDP_CODEC_PCMA,            "PCMA" },
   { SdpCodec::SDP_CODEC_PCMA,            "G711A" },
   { SdpCodec::SDP_CODEC_G729A,           "G729" },
   { SdpCodec::SDP_CODEC_G729A,           "G729A" },
   { SdpCodec::SDP_CODEC_G729AB,          "G729B" },
   { SdpCodec::SDP_CODEC_G729AB,          "G729AB" },
   { SdpCodec::SDP_CODEC_G723,            "G723" },
   { SdpCodec::SDP_CODEC_G729ACISCO7960,  "G729A-FOR-CISCO-7960" },
   { SdpCodec::SDP_CODEC_G726_16,         "G726-16" },
   { SdpCodec::SDP_CODEC_G726_24,         "G726-24" },
   { SdpCodec::SDP_CODEC_G726_32,         "G726-32" },
   { SdpCodec::SDP_CODEC_G726_40,         "G726-40" },
   { SdpCodec::SDP_CODEC_G722,            "G722" },
   { SdpCodec::SDP_CODEC_ILBC,            "ILBC" },
   { SdpCodec::SDP_CODEC_GSM,             "GSM" },
   { SdpCodec::SDP_CODEC_SPEEX,           "SPEEX" },
   { SdpCodec::SDP_CODEC_SPEEX_5,         "SPEEX_5" },
   { SdpCodec::SDP_CODEC_SPEEX_15,        "SPEEX_15" },
   { SdpCodec::SDP_CODEC_SPEEX_24,        "SPEEX_24" },
   { SdpCodec::SDP_CODEC_SPEEX_WB,        "SPEEX_WB" },
   { SdpCodec::SDP_CODEC_SPEEX_WB_5,      "SPEEX_WB_5" },
   { SdpCodec::SDP_CODEC_SPEEX_WB_21,     "SPEEX_WB_21" },
   { SdpCodec::SDP_CODEC_SPEEX_WB_42,     "SPEEX_WB_42" },
   { SdpCodec::SDP_CODEC_SPEEX_UWB,       "SPEEX_UWB" },
   { SdpCodec::SDP_CODEC_SPEEX_UWB_8,     "SPEEX_UWB_8" },
   { SdpCodec::SDP_CODEC_SPEEX_UWB_22,    "SPEEX_UWB_22" },
   { SdpCodec::SDP_CODEC_SPEEX_UWB_44,    "SPEEX_UWB_44" },
   { SdpCodec::SDP_CODEC_AMR,             "AMR" },
   { SdpCodec::SDP_CODEC_AMR_ALIGNED,     "AMR_ALIGNED" },
   { SdpCodec::SDP_CODEC_AMRWB,           "AMRWB" },
   { SdpCodec::SDP_CODEC_AMRWB_ALIGNED,   "AMRWB_ALIGNED" },
   { SdpCodec::SDP_CODEC_GIPS_IPCMU,      "EG711U" },
   { SdpCodec::SDP_CODEC_GIPS_IPCMA,      "EG711A" },
   { SdpCodec::SDP_CODEC_GIPS_IPCMWB,     "IPCMWB" },
   { SdpCodec::SDP_CODEC_GIPS_ISAC,       "ISAC" },
   { SdpCodec::SDP_CODEC_GIPS_ISAC_LC,    "ISACLC" },
   { SdpCodec::SDP_CODEC_VP71_CIF,        "VP71-CIF" },
   { SdpCodec::SDP_CODEC_VP71_QCIF,       "VP71-QCIF" },
   { SdpCodec::SDP_CODEC_VP71_SQCIF,      "VP71-SQCIF" },
   { SdpCodec::SDP_CODEC_VP71_QVGA,       "VP71-QVGA" },
   { SdpCodec::SDP_CODEC_VP71_VGA,        "VP71-VGA" },
   { SdpCodec::SDP_CODEC_IYUV_CIF,        "IYUV-CIF" },
   { SdpCodec::SDP_CODEC_IYUV_QCIF,       "IYUV-QCIF" },
   { SdpCodec::SDP_CODEC_IYUV_SQCIF,      "IYUV-SQCIF" },
   { SdpCodec::SDP_CODEC_IYUV_QVGA,       "IYUV-QVGA" },
   { SdpCodec::SDP_CODEC_I420_CIF,        "I420-CIF" },
   { SdpCodec::SDP_CODEC_I420_QCIF,       "I420-QCIF" },
   { SdpCodec::SDP_CODEC_I420_SQCIF,      "I420-SQCIF" },
   { SdpCodec::SDP_CODEC_I420_QVGA,       "I420-QVGA" },
   { SdpCodec::SDP_CODEC_H263_CIF,        "H263-CIF" },
   { SdpCodec::SDP_CODEC_H263_QCIF,       "H263-QCIF" },
   { SdpCodec::SDP_CODEC_H263_SQCIF,      "H263-SQCIF" },
   { SdpCodec::SDP_CODEC_H263_QVGA,       "H263-QVGA" },
   { SdpCodec::SDP_CODEC_RGB24_CIF,       "RGB24-CIF" },
   { SdpCodec::SDP_CODEC_RGB24_QCIF,      "RGB24-QCIF" },
   { SdpCodec::SDP_CODEC_RGB24_SQCIF,     "RGB24-SQCIF" },
   { SdpCodec::SDP_CODEC_RGB24_QVGA,      "RGB24-QVGA" }
};
#define SIZEOF_CODEC_NAMES_MAP     \
   (sizeof(sgCodecNamesMap) / sizeof(sgCodecNamesMap[0]))

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
SdpCodec SdpDefaultCodecFactory::getCodec(SdpCodec::SdpCodecTypes internalCodecId)
{
   switch(internalCodecId)
   {

   case SdpCodec::SDP_CODEC_L16_8K:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_L16_8K,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         "pcm",
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW);
      }
      break;

   case SdpCodec::SDP_CODEC_TONES:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_TONES,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_DTMF_TONES,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;

   case SdpCodec::SDP_CODEC_G729A:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G729A,
                         SdpCodec::SDP_CODEC_G729,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G729A,
                         8000,
                         10000,
                         1,
                         "annexb=no",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_G729AB:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G729AB,
                         SdpCodec::SDP_CODEC_G729,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G729AB,
                         8000,
                         10000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_G729ACISCO7960:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G729A,
                         SdpCodec::SDP_CODEC_G729,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G729ACISCO7960,
                         8000,
                         10000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;

   case SdpCodec::SDP_CODEC_PCMA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_PCMA,
                         SdpCodec::SDP_CODEC_PCMA,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_PCMA,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_PCMU:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_PCMU,
                         SdpCodec::SDP_CODEC_PCMU,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_PCMU,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_GIPS_IPCMA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_GIPS_IPCMA,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_IPCMA,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_GIPS_IPCMU:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_GIPS_IPCMU,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_IPCMU,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_G726_16:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G726_16,
                         SdpCodec::SDP_CODEC_G726_16,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G726_16,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_G726_24:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G726_24,
                         SdpCodec::SDP_CODEC_G726_24,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G726_24,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_G726_32:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G726_32,
                         SdpCodec::SDP_CODEC_G726_32,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G726_32,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_G726_40:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G726_40,
                         SdpCodec::SDP_CODEC_G726_40,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G726_40,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_G722:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G722,
                         SdpCodec::SDP_CODEC_G722,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G722,
                         16000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_ILBC:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_ILBC,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_ILBC,
                         8000,
                         30000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;

   case SdpCodec::SDP_CODEC_GIPS_ISAC:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_GIPS_ISAC,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_ISAC,
                         16000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_VARIABLE);
      }
      break;
   case SdpCodec::SDP_CODEC_GIPS_ISAC_LC:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_GIPS_ISAC_LC,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_ISAC_LC,
                         16000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_VARIABLE);
      }
      break;
   case SdpCodec::SDP_CODEC_GSM:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_GSM,
                         SdpCodec::SDP_CODEC_GSM,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_GSM,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_G723:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G723,
                         SdpCodec::SDP_CODEC_G723,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G723,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW); 
      }
      break;

   case SdpCodec::SDP_CODEC_GIPS_IPCMWB:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_GIPS_IPCMWB,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_IPCMWB,
                         16000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH);
      }
      break;

   case SdpCodec::SDP_CODEC_SPEEX:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX,
                         SdpCodec::SDP_CODEC_SPEEX,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_SPEEX_5:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_5,
                         SdpCodec::SDP_CODEC_SPEEX_5,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         8000,
                         20000,
                         1,
                         "mode=2",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_SPEEX_15:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_15,
                         SdpCodec::SDP_CODEC_SPEEX_15,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         8000,
                         20000,
                         1,
                         "mode=5",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;
   case SdpCodec::SDP_CODEC_SPEEX_24:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_24,
                         SdpCodec::SDP_CODEC_SPEEX_24,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         8000,
                         20000,
                         1,
                         "mode=7",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_SPEEX_WB:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_WB,
                         SdpCodec::SDP_CODEC_SPEEX_WB,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         16000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_SPEEX_WB_5:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_WB_5,
                         SdpCodec::SDP_CODEC_SPEEX_WB_5,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         16000,
                         20000,
                         1,
                         "mode=1",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_SPEEX_WB_21:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_WB_21,
                         SdpCodec::SDP_CODEC_SPEEX_WB_21,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         16000,
                         20000,
                         1,
                         "mode=6",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;
   case SdpCodec::SDP_CODEC_SPEEX_WB_42:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_WB_42,
                         SdpCodec::SDP_CODEC_SPEEX_WB_42,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         16000,
                         20000,
                         1,
                         "mode=10",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_SPEEX_UWB:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_UWB,
                         SdpCodec::SDP_CODEC_SPEEX_UWB,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         32000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_SPEEX_UWB_8:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_UWB_8,
                         SdpCodec::SDP_CODEC_SPEEX_UWB_8,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         32000,
                         20000,
                         1,
                         "mode=1",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_SPEEX_UWB_22:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_UWB_22,
                         SdpCodec::SDP_CODEC_SPEEX_UWB_22,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         32000,
                         20000,
                         1,
                         "mode=6",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;
   case SdpCodec::SDP_CODEC_SPEEX_UWB_44:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_UWB_44,
                         SdpCodec::SDP_CODEC_SPEEX_UWB_44,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         32000,
                         20000,
                         1,
                         "mode=10",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_AMR:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_AMR,
                         SdpCodec::SDP_CODEC_AMR,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_AMR,
                         8000,
                         20000,
                         1,
                         "", // octet-align=0 assumed
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_AMR_ALIGNED:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_AMR_ALIGNED,
                         SdpCodec::SDP_CODEC_AMR_ALIGNED,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_AMR,
                         8000,
                         20000,
                         1,
                         "octet-align=1",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;

   case SdpCodec::SDP_CODEC_AMRWB:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_AMRWB,
                         SdpCodec::SDP_CODEC_AMRWB,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_AMRWB,
                         16000,
                         20000,
                         1,
                         "", // octet-align=0 assumed
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_AMRWB_ALIGNED:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_AMRWB_ALIGNED,
                         SdpCodec::SDP_CODEC_AMRWB_ALIGNED,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_AMRWB,
                         16000,
                         20000,
                         1,
                         "octet-align=1",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;

   case SdpCodec::SDP_CODEC_VP71_CIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_VP71_CIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_VP71_QCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_VP71_QCIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_VP71_SQCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_VP71_SQCIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_VP71_QVGA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_VP71_QVGA,
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
      }
      break;

   case SdpCodec::SDP_CODEC_VP71_VGA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_VP71_VGA,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_VP71,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL,
                         SDP_VIDEO_FORMAT_VGA);
      }
      break;

   case SdpCodec::SDP_CODEC_IYUV_CIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_IYUV_CIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_IYUV_QCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_IYUV_QCIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_IYUV_SQCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_IYUV_SQCIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_IYUV_QVGA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_IYUV_QVGA,
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
      }
      break;

   case SdpCodec::SDP_CODEC_I420_CIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_I420_CIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_I420_QCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_I420_QCIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_I420_SQCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_I420_SQCIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_I420_QVGA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_I420_QVGA,
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
      }
      break;

   case SdpCodec::SDP_CODEC_RGB24_CIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_RGB24_CIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_RGB24_QCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_RGB24_QCIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_RGB24_SQCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_RGB24_SQCIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_H263_CIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_H263_CIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_H263_QCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_H263_QCIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_H263_SQCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_H263_SQCIF,
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
      }
      break;

   case SdpCodec::SDP_CODEC_H263_QVGA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_H263_QVGA,
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
      }
      break;

   case SdpCodec::SDP_CODEC_RGB24_QVGA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_RGB24_QVGA,
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
      }
      break;

   default:
      osPrintf("\n\n  **** Unsupported codec: %d **** \n\n",
               internalCodecId);
      assert(FALSE);
      return SdpCodec();
   }
}


SdpCodec::SdpCodecTypes SdpDefaultCodecFactory::getCodecType(const char* pCodecName)
{
   UtlString compareString(pCodecName);

   for (int i = 0; i < SIZEOF_CODEC_NAMES_MAP; i++)
   {
      if (compareString.compareTo(sgCodecNamesMap[i].codecName, UtlString::ignoreCase) == 0)
      {
         return sgCodecNamesMap[i].codecType;
      }
   }

   return SdpCodec::SDP_CODEC_UNKNOWN;
}

OsStatus SdpDefaultCodecFactory::getCodecNameByType(SdpCodec::SdpCodecTypes type, UtlString& codecName)
{
   for (int i = 0; i < SIZEOF_CODEC_NAMES_MAP; i++)
   {
      if (sgCodecNamesMap[i].codecType == type)
      {
         codecName = sgCodecNamesMap[i].codecName;
         return OS_SUCCESS;
      }
   }

   assert(!"Unsupported codec type");
   codecName = "";
   return OS_NOT_FOUND;
}

OsStatus SdpDefaultCodecFactory::getMimeInfoByType(SdpCodec::SdpCodecTypes codecType,
                                                   UtlString& mimeSubtype,
                                                   UtlString& fmtp)
{
   for (int i = 0; i < SIZEOF_MIME_INFO_MAP; i++)
   {
      if (sgMimeInfoMap[i].codecType == codecType)
      {
         mimeSubtype = sgMimeInfoMap[i].mimeSubtype;
         fmtp = sgMimeInfoMap[i].fmtp;
         return OS_SUCCESS;
      }
   }

   return OS_NOT_FOUND;
}

OsStatus SdpDefaultCodecFactory::getCodecType(const UtlString &mimeSubtype,
                                              unsigned sampleRate,
                                              unsigned numChannels,
                                              const UtlString &fmtp,
                                              SdpCodec::SdpCodecTypes &codecType)
{
   for (int i = 0; i < SIZEOF_MIME_INFO_MAP; i++)
   {
      if (  mimeSubtype.compareTo(sgMimeInfoMap[i].mimeSubtype, UtlString::ignoreCase) == 0
         && sgMimeInfoMap[i].sampleRate == sampleRate
         && sgMimeInfoMap[i].numChannels == numChannels
         && fmtp.compareTo(sgMimeInfoMap[i].fmtp, UtlString::ignoreCase) == 0)
      {
         codecType = sgMimeInfoMap[i].codecType;
         return OS_SUCCESS;
      }
   }

   return OS_NOT_FOUND;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

