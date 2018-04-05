//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// Deskcon.cpp
//
//	Copyright (c) 2018 Stephen Kew
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in all
//	copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//	SOFTWARE.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"
#include "resource.h"
#include <stdio.h>
#include <commctrl.h>
#include "deskcon.h"
#include "deskconinternal.h"
#include "AboutDeskcon.h"
#include "..\mMUtils\Logger.h"

#include <iostream>
//#include <vector>

#include <gdiplus.h>
#include <shlobj.h>

#pragma comment(lib,"gdiplus.lib")

//using namespace std;

HWND _mainHwnd = NULL;
CLogger* _logger = NULL;

CComModule _Module;

CComPtr< IDebugWindow > _debugWindow;
CComPtr< ITaskbarList > _taskbar;
//CComPtr< IPassportCtl > _poller;
CComPtr< IDeskconWindow > _deskconMainWindow;
CComPtr< INotifyIcon > _notifyIcon;

//int _startX = 100;
//int _startY = 100;
//int _availableX = _startX;
//int _availableY = _startY;

TCHAR _deskconPath[MAX_PATH];

int _monitorCount = 0;

HINSTANCE _hInstance;

UINT UWM_SETTINGCHANGED = 0;

vector< CComPtr< IThumbWindow > > _deskconWindows;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

//void CalculateAvailablePosition( HWND hWnd, int* x, int* y )
//{
//	// ok, we need to find an available position
//	// we start at out set start position
//	// then we see if there is a thumbnail in that location
//	// if not, we can use that location, otherwise we calculate
//	// the next available location to the right or down if it
//	// will go off the current monitor
//
//	(*x) = _availableX;
//	(*y) = _availableY;
//
//	// check if the startx is going off the screen
//	HMONITOR monitor = ::MonitorFromWindow( hWnd, MONITOR_DEFAULTTONEAREST );
//
//	RECT monitorRect = { 0 };
//    MONITORINFO monitorInfo = { 0 };
//    monitorInfo.cbSize = sizeof( monitorInfo );
//    GetMonitorInfo( monitor, &monitorInfo );
//    monitorRect = monitorInfo.rcMonitor;
//
//	_availableX += DESKCON_THUMBNAIL_WIDTH + DESKCON_THUMBNAIL_GAP;
//
//	if( ( _availableX + DESKCON_THUMBNAIL_WIDTH ) > monitorRect.right ) 
//	{
//		_availableY += DESKCON_THUMBNAIL_HEIGHT + DESKCON_THUMBNAIL_GAP;
//		_availableX = _startX;
//
//		if( ( _availableY + DESKCON_THUMBNAIL_HEIGHT ) > monitorRect.bottom )
//		{
//			// ok, so the screen is full, lets try start again
//			_availableY = _startY;
//			return;
//		}
//	}
//}
//

BOOL CALLBACK MyInfoEnumProc( HMONITOR hMonitor,  // handle to display monitor
								HDC hdcMonitor,     // handle to monitor DC
								LPRECT lprcMonitor, // monitor intersection rectangle
								LPARAM dwData       )// data
{
	_monitorCount++;

	return TRUE;
}


void GetMonitorCount( )
{
	EnumDisplayMonitors(NULL, NULL, MyInfoEnumProc, 0);
}




DWORD WINAPI ShowSplash( LPVOID lpParam ) 
{
	MSG msg;

	CComPtr< ISplashWindow > splashWindow = NULL;

	CreateSplashWindow( &splashWindow );

	if( splashWindow->Initialize( _T( "\\images\\splash.png" ) ) )
	{
		splashWindow->Show( );
	}

	while( splashWindow->Showing( ) )
	{
		if( GetMessage( &msg, NULL, 0, 0 ) != 0 )
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	}
	splashWindow = NULL;

	return 0;
}

void ShowSplashThreaded( )
{
	CreateThread( 
        NULL,                        // default security attributes 
        0,                           // use default stack size  
        ShowSplash,                  // thread function 
        NULL,                // argument to thread function 
        0,                           // use default creation flags 
        NULL);                // returns the thread identifier 
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	BOOL bRet = 0;
	MSG msg;
	//CComPtr< ISplashWindow > splashWindow = NULL;

	// first thing. Are we opening the debug window?
	DWORD debug = 0;
	GetDWORDSetting( _T( "Debug" ) , &debug, 0, false );

	_logger = new CLogger((LPCSTR)"D:\\Development\\SVN\\Projects\\miniMIZE\\minimize\\mmLog.txt");

	if( debug )
	{
		CreateDebugWindow( &_debugWindow );
	}

	_mainHwnd = FindWindow( _T( "miniMIZE_Window" ), _T( "" ) );

	if( _mainHwnd )
	{
		// only want one instance, if we get here, miniMIZE is already running
		// instead, assume the notify icon isn't showing and they want to show it
		UINT showTrayIconMsg = ::RegisterWindowMessage( UWM_SHOWNOTIFYICON_MSG );
		
		::SendMessage( _mainHwnd, showTrayIconMsg, NULL, NULL );

		bRet = 2;
	}
	else
	{
		// store our hinstance
		_hInstance = hInstance; 

		// startup gdiplus
		Gdiplus::GdiplusStartupInput g_gdiplusStartupInput;

		ULONG_PTR g_gdiplusToken;

		Gdiplus::GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, 0);

		// get our path
		GetModuleFileName( NULL, _deskconPath, MAX_PATH );

		// do the splash screen thing
		/*CreateSplashWindow( &splashWindow );

		if( splashWindow->Initialize( _T( "\\images\\splash.png" ) ) )
		{
			splashWindow->Show( );
		}

		splashWindow = NULL;*/
		ShowSplashThreaded( );

		// we need common controls
		InitCommonControls();

		// and COM
		CoInitialize( NULL );

		// from COM, we want the ITaskbarlist
		CoCreateInstance( CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (void**)&_taskbar );
		_taskbar->HrInit( );

		// How many monitors do they have?
		GetMonitorCount( );

		// create our main window
		CreateDeskconWindow( &_deskconMainWindow );

		_mainHwnd = _deskconMainWindow->Initialize( );

		// register our shell hook to get flash info
		RegisterShellHookWindow( _mainHwnd );

		while ( ( bRet = GetMessage( &msg, NULL, 0, 0 ) ) != 0 ) 
		{ 
			if (bRet == -1 )
			{
				// handle the error and possibly exit
			}
			else 
			{ 
				TranslateMessage(&msg); 
				DispatchMessage(&msg); 
			} 
		} 

		// shutdown GDIPlus
		Gdiplus::GdiplusShutdown(g_gdiplusToken);

		// get rid of our shell hook
		DeregisterShellHookWindow( _mainHwnd );
	}

	_logger->StopLogger();
	delete _logger;

	return bRet;
}



