#ifndef STDAFX_H
#define STDAFX_H

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcmn.h>         // MFC support for Windows 95 Common Controls

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#include <COMUTIL.H>
#include <comdef.h>

#import "ksConstants.tlb" rename_namespace("kapi_consts") named_guids
#import "ksConstants3D.tlb" rename_namespace("kapi_consts3d") named_guids
#import "kAPI5.tlb" rename_namespace("kapi_5") named_guids \
    rename("min", "kApi5_min") rename("max", "kApi5_max") rename("GetObject", "kApi5_GetObject")
#import "kAPI7.tlb" rename_namespace("kapi_7") named_guids \
    rename("DeleteFile", "kApi7_DeleteFile") rename("PostMessage", "kApi7_PostMessage") rename("GetObject", "kApi7_GetObject") \
    rename("FindText", "kApi7_FindText") rename("MessageBoxEx", "kApi7_MessageBoxEx") rename("DrawText", "kApi7_DrawText")

namespace kapi
{
using namespace kapi_consts;
using namespace kapi_consts3d;
using namespace kapi_5;
using namespace kapi_7;
}

#endif /* STDAFX_H */
