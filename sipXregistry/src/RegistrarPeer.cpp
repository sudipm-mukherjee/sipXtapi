// $Id$
//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "SipRegistrar.h"
#include "RegistrarTest.h"
#include "RegistrarSync.h"
#include "RegistrarPeer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char* RegistrarPeer::STATE_NAMES[] =
   { "Uninitialized", "Reachable", "UnReachable", "Incompatible" };


// Constructor
RegistrarPeer::RegistrarPeer( SipRegistrar*   registrar
                             ,const UtlString& name
                             ,int             rpcPort
                             ,const char*     rpcPath
                             )
   : UtlString(name),
     mLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
     mSyncState(Uninitialized),
     mSentTo(-1),        // intentionally bogus initial state, for debugging
     mReceivedFrom(-1),  // ditto
     mRegistrar(registrar)
{
   mUrl.setScheme(Url::HttpsUrlScheme);
   mUrl.setHostAddress(name);
   mUrl.setHostPort(rpcPort);
   mUrl.setPath(rpcPath);
}

// Destructor
RegistrarPeer::~RegistrarPeer()
{
}

/// The full URL to be used to make an XML RPC request of this peer.
void RegistrarPeer::rpcURL(Url& url)
{
   // no need for lock;  url is set in constructor and never changed.
   url = mUrl;
}

/// Whether or not the most recent attempt to reach this peer succeeded.
RegistrarPeer::SynchronizationState RegistrarPeer::synchronizationState()
{
   OsLock mutex(mLock);
   
   return mSyncState;
}

/// Indicate that a request to this peer failed.
void RegistrarPeer::markUnReachable()
{
   bool notifyTestThread = false;
   
   { // lock scope
      OsLock mutex(mLock);
   
      // If the peer was previously UnReachable, then marking it UnReachable again
      // is a noop.  If the peer was in any other state (Reachable, Uninitialized,
      // Incompatible) then notify the test thread so that we can try to reach the peer
      // again later.
      if (mSyncState != UnReachable)
      {
         mSyncState = UnReachable;
         notifyTestThread = true;

         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "RegistrarPeer::markUnreachable called on peer %s", name());
      }
   }  // release lock before signalling RegistrarTest thread
   
   if (notifyTestThread)
   {
      RegistrarTest* registrarTestThread = mRegistrar->getRegistrarTest();
      assert(registrarTestThread);

      // Tell the RegistrarTest thread to start polling
      registrarTestThread->check();
   }
}

/// Indicate that a request to this peer succeeded or a request was received from it.
void RegistrarPeer::markReachable()
{
   RegistrarSync* notifySyncThread;
   
   { // lock scope
      OsLock mutex(mLock);
   
      notifySyncThread = (  mSyncState != Reachable
                          ? mRegistrar->getRegistrarSync() // was not reachable, so get the thread
                          : NULL // was reachable, so no need to notify the thread again
                          );   
      mSyncState = Reachable;

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "RegistrarPeer::markReachable called on peer %s", name());
   }  // release lock before signalling RegistrarSync thread
   
   if (notifySyncThread)
   {
      // Tell the RegistrarSync thread to start updating
      notifySyncThread->sendUpdates();
   }
}

/// Indicate that a permanent error has occurred with this peer.
void RegistrarPeer::markIncompatible()
{
   OsLock mutex(mLock);

   mSyncState = Incompatible;

   OsSysLog::add(FAC_SIP, PRI_ERR,
                 "RegistrarPeer::markIncompatible called on peer %s", name());
}

/// Set the peer state to a non-initial state (any state but Uninitialized)
void RegistrarPeer::setState(SynchronizationState state)
{
   switch(state)
   {
   case Uninitialized:
      assert(false);
      break;

   case Reachable:
      markReachable();
      break;
   
   case UnReachable:
      markUnReachable();
      break;

   case Incompatible:
      markIncompatible();
      break;

   default:
      assert(false);
      break;
   }
}

/// Return the name of a synchronization state, for debugging
const char* RegistrarPeer::getStateName()
{
   return STATE_NAMES[mSyncState];
}

/// The oldest update successfully sent to this peer.
intll RegistrarPeer::sentTo()
{
   OsLock mutex(mLock);
   
   return mSentTo;
}

/// The last update received from this peer.
intll RegistrarPeer::receivedFrom()
{
   OsLock mutex(mLock);
   
   return mReceivedFrom;
}

void RegistrarPeer::setSentTo(intll updateNumber)
{
   OsLock mutex(mLock);
   
   mSentTo = updateNumber;
}

void RegistrarPeer::setReceivedFrom(intll updateNumber)
{
   OsLock mutex(mLock);
   
   mReceivedFrom = updateNumber;
}

