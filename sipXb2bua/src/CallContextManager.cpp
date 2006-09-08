/** 
 * Copyright (c) 2006 BreakPoint Software, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are 
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer. 
 * 
 * Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution. 
 * 
 * Neither the name of the BreakPoint Software, Inc. nor the names of its 
 * contributors may be used to endorse or promote products derived from this 
 * software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author Bob Andreasen (rjandreasen gmail com)
 */

#include <assert.h>

#include "CallContextManager.h"

#include "os/OsSysLog.h"
#include "os/OsLock.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

/****************************************************************************/

CallContextManager::CallContextManager(int iMaxCalls)
    : mCallContextLock(OsMutex::Q_FIFO)
{
    assert(iMaxCalls > 0) ;
    if (iMaxCalls > 0)
        mMaxCalls = iMaxCalls ;
    else
        mMaxCalls = 32 ;

    mpCallContexts = new CallContext[mMaxCalls] ;
    assert(mpCallContexts != NULL) ;
}

/****************************************************************************/

CallContextManager::~CallContextManager()
{
    delete [] mpCallContexts ;
}

/****************************************************************************/

CallContext* 
CallContextManager::allocCallContext(SIPX_CALL           hCall, 
							         CALL_CONTEXT_ORIGIN eOrigin)
{    
    OsLock lock(mCallContextLock) ;

    CallContext* pRC = getCallContext(hCall) ;
    if (pRC == NULL)
    {
        pRC = getCallContext(SIPX_CALL_NULL) ;
        if (pRC)
        {
            pRC->setCallHandle(hCall) ;
			pRC->setOrigin(eOrigin) ;
            printf("<-> Call Context created for call %d\n", hCall) ;
        }
    }
    return pRC ;
}

/****************************************************************************/

void CallContextManager::freeCallContext(SIPX_CALL hCall) 
{
    OsLock lock(mCallContextLock) ;

    CallContext* pRC = getCallContext(hCall) ;
    if (pRC != NULL)
    {
        pRC->reset() ;
        printf("<-> Call Context released for call %d\n", hCall) ;
    }

    sipxCallDestroy(hCall) ;
}

/****************************************************************************/

CallContext* CallContextManager::getCallContext(SIPX_CALL hCall) 
{
    CallContext* pRC = NULL ;
    OsLock lock(mCallContextLock) ;

    for (int i=0; i<mMaxCalls; i++)
    {
        if (mpCallContexts[i].getCallHandle() == hCall)
        {
            pRC = &mpCallContexts[i] ;
            break ;
        }
    }
    return pRC ;
}

/****************************************************************************/

