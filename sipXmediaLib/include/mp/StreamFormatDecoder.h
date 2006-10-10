//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _StreamFormatDecoder_h_
#define _StreamFormatDecoder_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlString.h>

#include "mp/StreamDefs.h"
#include "os/OsStatus.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef enum                  // Format Decoder Event
{
   DecodingStartedEvent,      // Started Decoding
   DecodingUnderrunEvent,     // Supply cannot meet demand
   DecodingThrottledEvent,    // Being throttled in frame generation
   DecodingCompletedEvent,    // Completed decoding
   DecodingErrorEvent         // Error while decoding

} StreamDecoderEvent;

// FORWARD DECLARATIONS
class StreamDecoderListener;
class StreamDataSource;

//:Define an abstract Stream Format Decoder.  The Stream Format Decoder
//:pulls data from an abstract data source and decodes the data for playback
//:on an xpressa phone.
class StreamFormatDecoder
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
   StreamFormatDecoder(StreamDataSource* pDataSource);
     //:Constructs a decoder given a data source

   virtual ~StreamFormatDecoder();
     //:Destructor

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
   virtual OsStatus init() = 0 ;
     //:Initializes the decoder

   virtual OsStatus free() = 0 ;
     //:Frees all resources consumed by the decoder

   virtual OsStatus begin() = 0 ;
     //:Begins decoding

   virtual OsStatus end() = 0 ;
     //:Ends decoding

   virtual OsStatus getFrame(unsigned short* pSamples) = 0 ;
     //:Gets the next available frame.
     //!returns TRUE if a frame is available, otherwise false

   void setListener(StreamDecoderListener* pListener);
     //:Sets a listener to receive StreamDecoderEvents.

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   virtual OsStatus toString(UtlString& string) = 0 ;
     //:Renders a string describing this decoder.  
     // This is often used for debugging purposes.

   StreamDataSource* getDataSource() ;
     //:Gets the the data source for this decoder

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

   virtual UtlBoolean isDecoding() = 0 ;
     //:Gets the decoding status.  TRUE indicates decoding activity, false
     //:indicates the decoder has completed.

   virtual UtlBoolean validDecoder() = 0 ;
     //:Determines if this is a valid decoder given the associated data 
     //:source.
     // The data source is "peeked" for data, so that the stream's
     // data is not disturbed.

/* ============================ TESTING =================================== */

#ifdef MP_STREAM_DEBUG /* [ */
static const char* getEventString(StreamDecoderEvent event);
#endif /* MP_STREAM_DEBUG ] */


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   StreamFormatDecoder(const StreamFormatDecoder& rStreamFormatDecoder);
     //:Copy constructor

   StreamFormatDecoder& operator=(const StreamFormatDecoder& rhs);
     //:Assignment operator

   virtual void fireEvent(StreamDecoderEvent event);
     //:Fire an event to an interested consumer.


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   StreamDecoderListener *mpListener ;    // Stream Decoder Listener
   StreamDataSource* mpDataSource ;       // Abstract Data Source
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamFormatDecoder_h_
