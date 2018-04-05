// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DCHOOK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DCHOOK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
BOOL WINAPI setMyHook(HWND hWnd);
BOOL WINAPI clearMyHook(HWND hWnd);

#define UWM_WINMINIMIZE_MSG _T("UWM_WINMINIMIZE_MSG-EC8C34C3-15FE-4911-AA41-46176D2251F9")
#define UWM_WINMINIMIZED_MSG _T("UWM_WINMINIMIZED_MSG-AFB82EFD-5DD8-4a66-AA35-00EB8F630ACC")
#define UWM_WINMAXIMIZE_MSG _T("UWM_WINMAXIMIZE_MSG-09DABCF4-A2CF-4d80-BBEF-5656C2FBFEE3")
#define UWM_WINCLOSE_MSG _T("UWM_WINCLOSE_MSG-6BC92757-63D7-47d5-B2CF-FE610C0BE290")