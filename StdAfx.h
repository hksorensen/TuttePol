// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__F3B46BE4_5853_405D_A965_DDF970339F23__INCLUDED_)
#define AFX_STDAFX_H__F3B46BE4_5853_405D_A965_DDF970339F23__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// TODO: reference additional headers your program requires here
#include <stdlib.h>

#ifndef __max
#define __max(a,b)	((a)>(b) ? (a) : (b))
#endif

#ifndef __min
#define __min(a,b)	((a)<(b) ? (a) : (b))
#endif


#if defined(_WIN32)
#define int64 __int64
#define uint64 __int64
#elif defined(unix)
#define int64 long long int
#define uint64 unsigned long long int
#endif


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__F3B46BE4_5853_405D_A965_DDF970339F23__INCLUDED_)
