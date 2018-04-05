//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// mMLoader.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"
#include "resource.h"
#include "mMLoader.h"
#include "..\mMUtils\Logger.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()


//
// Hook functions
//
typedef BOOL (WINAPI *fnDllSetTrayHook)(HWND window);
typedef BOOL (WINAPI *fnDllUnsetTrayHook)(HWND window);

fnDllSetTrayHook g_fnSetTrayHook = NULL;
fnDllUnsetTrayHook g_fnUnsetTrayHook = NULL;

bool _hookInstalled = false;

HWND _hWnd = NULL;

CComPtr< IMMLoaderWindow > _loaderWindow = NULL;

//CLogger* _logger = NULL;
DEFINE_LOGGER( )

bool LoadHookLibrary( LPTSTR hookLibraryName )
{
	bool retval = false;
	HMODULE	hModTraySpy	= LoadLibrary( hookLibraryName ); //_T( "miniMIZE.dll" ) );
	
	if (hModTraySpy != NULL)
	{
		g_fnSetTrayHook		= (fnDllSetTrayHook)GetProcAddress(hModTraySpy, "setMyHook");
		g_fnUnsetTrayHook	= (fnDllUnsetTrayHook)GetProcAddress(hModTraySpy, "clearMyHook");

		if (!g_fnSetTrayHook || !g_fnUnsetTrayHook )
		{
			FreeLibrary(hModTraySpy);

			//SAFEDEBUGWRITELINE( _debugWindow, _T( "miniMIZE.dll is invalid" ) )
		}
		else
		{
			retval = true;
		}
	}

	return retval;
}

bool LoadHook( LPTSTR hookLibraryName )
{
	LOG_WRITELINE( _T( "Installing miniMIZE 32 bit Hook" ) )

	bool retval = false;
	_hWnd = FindWindow(_T("miniMIZE_Window"), _T(""));

		if( LoadHookLibrary( hookLibraryName ) )
		{
			if( ( g_fnSetTrayHook != NULL ) && ( _hWnd != NULL ) )
			{
				if( g_fnSetTrayHook ( _hWnd ) )
				{
					_hookInstalled = true;
					retval = true;
					LOG_WRITELINE( _T( "miniMIZE 32 bit Hook installed sucessfully" ) )
				}
			}
			else
			{
				LOG_WRITELINE( _T( "32 bit hook dll is not valid" ) )
			}
		}
		else
		{
			LOG_WRITELINE( _T( "Failed to find 32 bit hook dll" ) )
		}

	return retval;
}

bool UnloadHook( )
{
	LOG_WRITELINE( _T( "Removing miniMIZE 32 bit Hook" ) )

	bool retval = false;

	if( _hookInstalled )
	{
		if( ( g_fnUnsetTrayHook != NULL ) && ( _hWnd != NULL ) )
		{
			if( g_fnUnsetTrayHook( _hWnd ) )
			{
				_hookInstalled = false;
				retval = true;
				LOG_WRITELINE( _T( "miniMIZE 32 bit Hook unloaded successfully" ) )
			}
		}
		else
		{
			LOG_WRITELINE( _T( "Global unload function not found to unload 32 bit Hook" ) )
		}
	}

	return retval;
}



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
 	BOOL bRet = -1;
	MSG msg;

	HWND mainHwnd = NULL;

	CREATE_LOGGER_WITH_MSG( (LPCSTR) "D:\\Development\\SVN\\Projects\\miniMIZE\\minimize\\mmLog.txt", _T( "miniMIZE 32 bit Hook Loader Logger Started" ) )
	
	mainHwnd = FindWindow( _T( "miniMIZE_Loader_Window" ), _T( "" ) );

	if( mainHwnd == NULL )
	//{
	//	LOG_WRITELINE( _T("Found miniMIZE Loader Window. Sending Shutdown Message") )

	//	// already running, so send an exit message
	//	UINT shutdownMMLoaderMsg = ::RegisterWindowMessage( UWM_SHUTDOWN_MMLOADER_MSG );
	//	
	//	::SendMessage( mainHwnd, shutdownMMLoaderMsg, NULL, NULL );

	//	bRet = 1;
	//}
	//else
	{
		LOG_WRITELINE( _T( "Failed to find existing miniMIZE Loader. Starting up..." ) )

		if( LoadHook( _T( "mmHook32.dll" ) ) )
		{
			LOG_WRITELINE( _T( "Creating loader window" ) )
			CreateLoaderWindow( &_loaderWindow );

			_loaderWindow->Initialize();

			LOG_WRITELINE( _T( "Starting loader message loop" ) )
			// start a message loop
			while ( ( bRet = GetMessage( &msg, NULL, 0, 0 ) ) != 0 ) 
			{ 
				if ( bRet == -1 )
				{
					// handle the error and possibly exit
				}
				else 
				{ 
					TranslateMessage( &msg ); 
					DispatchMessage( &msg ); 
				} 
			} 

			LOG_WRITELINE(_T("Done"))
			
			if( !UnloadHook( ) )
			{
				bRet = -2;
			}
		}
	}

	LOG_WRITELINE( _T( "miniMIZE loader exiting" ) )

	return bRet;
}