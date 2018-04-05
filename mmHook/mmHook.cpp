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

// mmHook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "..\mMUtils\Logger.h"
#include "mmHook.h"
#include <stdio.h>

#pragma data_seg(".SHARED")
static HWND _hmMWndServer = NULL;
#pragma data_seg()
#pragma comment(linker, "/section:.SHARED,RWS")

HHOOK sendHook = NULL;
HHOOK cbtHook = NULL;
HHOOK doneHook = NULL;

HINSTANCE hInstance = NULL;

UINT UWM_WINMINIMIZE = 0;
UINT UWM_WINMINIMIZED = 0;
UINT UWM_WINMAXIMIZE = 0;
UINT UWM_WINCLOSE = 0;

CLogger _logger((LPCSTR)"D:\\Development\\SVN\\Projects\\miniMIZE\\minimize\\mmLog.txt");

#define APP_ERROR 268435456
#define INVALID_HANDLE APP_ERROR + 1
#define SEND_HOOK_FAILED APP_ERROR + 2
#define CBT_HOOK_FAILED APP_ERROR + 3
#define DONE_HOOK_FAILED APP_ERROR + 4

// Forward declaration
static LRESULT CALLBACK SendMsgCallback(int nCode, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK DoneMsgCallback(int nCode, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK CBTCallback( int nCode, WPARAM wParam, LPARAM lParam );

BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			// Save the instance handle because we need it to set the hook later
			hInstance = hModule;
			// This code initializes the hook notification message
			UWM_WINMINIMIZE = ::RegisterWindowMessage( UWM_WINMINIMIZE_MSG );
			UWM_WINMINIMIZED = ::RegisterWindowMessage( UWM_WINMINIMIZED_MSG );
			UWM_WINMAXIMIZE = ::RegisterWindowMessage( UWM_WINMAXIMIZE_MSG );
			UWM_WINCLOSE = ::RegisterWindowMessage( UWM_WINCLOSE_MSG );

			////_logger = new CLogger((LPCSTR)"C:\\mmLog.txt");
			//CLogger _logger((LPCSTR)"D:\\Development\\SVN\\Projects\\miniMIZE\\minimize\\x64\\debug\\mmLog.txt");
			_logger.WriteLine(_T("In DllMain - DLL_PROCESS_ATTACH"));

			return TRUE;
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		{
			// were getting a really odd case here where we were being unloaded and thus unhooking
			// but on startup?!?

			//MessageBox(NULL, "Detatch Unhooking", "Unhook", MB_OK );

			// If the server has not unhooked the hook, unhook it as we unload
			//if(_hmMWndServer != NULL)
			//	clearMyHook(_hmMWndServer);
			////_logger->StopLogger();
			//delete //_logger;
			_logger.WriteLine(_T("In DllMain - DLL_PROCESS_DETACH"));

			return TRUE;
		}
		break;
	}
    return TRUE;
}


BOOL WINAPI setMyHook(HWND hWnd)
{
	_logger.WriteLine(_T("In setMyHook"));

	// are we already installed?
	if(_hmMWndServer != NULL)
		return FALSE;

	// load our hooks
	cbtHook = SetWindowsHookEx(	WH_CBT,
								(HOOKPROC)CBTCallback,
								hInstance,
								0);

	if( NULL == cbtHook )
	{
		return FALSE;
	}
	
	sendHook = SetWindowsHookEx(	WH_CALLWNDPROC,
									(HOOKPROC)SendMsgCallback,
									hInstance,
									0);

	if( NULL == sendHook )
	{
		UnhookWindowsHookEx( cbtHook );

		return FALSE;
	}

	doneHook = SetWindowsHookEx(	WH_CALLWNDPROCRET,
									(HOOKPROC)DoneMsgCallback,
									hInstance,
									0);

	if( NULL == doneHook )
	{
		UnhookWindowsHookEx( sendHook );
		UnhookWindowsHookEx( cbtHook );

		return FALSE;
	}

	// store the handle back to our main window
	_hmMWndServer = hWnd;

	_logger.WriteLine(_T("miniMIZE Hook installed successfully"));

	return TRUE;
}

BOOL WINAPI clearMyHook(HWND hWnd)
{
	_logger.WriteLine(_T("In clearMyHook"));

	BOOL unhooked = FALSE;

	if(hWnd != _hmMWndServer)
	{
		SetLastError( INVALID_HANDLE );

		return FALSE;
	}

	BOOL doneUnhooked = UnhookWindowsHookEx( doneHook );

	BOOL sendUnhooked = UnhookWindowsHookEx( sendHook );

	BOOL cbtUnhooked = UnhookWindowsHookEx( cbtHook );
	
	if( sendUnhooked && cbtUnhooked && doneUnhooked )
	{
		_hmMWndServer = NULL;
		unhooked = TRUE;

		_logger.WriteLine(_T("miniMIZE Hook removed successfully"));
	}

	return unhooked;
}

static LRESULT CALLBACK CBTCallback( int nCode, WPARAM wParam, LPARAM lParam )
{
	HWND window = NULL;

	if(nCode < 0)
	{
		return CallNextHookEx( sendHook, nCode, wParam, lParam );
    }

	window = (HWND)wParam;
    switch( nCode )
	{
		case HCBT_MINMAX:
			{
				switch( LOWORD( lParam ) )
				{
					case SW_MINIMIZE:
						{
							_logger.WriteLine(_T("In CBTCallback: SW_MINIMIZE"));
							SendMessage( _hmMWndServer, UWM_WINMINIMIZE, (WPARAM)window, 0 );
						}
						break;
					case SW_MAXIMIZE:
						{
							_logger.WriteLine(_T("In CBTCallback: SW_MAXIMIZE"));
							PostMessage( _hmMWndServer, UWM_WINMAXIMIZE, (WPARAM)window, 0 );
						}
						break;
					case SW_RESTORE:
						{
							_logger.WriteLine(_T("In CBTCallback: SW_RESTORE"));
							PostMessage( _hmMWndServer, UWM_WINMAXIMIZE, (WPARAM)window, 0 );
						}
						break;
				}
			}
			break;
		case HCBT_DESTROYWND:
			{
				PostMessage( _hmMWndServer, UWM_WINCLOSE, (WPARAM)window , 0 );
			}
			break;
	}

    // Pass the message on to the next hook
    return CallNextHookEx( cbtHook, nCode, wParam, lParam );
}

static LRESULT CALLBACK SendMsgCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	

	if(nCode < 0)
	{
		CallNextHookEx(sendHook, nCode, wParam, lParam);
		return 0;
    }

	PCWPSTRUCT msg = (PCWPSTRUCT)lParam;

    // If it is a mouse-move message, either in the client area or
    // the non-client area, we want to notify the parent that it has
    // occurred. Note the use of PostMessage instead of SendMessage
    if( msg->message == WM_SYSCOMMAND )
	{
		if( msg->wParam == SC_MINIMIZE )
		{
			_logger.WriteLine(_T("In SendMsgCallback: SC_MINIMIZE"));

			// ok, we got the minimize message, so now lets send a message to our app
			SendMessage( _hmMWndServer, UWM_WINMINIMIZE, (WPARAM) msg->hwnd , 0 );
		}
	}
	
    // Pass the message on to the next hook
    return CallNextHookEx( sendHook, nCode, wParam, lParam );
}

static LRESULT CALLBACK DoneMsgCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode < 0)
	{
		CallNextHookEx(sendHook, nCode, wParam, lParam);
		return 0;
    }

	PCWPRETSTRUCT msg = (PCWPRETSTRUCT)lParam;

    // If it is a mouse-move message, either in the client area or
    // the non-client area, we want to notify the parent that it has
    // occurred. Note the use of PostMessage instead of SendMessage
    if( msg->message == WM_SYSCOMMAND )
	{
		if( msg->wParam == SC_MINIMIZE )
		{
			_logger.Write(_T("bob"));
			_logger.Write(_T("bob"), true);
			_logger.WriteLine(_T("In DoneMsgCallback: WM_SYSCOMMAND->SC_MINIMIZE"));

			// ok, we got the minimize message, so now lets send a message to our app
			SendMessage( _hmMWndServer, UWM_WINMINIMIZED, (WPARAM) msg->hwnd , 0 );
		}
	}
	else
	if( msg->message == WM_SIZE )
	{
		if( msg->wParam == SIZE_MINIMIZED )
		{
			_logger.WriteLine(_T("In DoneMsgCallback: WM_SIZE->SC_MINIMIZE"));

			// ok, we got the minimize message, so now lets send a message to our app
			SendMessage( _hmMWndServer, UWM_WINMINIMIZED, (WPARAM) msg->hwnd , 0 );
		}
	}
	
    // Pass the message on to the next hook
    return CallNextHookEx( doneHook, nCode, wParam, lParam );
}