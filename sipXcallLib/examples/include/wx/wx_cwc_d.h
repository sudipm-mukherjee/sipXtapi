/////////////////////////////////////////////////////////////////////////////
// Name:        wx_cw_d.h
// Purpose:     wxWindows definitions for CodeWarrior builds (Debug)
// Author:      Stefan Csomor
// Modified by:
// Created:     12/10/98
// RCS-ID:      $Id: wx_cwc_d.h,v 1.2 2000/12/10 10:15:06 csomor Exp $
// Copyright:   (c) Stefan Csomor
// Licence:   	wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CW__
#define _WX_CW__

#if __MWERKS__ >= 0x2400
#pragma old_argmatch on
#endif

#if __option(profile)
#error "profiling is not supported in debug versions"
#else
#ifdef __cplusplus
	#if __POWERPC__
		#include <wx_Carbon++_d.mch>
	#endif
#else
	#if __POWERPC__
		#include <wx_Carbon_d.mch>
	#endif
#endif
#endif

#endif
    // _WX_CW__
