//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// DeskconWindow.cpp
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
#include "resource.h"       // main symbols
#include "Deskcon.h"
#include "deskconinternal.h"
#include "settings.h"
#include "Predicates.h"
#include "..\mMUtils\Logger.h"
#include "ThumbnailFactory.h"

#include <shellapi.h>

//
// Tray messages
//
#define SWM_TRAYMSG		WM_APP		//	the message ID sent to our window
#define SWM_SHOWDEBUG	WM_APP + 1	//	show the window
#define SWM_ACTIVATE	WM_APP + 2	//	hide the window
#define SWM_DEACTIVATE	WM_APP + 3	//	hide the window
#define SWM_SETTINGS	WM_APP + 4	//	hide the window
#define SWM_EXIT		WM_APP + 5	//	close the window
#define SWM_ABOUT		WM_APP + 6	//	about box
#define SWM_TOPMOST		WM_APP + 7
#define SWM_BOTTOMMOST	WM_APP + 8
#define SWM_NORMAL		WM_APP + 9
#define SWM_HELP		WM_APP + 10

#define REFRESHTIMER 101

//
// Globals
//
vector< PEXCLUSIONITEM > _exclusionList;
bool _customShadows = true;
RECT _screenRect = { 0 };
bool _useSnapto = true;

//
// Hook functions
//
typedef BOOL (WINAPI *fnDllSetTrayHook)(HWND window);
typedef BOOL (WINAPI *fnDllUnsetTrayHook)(HWND window);

fnDllUnsetTrayHook g_fnUnsetTrayHook = NULL;

//
// Vista Functions
//
#define MSGFLT_ADD		1

typedef BOOL (WINAPI *fnChangeWindowMessageFilter)(UINT message, DWORD dwFlag);

//
// Externs
//
extern HINSTANCE _hInstance;
extern HWND _mainHwnd;
extern vector< CComPtr< IThumbWindow > > _deskconWindows;
extern UINT UWM_SETTINGCHANGED;
extern CComPtr< IDebugWindow > _debugWindow;
extern CComPtr< INotifyIcon > _notifyIcon;
extern int _monitorCount;
extern TCHAR _deskconPath[MAX_PATH];
extern CLogger* _logger;

//
// Forward declarations
//
BOOL CALLBACK MyInfoEnumProcProxy( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData );
BOOL CALLBACK MinimizeAllEWPP( HWND hwnd, LPARAM lParam );
BOOL CALLBACK RestoreWindowEWPP( HWND hwnd, LPARAM lParam );

#define UWM_SHUTDOWN_MMLOADER_MSG _T( "UWM_SHUTDOWN_MMLOADER_MSG-A5B6F7F9-6BFD-4e4a-84F7-71626DFEFA27" )

//
// CDeskconWindow - main miniMIZE window class
//
class CDeskconWindow : public CWindowImpl< CDeskconWindow >,
						public CComObjectRootEx<CComSingleThreadModel>,
						public IDeskconWindow
{
private:
	UINT UWM_WINMINIMIZE;
	UINT UWM_WINMINIMIZED;
	UINT UWM_WINMAXIMIZE;
	UINT UWM_WINCLOSE;
	UINT UWM_SHELLHOOK;
	UINT UWM_SHOWNOTIFYICON;
	UINT MSG_TASKBARCREATED;

	bool _hookInstalled;
	bool m_regexExclusions;
	bool m_helpExists;

	int m_thumbMargin;
	int m_availableX;
	int m_availableY;
	int m_thumbGap;

	// defaults
	int m_defPositionMode;
	int m_defThumbSize;
	int m_defOpacity;
	int m_monitorNum;
	int m_monitorCount;
	bool m_defShowIcons;
	int m_defSizeMode;
	int m_defWindowLevel;
	int m_defClickStyle;
	COLORREF m_defFlashColor;
	bool m_showNotifyIcon;
	bool m_defUseCtrlDrag;
	int m_minimizeAllMode;
	bool m_hideThumbnails;
	HICON m_trayIcon;
	int m_exclusionKey;
	LPTSTR m_activeIcon;
	LPTSTR m_inactiveIcon;
	bool m_hideTaskbar;
	CComPtr< ITaskHideWindow > m_taskbarHideWindow;

	CComPtr< ISettingsDialog > m_settings;

private:

public:
	CDeskconWindow()
	{
		// register our internam messages
		UWM_WINMINIMIZE = ::RegisterWindowMessage( UWM_WINMINIMIZE_MSG );
		UWM_WINMINIMIZED = ::RegisterWindowMessage( UWM_WINMINIMIZED_MSG );
		UWM_WINMAXIMIZE = ::RegisterWindowMessage( UWM_WINMAXIMIZE_MSG );
		UWM_WINCLOSE = ::RegisterWindowMessage( UWM_WINCLOSE_MSG );
		UWM_SHELLHOOK = ::RegisterWindowMessage( TEXT("SHELLHOOK") );
		UWM_SETTINGCHANGED = ::RegisterWindowMessage( UWM_SETTINGCHANGED_MSG );
		UWM_SHOWNOTIFYICON = ::RegisterWindowMessage( UWM_SHOWNOTIFYICON_MSG );
		MSG_TASKBARCREATED = ::RegisterWindowMessage( UWM_TASKBARCREATED_MSG );

		ChangeWindowMessageFilter(UWM_WINMINIMIZE, MSGFLT_ADD);
		ChangeWindowMessageFilter(UWM_WINMINIMIZED, MSGFLT_ADD);
		ChangeWindowMessageFilter(UWM_WINMAXIMIZE, MSGFLT_ADD);
		ChangeWindowMessageFilter(UWM_WINCLOSE, MSGFLT_ADD);
		ChangeWindowMessageFilter(UWM_SHELLHOOK, MSGFLT_ADD);

		_hookInstalled = false;
		m_helpExists = false;

		m_thumbGap = 10;
		m_thumbMargin = 20;
		m_availableX = 0;
		m_availableY = 0;

		m_defThumbSize = 100; // default size of 100
		m_defOpacity = 100; // default to full opacity
		m_monitorNum = 1; // default to showing on the first monitor
		m_defShowIcons = true; // default to showing icons
		m_defSizeMode = SM_USEWIDTH; // default to width sizing
		m_defWindowLevel = WL_NORMAL; // default to always on bottom
		m_defClickStyle = CS_DOUBLECLICK; // default to double click
		m_showNotifyIcon = true; // default to showing the notify icon
		m_defUseCtrlDrag = false; // default to normal drag
		m_hideTaskbar = true; // hide taskbar buttons by default, seems like everyone wants that
		m_defPositionMode = POS_TOPLEFTRIGHT; // top left corner, moving right by default

		m_minimizeAllMode = SW_MINIMIZE; // set the default minimize All mode to minimize

		m_hideThumbnails = true;
		m_trayIcon = NULL;

		m_settings = NULL;

		m_exclusionKey = VK_CONTROL;

		m_regexExclusions = FALSE;

		m_activeIcon = NULL;
		m_inactiveIcon = NULL;
		m_taskbarHideWindow = NULL;
	}

	~CDeskconWindow( )
	{
		// cleanup the hook
		UnloadHook( );
		
		// cleanup icons
		if( m_activeIcon )
		{
			delete [] m_activeIcon;
			m_activeIcon = NULL;
		}

		if( m_inactiveIcon )
		{
			delete [] m_inactiveIcon;
			m_inactiveIcon = NULL;
		}

		// clean up our exclusion list
		//CleanupExclusions( );

		// clean up our thumbwindows
		//CleanupThumbwindows( );
	}

DECLARE_WND_CLASS( _T( "miniMIZE_Window" ) )

//
// IUnknown
//
BEGIN_COM_MAP( CDeskconWindow )
	COM_INTERFACE_ENTRY( IDeskconWindow )
END_COM_MAP( )

//
// WndProc
//
BEGIN_MSG_MAP(CDeskconWindow)
	MESSAGE_HANDLER( WM_NCCREATE, OnNCCreate )
	MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
	MESSAGE_HANDLER( WM_CLOSE, OnClose )
	MESSAGE_HANDLER( UWM_WINMINIMIZE, OnWindowMinimize )
	MESSAGE_HANDLER( UWM_WINMAXIMIZE, OnWindowMaximize )
	MESSAGE_HANDLER( UWM_WINMINIMIZED, OnWindowMinimized )
	MESSAGE_HANDLER( UWM_WINCLOSE, OnWindowClose )
	MESSAGE_HANDLER( UWM_SHELLHOOK, OnShellHook )
	MESSAGE_HANDLER( SWM_TRAYMSG, OnTrayMessage )
	MESSAGE_HANDLER( UWM_SETTINGCHANGED, OnSettingChanged )
	MESSAGE_HANDLER( UWM_SHOWNOTIFYICON, OnShowNotifyIcon )
	MESSAGE_HANDLER( WM_HOTKEY, OnHotkey )
	MESSAGE_HANDLER( WM_TIMER, OnTimer )
	MESSAGE_HANDLER( MSG_TASKBARCREATED, OnTaskbarCreated )
	COMMAND_ID_HANDLER( SWM_EXIT, OnTrayMenuExitClick )
	COMMAND_ID_HANDLER( SWM_ABOUT, OnTrayMenuAboutClick )
	COMMAND_ID_HANDLER( SWM_SHOWDEBUG, OnTrayMenuShowDbgClick )
	COMMAND_ID_HANDLER( SWM_ACTIVATE, OnTrayMenuActivateClick )
	COMMAND_ID_HANDLER( SWM_DEACTIVATE, OnTrayMenuDeactivateClick )
	COMMAND_ID_HANDLER( SWM_TOPMOST, OnTrayMenuTopMostClick )
	COMMAND_ID_HANDLER( SWM_NORMAL, OnTrayMenuNormalClick )
	COMMAND_ID_HANDLER( SWM_BOTTOMMOST, OnTrayMenuBottomMostClick )
	COMMAND_ID_HANDLER( SWM_SETTINGS, OnTrayMenuSettingsClick )
	COMMAND_ID_HANDLER( SWM_HELP, OnTrayMenuHelpClick )
END_MSG_MAP()

LRESULT OnNCCreate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	AddRef( );
	return 1;
}

LRESULT OnDestroy( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	SAFEDEBUGWRITELINE( _debugWindow, _T( "Main window destroyed" ) )
	_notifyIcon->Show( FALSE );
	return 1;
}
LRESULT OnClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	Shutdown( );
	return 1;
}
LRESULT OnWindowMinimize( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	CComPtr< IThumbWindow > win;
	vector< CComPtr< IThumbWindow > >::iterator it;

	HWND window = (HWND)wParam;
	TCHAR buffer[100];
	bool windowFound = false;
	bool excludeWindow = false;
	GetClassName( window, buffer, 100 );

	TCHAR modulename[100];
	GetProcessExecutableName( window, modulename, 100 );

	TCHAR windowTitle[255];
	::GetWindowText( window, windowTitle, 255 );

	SAFEDEBUGWRITE( _debugWindow, _T( "Module: " ) )
	SAFEDEBUGWRITELINE( _debugWindow, modulename )

	_logger->Write( _T( "Minimise called for module " ) );
	_logger->Write( modulename, true );

	SHORT nVirtKey = 0;
	
	if( m_exclusionKey > 0 )
	{
		nVirtKey = GetKeyState( m_exclusionKey );

		if (nVirtKey & KEY_DOWN) 
		{ 
			// they want to send the window to the exclusion list, so lets doo it! :)
			if( IsAppWindow( window ) )
			{
				PEXCLUSIONITEM ei = MakeExclusionItem( modulename, buffer, windowTitle );
				_exclusionList.push_back( ei );
			}
		}
	}

	
	if( ( IsAppWindow( window ) ) )
	{
		SAFEDEBUGWRITE( _debugWindow, _T( "Window minimized: " ) )
		SAFEDEBUGWRITELINE( _debugWindow, buffer )

		excludeWindow = IsWindowExcluded( modulename, buffer, windowTitle );

		// window is being restored or maximized, so we need to get rid of our deskcon
		if( ! _deskconWindows.empty( ) )
		{

			for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
			{
				// either a window or it's parent will get restored
				if( (*it)->GetMonitoredWindow( ) == window )
				{
					if( excludeWindow )
					{
						// window is being excluded, so remove from list and close
						CComPtr<IThumbWindow> win = (*it);

						_deskconWindows.erase( it );

						win->Close( );

						win = NULL;

						break;
					}
					else
					{
						// ok, we've been found, but does our window still exist?
						if( (*it)->GetHWND( ) )
						{
							if( !(*it)->IsVisible( ) )
							{
								SAFEDEBUGWRITELINE( _debugWindow, _T( "Found Window In List" ) )
								
								(*it)->WindowMinimized();//Initialize( window, 0, 0, (*it)->IgnoreBlack( ), false );

								(*it)->Show( );
							}
						}
						else
						{
							// crap, we've lost our window, probably means we were set to pin to desktop
							//  and explorer crashed. Let's try re-create
							int x = 0;
							int y = 0;

							(*it)->GetLocation( &x, &y );

							(*it)->Initialize( window, 0, 0, (*it)->IgnoreBlack( ) );

							(*it)->SetLocation( x, y );
						}
					}

					windowFound = true;

					break;
				}
			}
		}

		if( !windowFound )
		{
			if( !excludeWindow ) // means it isn't in our list
			{
				SAFEDEBUGWRITELINE( _debugWindow, _T( "New Window" ) )

				bool ignoreBlack = false;

				//CreateThumbWindow( &win );
				//CreateThumbnail( &win, 0 );
				CreateThumbnail( &win, GENERIC_THUMBNAIL );//VISTA_THUMBNAIL );

				if( _tcsicmp( buffer, _T( "consolewindowclass" ) ) == 0 )
				{
					ignoreBlack = true;
				}

				if( 
					( _tcsicmp( buffer, _T( "tapplication" ) ) == 0 ) ||
					( _tcsicmp( buffer, _T( "ideowner" ) ) == 0 )
				)
				{
					win->SetDelphiApp( true );
				}

				win->SetOpacity( m_defOpacity );
				win->SetSize( m_defThumbSize );
				win->SetSizeMode( m_defSizeMode );
				win->SetShowIcons( m_defShowIcons );
				win->SetFlashColor( m_defFlashColor );
				win->SetWindowLevel( m_defWindowLevel );
				win->SetClickStyle( m_defClickStyle );
				win->SetUseCtrlDrag( m_defUseCtrlDrag );

				win->Initialize( window, _screenRect.left, _screenRect.top, ignoreBlack );

				int x = 0;
				int y = 0;
				
				CalculateAvailablePosition( win->GetHWND( ), &x, &y );

				win->SetLocation( x, y );

				win->Show( );

				_deskconWindows.push_back( win );
			}
			else
			{
				SAFEDEBUGWRITELINE( _debugWindow, _T( "Found Exclusion" ) )
			}
		}
	}
	return 1;
}
LRESULT OnWindowMinimized( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	SAFEDEBUGWRITELINE( _debugWindow, _T( "Minimize complete" ) )

	HWND window = (HWND)wParam;
	
	vector< CComPtr< IThumbWindow > >::iterator it;

	if( m_hideTaskbar )
	{
		// window is being restored or maximized, so we need to get rid of our deskcon
		if( ! _deskconWindows.empty( ) )
		{
			for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
			{
				//HWND window = m_hiddenWindow;

				HWND wnd = ::GetWindow( window, GW_OWNER );

				if( wnd )
				{
					window = wnd;
				}

				// either a window or it's parent will get restored
				if( (*it)->GetMonitoredWindow( ) == window )
				{
					// we have a match, probably is the correct one
					(*it)->ShowTaskbarButton( false );

					break;
				}
			}
		}
	}

	return 0;
}
LRESULT OnWindowMaximize( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	HWND window = (HWND)wParam;

	TCHAR buffer[100];
	GetClassName( window, buffer, 100 );

	SAFEDEBUGWRITE( _debugWindow, _T( "Window Maximized / Restored: " ) )
	SAFEDEBUGWRITELINE( _debugWindow, buffer );

	vector< CComPtr< IThumbWindow > >::iterator it;

	// window is being restored or maximized, so we need to get rid of our deskcon
	if( ! _deskconWindows.empty( ) )
	{
		for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
		{
			// either a window or it's parent will get restored
			if( (*it)->GetMonitoredWindow( ) == window )
			{
				// we have a match, probably is the correct one
				// Hide it
				if( (*it)->IsVisible( ) )
				{
					SAFEDEBUGWRITELINE( _debugWindow, _T( "Hiding Window" ) )
					(*it)->WindowMaximized( );
					(*it)->Hide( );
				}
				break;
			}
		}
	}
	return 1;
}
LRESULT OnWindowClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	HWND window = (HWND)wParam;

	TCHAR buffer[100];
	GetClassName( window, buffer, 100 );
	
	vector< CComPtr< IThumbWindow > >::iterator it;

	// window is being closed, so we need to get rid of our deskcon
	if( ! _deskconWindows.empty( ) )
	{
		for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
		{
			if( ( (*it)->GetMonitoredWindow( ) == window ) )
			{
				// we have a match, probably is the correct one
				// remove it from our list
				SAFEDEBUGWRITE( _debugWindow, _T( "Window Closed: " ) )
				SAFEDEBUGWRITELINE( _debugWindow, buffer );
				
				CComPtr< IThumbWindow > win = (*it);
								
				_deskconWindows.erase( it );

				win->Close( );
				win = NULL;
				break;
			}
		}
	}
	return 1;
}
LRESULT OnShellHook( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	// ok, get the window
	HWND window = (HWND)lParam;

	if( (UINT)wParam == HSHELL_FLASH )
	{
		vector< CComPtr< IThumbWindow > >::iterator it;

		// window is being closed, so we need to get rid of our deskcon
		if( ! _deskconWindows.empty( ) )
		{
			for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
			{
				if( (*it)->GetMonitoredWindow( ) == window )
				{
					(*it)->Flash( );
					break;
				}
			}
		}
		return TRUE;
	}
	return 1;
}

LRESULT OnSettingChanged( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	switch( wParam )
	{
		case SETTING_DEFAULTMONITOR:
			{
				// default monitor changed, so update the _startX and _startY settings
				if( (int)lParam <= _monitorCount )
				{
					m_monitorNum = (int)lParam;
					LoadMonitorInfo( );
				}
			}
			break;
		case SETTING_THUMBNAILSIZE:
			{
				// thumbnail size has changed, so update all our deskcons and the default
				m_defThumbSize = (int)lParam;

				vector< CComPtr< IThumbWindow > >::iterator it;

				// window is being closed, so we need to get rid of our deskcon
				if( ! _deskconWindows.empty( ) )
				{
					for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
					{
						(*it)->SetSize( m_defThumbSize );
					}
				}
			}
			break;
		case SETTING_SIZINGMODE:
			{
				// the base for the size of the deskcon has changed up date the deskcons and our default
				m_defSizeMode = (int)lParam;

				vector< CComPtr< IThumbWindow > >::iterator it;

				// window is being closed, so we need to get rid of our deskcon
				if( ! _deskconWindows.empty( ) )
				{
					for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
					{
						(*it)->SetSizeMode( m_defSizeMode );
					}
				}
			}
			break;
		case SETTING_SHOWICONS:
			{
				// the show icons on deskcon has changed up date the deskcons and our default
				m_defShowIcons = (_tcsicmp( (LPCTSTR)lParam, _T( "true" ) ) == 0);

				vector< CComPtr< IThumbWindow > >::iterator it;

				// window is being closed, so we need to get rid of our deskcon
				if( ! _deskconWindows.empty( ) )
				{
					for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
					{
						(*it)->SetShowIcons( m_defShowIcons );
					}
				}
			}
			break;
		case SETTING_OPACITY:
			{
				// the opacity of deskcon has changed up date the deskcons and our default
				m_defOpacity = (int)lParam;

				vector< CComPtr< IThumbWindow > >::iterator it;

				// window is being closed, so we need to get rid of our deskcon
				if( ! _deskconWindows.empty( ) )
				{
					for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
					{
						(*it)->SetOpacity( m_defOpacity );
					}
				}
			}
			break;
		case SETTING_FLASHCOLOR:
			{
				m_defFlashColor = (COLORREF)lParam;

				vector< CComPtr< IThumbWindow > >::iterator it;

				// window is being closed, so we need to get rid of our deskcon
				if( ! _deskconWindows.empty( ) )
				{
					for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
					{
						(*it)->SetFlashColor( m_defFlashColor );
					}
				}
			}
			break;
		case SETTING_NOTIFYICON:
			{
				m_showNotifyIcon = (_tcsicmp( (LPCTSTR)lParam, _T( "true" ) ) == 0);

				if( m_showNotifyIcon )
				{
					if( ( m_showNotifyIcon == false ) && ( _notifyIcon == NULL ) )
					{
						SetupNotifyIcon( );
					}
				}
				else
				{
					if( _notifyIcon != NULL )
					{
						_notifyIcon->Show( FALSE );

						_notifyIcon = NULL;
					}
				}

			}
			break;
		case SETTING_WINDOWLEVEL:
			{
				m_defWindowLevel = (int)lParam;

				// had a request not to change the level for all 
				//  thumbnails when we change this setting
				// SetAllThumbnailLevels( m_defWindowLevel );
			}
			break;
		case SETTING_CLICKSTYLE:
			{
				m_defClickStyle = (int)lParam;

				vector< CComPtr< IThumbWindow > >::iterator it;

				// window is being closed, so we need to get rid of our deskcon
				if( ! _deskconWindows.empty( ) )
				{
					for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
					{
						(*it)->SetClickStyle( m_defClickStyle );
					}
				}
			}
			break;
		case SETTING_CTRLDRAG:
			{
				m_defUseCtrlDrag = (_tcsicmp( (LPCTSTR)lParam, _T( "true" ) ) == 0);

				vector< CComPtr< IThumbWindow > >::iterator it;

				// window is being closed, so we need to get rid of our deskcon
				if( ! _deskconWindows.empty( ) )
				{
					for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
					{
						(*it)->SetUseCtrlDrag( m_defUseCtrlDrag );
					}
				}
			}
			break;
		case SETTING_HKCYCLELEVEL:
			{
				// register the hotkey
				RegisterHotkey( HOTKEY_CYCLELEVEL, (DWORD)lParam );
			}
			break;
		case SETTING_HKMINIMIZEALL:
			{
				// register the hotkey
				RegisterHotkey( HOTKEY_MINIMIZEALL, (DWORD)lParam );
			}
			break;
		case SETTING_HKSHOWHIDETHUMB:
			{
				// register the hotkey
				RegisterHotkey( HOTKEY_HIDESHOW, (DWORD)lParam );
			}
			break;
		case SETTING_CUSTOMSHADOWS:
			{
				_customShadows = (_tcsicmp( (LPCTSTR)lParam, _T( "true" ) ) == 0);

				MessageBox( _T( "You have changed the Cusom Shadow setting, which requires miniMIZE to restart." ), _T( "miniMIZE" ), MB_OK ); 
			}
			break;
		case SETTING_HKEXCLUSION:
			{
				m_exclusionKey = (int)lParam;
			}
			break;
		case SETTING_USEREGEX:
			{
				// are we using regular expressions?
				m_regexExclusions = (_tcsicmp( (LPCTSTR)lParam, _T( "true" ) ) == 0);
			}
			break;
		case SETTING_HIDETASKBAR:
			{
				bool show = false;

				m_hideTaskbar = (_tcsicmp( (LPCTSTR)lParam, _T( "true" ) ) == 0);

				vector< CComPtr< IThumbWindow > >::iterator it;

				if( !m_hideTaskbar )
				{
					show = true;
				}

				// window is being closed, so we need to get rid of our deskcon
				if( ! _deskconWindows.empty( ) )
				{
					for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
					{
						if( ::IsIconic( (*it)->GetMonitoredWindow( ) ) )
						{
							(*it)->ShowTaskbarButton( show );
						}
					}
				}
			}
			break;
		case SETTING_POSITION:
			{
				m_defPositionMode = (int)lParam;

				// mode changed, reset the start position by changing the available position
				m_availableX = 0;
				m_availableY = 0;
			}
			break;
		case SETTING_MARGIN:
			{
				m_thumbMargin = (int)lParam;

				// margin changed, reset the start position by changing the available position
				//m_availableX = 0;
				//m_availableY = 0;
			}
			break;
		case SETTING_SNAPTO:
			{
				// using snapto?
				_useSnapto = (_tcsicmp( (LPCTSTR)lParam, _T( "true" ) ) == 0);
			}
			break;
	}

	return 1;
}
LRESULT OnHotkey( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	switch( wParam )
	{
		case HOTKEY_CYCLELEVEL:
			{
				SAFEDEBUGWRITELINE( _debugWindow, _T( "Cycle" ) )
				
				m_defWindowLevel++;

				if( m_defWindowLevel > WL_ALWAYSONTOP )
					m_defWindowLevel = WL_ALWAYSONBOTTOM;

				SetAllThumbnailLevels( m_defWindowLevel );
			}
			break;
		case HOTKEY_MINIMIZEALL:
			{
				SAFEDEBUGWRITELINE( _debugWindow, _T( "minimize" ) )

				MinimizeAllWindows( SW_MINIMIZE );
			}
			break;
		case HOTKEY_HIDESHOW:
			{
				SAFEDEBUGWRITELINE( _debugWindow, _T( "hide show" ) )
				
				HideAllThumbnails( m_hideThumbnails );

				m_hideThumbnails = !m_hideThumbnails; // set it to the the opposite of what it was
			}
			break;
	}

	return 0;
}
LRESULT OnTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	vector< CComPtr< IThumbWindow > >::iterator it;

	if( wParam == REFRESHTIMER )
	{
		SAFEDEBUGWRITELINE( _debugWindow, _T( "Refresh Timer" ) )

		// loop through the thumbnails to see if they are all valid
		if( ! _deskconWindows.empty( ) )
		{
			for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
			{
				if( !::IsWindow( (*it)->GetMonitoredWindow( ) ) )
				{
					CComPtr< IThumbWindow > win = (*it);
								
					_deskconWindows.erase( it );

					win->Close( );
					win = NULL;
					break;
				}
			}
		}

	}

    return 0;
}


LRESULT OnTaskbarCreated( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	if( _notifyIcon )
	{
		_notifyIcon->Show( TRUE );
	}

    return 0;
}


LRESULT OnShowNotifyIcon( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	if( ( m_showNotifyIcon == false ) && ( _notifyIcon == NULL ) )
	{
		SetupNotifyIcon( );
	}

	return 1;
}
LRESULT OnTrayMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	switch(lParam)
	{
		case WM_LBUTTONDBLCLK:
			{
				ShowSettingsDialog( );
			}
			break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_CONTEXTMENU:
			{
				SHORT nVirtKey = GetKeyState( VK_SHIFT ); 
				
				if (nVirtKey & KEY_DOWN) 
                { 
					ShowContextMenu( true );
				}
				else
				{
					ShowContextMenu( false );
				}
			}
	}
	return 1;
}
LRESULT OnTrayMenuExitClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CleanupExclusions( );
	CleanupThumbwindows( );

	SendMessage( WM_CLOSE );
	return 1;
}
LRESULT OnTrayMenuAboutClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CComPtr<IAboutDeskcon> about;

	CreateAboutDeskconWindow( &about );

	about->ShowModal( m_hWnd );
	
	return 1;
}
LRESULT OnTrayMenuShowDbgClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if( _debugWindow == NULL )
	{
		CreateDebugWindow( &_debugWindow );
	}
	
	return 1;
}
LRESULT OnTrayMenuActivateClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if( LoadHook( ) )
	{
		ChangeNotifyIcon( false );
	}
	
	return 1;
}
LRESULT OnTrayMenuDeactivateClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if( UnloadHook( ) )
	{
		ChangeNotifyIcon( true );
	}
	else
	{
		SAFEDEBUGWRITELINE( _debugWindow, _T( "Error Unloading Hook" ) )
	}

	return 1;
}
LRESULT OnTrayMenuSettingsClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ShowSettingsDialog( );

	return 1;
}

LRESULT OnTrayMenuTopMostClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//vector< CComPtr< IThumbWindow > >::iterator it;

	//// window is being closed, so we need to get rid of our deskcon
	//if( ! _deskconWindows.empty( ) )
	//{
	//	for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
	//	{
	//		(*it)->SetWindowLevel( WL_ALWAYSONTOP );
	//	}
	//}
	SetAllThumbnailLevels( WL_ALWAYSONTOP );

	return 1;
}
LRESULT OnTrayMenuNormalClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//vector< CComPtr< IThumbWindow > >::iterator it;

	//// window is being closed, so we need to get rid of our deskcon
	//if( ! _deskconWindows.empty( ) )
	//{
	//	for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
	//	{
	//		(*it)->SetWindowLevel( WL_NORMAL );
	//	}
	//}
	SetAllThumbnailLevels( WL_NORMAL );

	return 1;
}
LRESULT OnTrayMenuBottomMostClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//vector< CComPtr< IThumbWindow > >::iterator it;

	//// window is being closed, so we need to get rid of our deskcon
	//if( ! _deskconWindows.empty( ) )
	//{
	//	for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
	//	{
	//		(*it)->SetWindowLevel( WL_ALWAYSONBOTTOM );
	//	}
	//}
	SetAllThumbnailLevels( WL_ALWAYSONBOTTOM );

	return 1;
}
LRESULT OnTrayMenuHelpClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TCHAR filepath[MAX_PATH];

	_tcscpy( filepath, ( LPCTSTR )_deskconPath );

	if( PathRemoveFileSpec( filepath ) )
	{
		_tcscat( filepath, _T( "\\help\\index.html" ) );

		::ShellExecute(NULL,_T("open"), filepath ,NULL,NULL,SW_SHOWNORMAL); 
	}

	return 1;
}

virtual void OnFinalMessage( HWND hWnd )
{
	Release( );
}


private:
void LoadSettings( )
{
	// load our settings into the default variables
	LPTSTR szValue = NULL;
	DWORD dwValue = -1;

	// opacity
	GetDWORDSetting( _T( "Global Opacity" ), &dwValue, 100 );

	if( dwValue > 100 )
		dwValue = 100;

	if( dwValue < 10 )
		dwValue = 10;

	m_defOpacity = FindNearestTen( dwValue );

	// Size
	GetDWORDSetting( _T( "Thumbnail Size" ), &dwValue, 100 );

	//if( dwValue > 200 )
	//	dwValue = 200;

	if( dwValue < 10 )
		dwValue = 10;

	m_defThumbSize = dwValue;

	// sizing mode
	GetDWORDSetting( _T( "Size Mode" ), &dwValue, SM_USEWIDTH );

	if( dwValue > 2 )
		dwValue = 2;

	if( dwValue < 0 )
		dwValue = 0;

	m_defSizeMode = dwValue;

	// Window Level
	GetDWORDSetting( _T( "Thumbnail Level" ), &dwValue, WL_NORMAL );

	if( dwValue > 3 ) 
		dwValue = 3;
	
	if( dwValue < 1 ) 
		dwValue = 1;

	m_defWindowLevel = dwValue;

	// icons
	GetStringSetting( _T( "Show Icons" ), &szValue, _T( "True" ) );

	if( szValue )
	{
		m_defShowIcons = ( _tcsicmp( szValue, _T( "true" ) ) == 0 );

		delete [] szValue;
		szValue = NULL;
	}

	// monitors
	GetDWORDSetting( _T( "Default Monitor" ), &dwValue, 1 );

	if( dwValue <= _monitorCount )
	{
		m_monitorNum = dwValue;
		LoadMonitorInfo( );
	}

	// flash color
	GetDWORDSetting( _T( "Flash Color" ), &m_defFlashColor, RGB( 255, 0, 0 ) );

	// notify icon
	GetStringSetting( _T( "Show In Tray" ), &szValue, _T( "True" ) );

	if( szValue )
	{
		m_showNotifyIcon = ( _tcsicmp( szValue, _T( "true" ) ) == 0 );

		delete [] szValue;
		szValue = NULL;
	}

	// Click style
	GetDWORDSetting( _T( "Click Style" ), &dwValue, CS_DOUBLECLICK );

	if( dwValue > 1 ) 
		dwValue = 1;
	
	if( dwValue < 0 ) 
		dwValue = 0;

	m_defClickStyle = dwValue;

	// ctrl drag
	GetStringSetting( _T( "Ctrl Drag" ), &szValue, _T( "False" ) );

	if( szValue )
	{
		m_defUseCtrlDrag = ( _tcsicmp( szValue, _T( "true" ) ) == 0 );

		delete [] szValue;
		szValue = NULL;
	}

	// hotkeys
	DWORD hotkey = 0;

	GetDWORDSetting( _T( "Cycle Level Key" ), &hotkey, 0 );

	if( hotkey > 0 )
		RegisterHotkey( HOTKEY_CYCLELEVEL, hotkey );

	GetDWORDSetting( _T( "Minimize Key" ), &hotkey, 0 );

	if( hotkey > 0 )
		RegisterHotkey( HOTKEY_MINIMIZEALL, hotkey );

	GetDWORDSetting( _T( "Show Thumb Key" ), &hotkey, 0 );

	if( hotkey > 0 )
		RegisterHotkey( HOTKEY_HIDESHOW, hotkey );

	// custom shadows
	GetStringSetting( _T( "Custom Shadows" ), &szValue, _T( "True" ), false );

	if( szValue )
	{
		_customShadows = ( _tcsicmp( szValue, _T( "true" ) ) == 0 );

		delete [] szValue;
		szValue = NULL;
	}

	// get the key used to set exclusions
	GetDWORDSetting( _T( "Exclude Key" ), &hotkey, 0 );

	if( hotkey > 0 )
	{
		if( hotkey < VK_SHIFT )
			hotkey = VK_SHIFT;

		if( hotkey > VK_MENU )
			hotkey = VK_MENU;
	}

	m_exclusionKey = hotkey;

	// regex exclusions?
	GetStringSetting( _T( "Advanced Excludes" ), &szValue, _T( "False" ) );

	if( szValue )
	{
		m_regexExclusions = ( _tcsicmp( szValue, _T( "true" ) ) == 0 );

		delete [] szValue;
		szValue = NULL;
	}


	// branding settings
	GetStringSetting( _T( "Branding Active" ), &szValue, NULL );

	if( szValue )
	{
		m_activeIcon = new TCHAR[ _tcslen( szValue ) + 1];
		_tcscpy( m_activeIcon, szValue );

		delete [] szValue;
		szValue = NULL;
	}

	GetStringSetting( _T( "Branding Inactive" ), &szValue, NULL );

	if( szValue )
	{
		m_inactiveIcon = new TCHAR[ _tcslen( szValue ) + 1];
		_tcscpy( m_inactiveIcon, szValue );

		delete [] szValue;
		szValue = NULL;
	}

	// hide taskbar buttons
	GetStringSetting( _T( "Hide Taskbar Buttons" ), &szValue, _T( "True" ) );

	if( szValue )
	{
		m_hideTaskbar = ( _tcsicmp( szValue, _T( "true" ) ) == 0 );

		delete [] szValue;
		szValue = NULL;
	}

	// positioning
	GetDWORDSetting( _T( "Positioning" ), &dwValue, POS_TOPLEFTRIGHT );

	if( dwValue < POS_TOPLEFTRIGHT )
		dwValue = POS_TOPLEFTRIGHT;

	if( dwValue > POS_BOTTOMRIGHTUP )
		dwValue = POS_BOTTOMRIGHTUP;

	m_defPositionMode = dwValue;

	// margin
	GetDWORDSetting( _T( "Margin" ), &dwValue, 20 );

	if( dwValue < 0 )
		dwValue = 0;

	m_thumbMargin = dwValue;

	// icons
	GetStringSetting( _T( "Snapto" ), &szValue, _T( "True" ) );

	if( szValue )
	{
		_useSnapto = ( _tcsicmp( szValue, _T( "true" ) ) == 0 );

		delete [] szValue;
		szValue = NULL;
	}

}

BOOL RegisterHotkey( int hotkeyId, DWORD newhotkey )
{
	BOOL ret = TRUE;

	UnregisterHotKey( m_hWnd, hotkeyId );

	if( newhotkey > 0 )
	{
		BYTE newMod = 0;
		BYTE vk = LOBYTE( newhotkey );
		BYTE mod = HIBYTE( newhotkey );
		
		// okeeey, there is a bug in the MS hotkey control, switches alt and ctrl
		// lets fix that here
		newMod = mod;

		if( ( mod & HOTKEYF_SHIFT ) == HOTKEYF_SHIFT )
		{
			newMod &= ~HOTKEYF_SHIFT;
			newMod |= HOTKEYF_ALT;
		}

		if( ( mod & HOTKEYF_ALT ) == HOTKEYF_ALT )
		{
			newMod &= ~HOTKEYF_ALT;
			newMod |= HOTKEYF_SHIFT;
		}

		ret = RegisterHotKey( m_hWnd, hotkeyId, newMod, vk );

		if( !ret )
		{
			SAFEDEBUGWRITELINE( _debugWindow, _T( "Failed to set hotkey" ) )
		}
	}

	return ret;
}

void LoadMonitorInfo( )
{
	m_monitorCount = 0;
	EnumDisplayMonitors(NULL, NULL, MyInfoEnumProcProxy, (LPARAM) this);
}

void Shutdown( )
{
	SAFEDEBUGWRITELINE( _debugWindow, _T( "Shutting Down" ) )
	UnloadHook( );

	m_taskbarHideWindow->Close( );

	m_taskbarHideWindow = NULL;
	
	::KillTimer( m_hWnd, REFRESHTIMER );

	DestroyWindow( ); 
	PostQuitMessage(0);
}

void SetAllThumbnailLevels( int level )
{
	vector< CComPtr< IThumbWindow > >::iterator it;

	// window is being closed, so we need to get rid of our deskcon
	if( ! _deskconWindows.empty( ) )
	{
		for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
		{
			// bring the windows to front if we're setting them on top
			if( level == WL_ALWAYSONTOP )
				SetForegroundWindow( (*it)->GetHWND( ) );

			(*it)->SetWindowLevel( level );
		}
	}
}

void MinimizeAllWindows( int minimize )
{
	m_minimizeAllMode = minimize;
	EnumWindows( (WNDENUMPROC) MinimizeAllEWPP, (LPARAM) this );
}
void HideAllThumbnails( bool hide )
{
	vector< CComPtr< IThumbWindow > >::iterator it;

	// window is being closed, so we need to get rid of our deskcon
	if( ! _deskconWindows.empty( ) )
	{
		for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
		{
			if( hide )
                (*it)->Hide( );
			else
				(*it)->Show( );
		}
	}
}

void ShowSettingsDialog( )
{
	if( m_settings == NULL )
	{
		CreateSettingsDialog( &m_settings );

		m_settings->ShowModal( m_hWnd );

		m_settings = NULL;
	}
	else
	{
		m_settings->BringToFront( );
	}
}
bool IsWindowExcluded( LPTSTR process, LPTSTR classname, LPTSTR title )
{
	bool retval = false;

	// always exclude console windows
	if( _tcsicmp( classname, _T( "consolewindowclass" ) ) == 0 )
	{
		return true;
	}

	if( !_exclusionList.empty( ) )
	{
		if( m_regexExclusions )
		{
			// is it in our exclusion list?
			FindExclusionItemREPredicate findPredicate( process, classname, title );

			vector<PEXCLUSIONITEM>::iterator it;

			it = find_if( _exclusionList.begin(), _exclusionList.end(), findPredicate );

			if( it != _exclusionList.end( ) )
				retval = true;
		}
		else
		{
			// is it in our exclusion list?
			FindExclusionItemPredicate findPredicate( process, classname, title );

			vector<PEXCLUSIONITEM>::iterator it;

			it = find_if( _exclusionList.begin(), _exclusionList.end(), findPredicate );

			if( it != _exclusionList.end( ) )
				retval = true;
		}
	}

	return retval;
}

bool IsAppWindow( HWND hwnd )
{
	// this function will work out if we have an app window or not
	if( ::IsWindowVisible( hwnd ) )
	{
		DWORD exStyle = ::GetWindowLong( hwnd, GWL_EXSTYLE );
		HWND parent = ( HWND )::GetWindowLong( hwnd, GWLP_HWNDPARENT );
		HWND owner = ::GetWindow( hwnd, GW_OWNER );

		// if WS_EX_APPWINDOW is set then we have 
		if( ( exStyle & WS_EX_APPWINDOW ) == WS_EX_APPWINDOW )
		{
			return true;
		}

		// otherwise, if it has no parent and isn't a child window
		if( ( parent == NULL ) && (  owner == NULL ) )
		{
			if( ( exStyle & WS_EX_TOOLWINDOW ) != WS_EX_TOOLWINDOW )
			{
				return true;
			}
		}
	}

	return false;
}

void ShowContextMenu( bool addDebug )
{
	POINT pt;
	HMENU hSubmenu = NULL;

	::GetCursorPos(&pt);

	HMENU hMenu = ::CreatePopupMenu();

	if(hMenu)
	{
		if( addDebug )
		{
			::InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SHOWDEBUG, _T( "Show Debug Window" ) );

			::InsertMenu(hMenu, -1, MF_SEPARATOR, 0, NULL);
		}

		UINT flags = MF_BYPOSITION;

		if( _hookInstalled )
			flags |= MF_GRAYED;

		::InsertMenu(hMenu, -1, flags, SWM_ACTIVATE, _T( "Activate" ) );

		flags = MF_BYPOSITION;

		if( !_hookInstalled )
			flags |= MF_GRAYED;

		::InsertMenu(hMenu, -1, flags, SWM_DEACTIVATE, _T( "Deactivate" ) );

		hSubmenu = ::CreatePopupMenu( );

		if( hSubmenu )
		{
			::InsertMenu( hSubmenu, -1, MF_BYPOSITION, SWM_TOPMOST, _T( "Pin to Top" ) );

			::InsertMenu( hSubmenu, -1, MF_BYPOSITION, SWM_NORMAL, _T( "Normal" ) );

			::InsertMenu( hSubmenu, -1, MF_BYPOSITION, SWM_BOTTOMMOST, _T( "Pin to Desktop" ) );

			::InsertMenu( hMenu, -1, MF_BYPOSITION | MF_POPUP ,(UINT_PTR) hSubmenu, _T( "Thumbnail Level" ) );
		}

		::InsertMenu(hMenu, -1, MF_SEPARATOR, 0, NULL);

		::InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SETTINGS, _T( "Settings" ) );

		flags = MF_BYPOSITION;

		if( !m_helpExists )
			flags |= MF_GRAYED;

		::InsertMenu(hMenu, -1, flags, SWM_HELP, _T( "Help" ) );
		
		::InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_ABOUT, _T( "About miniMIZE ..." ) );

		::InsertMenu(hMenu, -1, MF_SEPARATOR, 0, NULL);
		
		::InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, _T( "Exit" ) );

		// note:	must set window to the foreground or the
		//			menu won't disappear when it should
		::SetForegroundWindow( m_hWnd );

		::TrackPopupMenu( hMenu, TPM_BOTTOMALIGN,	pt.x, pt.y, 0, _mainHwnd, NULL );
		
		if( hSubmenu )
			::DestroyMenu( hSubmenu );

		::DestroyMenu( hMenu );
	}
}


bool RunMinimizeLoader(bool install)
{
	LPTSTR cmd;

	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
    if( !CreateProcess( NULL,   // No module name (use command line)
        _T("mMLoader.exe"),        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
		_logger->WriteLine("CreateProcess failed");
        return false;
    }

	return true;
}

bool LoadHook( )
{
	/*SAFEDEBUGWRITELINE( _debugWindow, _T( "Installing miniMIZE Hook" ) )

	if( RunMinimizeLoader(true) > 0 )
	{
		SAFEDEBUGWRITELINE( _debugWindow, _T( "miniMIZE installed sucessfully" ) )
		return true;
	}
	else
	{
		SAFEDEBUGWRITELINE( _debugWindow, _T( "Could not load miniMIZE.dll" ) )
		return false;
	}*/



	SAFEDEBUGWRITELINE( _debugWindow, _T( "Installing miniMIZE Hook" ) )

	_logger->WriteLine( _T("Installing miniMIZE Hook") );

	bool retval = false;
	LPTSTR hookName;

#ifdef _64BIT
	hookName = _T("mmHook64.dll");
#else
	hookName = _T("mmHook32.dll");
#endif

	HMODULE	hModTraySpy	= LoadLibrary( hookName );
	fnDllSetTrayHook fnSetTrayHook = NULL;
	
	if (hModTraySpy != NULL)
	{
		fnSetTrayHook		= (fnDllSetTrayHook)GetProcAddress(hModTraySpy, "setMyHook");
		g_fnUnsetTrayHook	= (fnDllUnsetTrayHook)GetProcAddress(hModTraySpy, "clearMyHook");

		if (!fnSetTrayHook || !g_fnUnsetTrayHook )
		{
			FreeLibrary(hModTraySpy);

			SAFEDEBUGWRITELINE( _debugWindow, _T( "miniMIZE.dll is invalid" ) )
		}
		else
		{
			if( !fnSetTrayHook ( m_hWnd ) )
			{
				SAFEDEBUGWRITELINE( _debugWindow, _T( "Could not load miniMIZE.dll" ) )
			}
			else
			{
				_hookInstalled = true;
				retval = true;
				SAFEDEBUGWRITELINE( _debugWindow, _T( "miniMIZE installed sucessfully" ) )

				TCHAR buffer[100];
			
				_stprintf( buffer, _T( "Handle: 0x%X" ), m_hWnd );
				SAFEDEBUGWRITELINE( _debugWindow, buffer )

			}
		}
	}
	else
	{
		SAFEDEBUGWRITELINE( _debugWindow, _T( "miniMIZE.dll not found" ) )
	}

// if we're running in 64 bit mode, load the 32bit hook to hook 32 bit apps
#ifdef _64BIT
	if( retval )
	{
		_logger->WriteLine( _T( "64 Bit detected, Loading 32 bit compatability Hook" ) );

		if( RunMinimizeLoader(true) > 0 )
		{
			_logger->WriteLine( _T( "miniMIZE 32 bit installed sucessfully" ) );
			return true;
		}
		else
		{
			_logger->WriteLine( _T( "Could not load mmHook32.dll" ) );
			return false;
		}
	}
#endif
	return retval;
}

bool UnloadHook( )
{
	/*SAFEDEBUGWRITELINE( _debugWindow, _T( "Removing miniMIZE Hook" ) )

		if( RunMinimizeLoader(false) > 0 )
		{
			SAFEDEBUGWRITELINE( _debugWindow, _T( "miniMIZE unloaded successfully" ) )
			return true;
		}
		else
		{
			SAFEDEBUGWRITELINE( _debugWindow, _T( "Failed to unload miniMIZE." ) )
			return false;
		}*/

	bool retval = false;

	if( g_fnUnsetTrayHook )
	{
		if( g_fnUnsetTrayHook( m_hWnd ) )
		{
			_hookInstalled = false;
			retval = true;
			SAFEDEBUGWRITELINE( _debugWindow, _T( "miniMIZE unloaded successfully" ) )
		}
		else
		{
			TCHAR buffer[100];
			DWORD error = GetLastError( );
			
			_stprintf( buffer, _T( "Failed to unload miniMIZE. Error Code: %d" ), error );
			SAFEDEBUGWRITELINE( _debugWindow, buffer )

			_stprintf( buffer, _T( "Handle: 0x%X" ), m_hWnd );
			SAFEDEBUGWRITELINE( _debugWindow, buffer )

		}
	}
	else
	{
		SAFEDEBUGWRITELINE( _debugWindow, _T( "Global unload function not found" ) )
	}

#ifdef _64BIT
	HWND loaderWnd = FindWindow( _T( "miniMIZE_Loader_Window" ), _T( "" ) );

	if( loaderWnd != NULL )
	{
		//LOG_WRITELINE( _T("Found miniMIZE Loader Window. Sending Shutdown Message") )

		// already running, so send an exit message
		UINT shutdownMMLoaderMsg = ::RegisterWindowMessage( UWM_SHUTDOWN_MMLOADER_MSG );
		
		::SendMessage( loaderWnd, shutdownMMLoaderMsg, NULL, NULL );

		//bRet = 1;
	}
#endif
	
	return retval;
}

HICON GetNotifyHIcon( bool disabled )
{
	HICON icon = NULL;

	// loading the disabled icon?
	if( disabled )
	{
		// using another branding?
		if( m_inactiveIcon )
		{
			// yup0r, so load it for me
			//icon = LoadIcon( _hInstance, m_inactiveIcon );
			icon = (HICON)LoadImage( _hInstance, m_inactiveIcon, IMAGE_ICON, 16 , 16, LR_LOADTRANSPARENT | LR_LOADFROMFILE );

			DWORD error = GetLastError( );

			int i = 0;
		}

		if( icon == NULL )
		{
			// either we are loading this be deafult, or the other icon didn't work
			icon = LoadIcon( _hInstance, MAKEINTRESOURCE( IDI_DISABLED ) );
		}
	}
	else
	{
		// using another branding?
		if( m_activeIcon )
		{
			// yup0r, so load it for me
			icon = (HICON)LoadImage( _hInstance, m_activeIcon, IMAGE_ICON, 16 , 16, LR_LOADTRANSPARENT | LR_LOADFROMFILE );
		}

		if( icon == NULL )
		{
			// either we are loading this be deafult, or the other icon didn't work
			icon = LoadIcon( _hInstance, MAKEINTRESOURCE( IDI_DESKCONSM ) );
		}
	}

	return icon;
}
void SetupNotifyIcon( )
{
	CreateNotifyIcon( &_notifyIcon );

	m_trayIcon = GetNotifyHIcon( true );
	_notifyIcon->Initialize( m_hWnd, SWM_TRAYMSG, m_trayIcon, _T( "miniMIZE" ) );
	_notifyIcon->Show( TRUE );
}

void ChangeNotifyIcon( bool disabled )
{
	LPTSTR resIcon = NULL;

	HICON tempIcon = m_trayIcon;

	if( _notifyIcon != NULL )
	{
		m_trayIcon = GetNotifyHIcon( disabled );

		_notifyIcon->SetIcon( m_trayIcon );

		DestroyIcon( tempIcon );
	}
}

void CleanupExclusions( )
{
	vector< PEXCLUSIONITEM >::iterator it;

	if( !_exclusionList.empty( ) )
	{
		for( it = _exclusionList.begin( ); it != _exclusionList.end( ); it++ )
		{
			delete (*it);
		}

		_exclusionList.clear( );
	}
}
void CleanupThumbwindows( )
{
	vector< CComPtr< IThumbWindow > >::iterator it;

	if( ! _deskconWindows.empty( ) )
	{
		for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
		{
			(*it)->ShowTaskbarButton( true );
			(*it)->Close( );

			(*it) = NULL;
		}

		_deskconWindows.clear( );
	}
}

void RestoreAllWindows( )
{
	EnumWindows( (WNDENUMPROC) RestoreWindowEWPP, (LPARAM) this );
}

void CalculateAvailablePosition( HWND hWnd, int* x, int* y )
{
	int startX = 0;
	int startY = 0;

	bool acrossDown = true; // true for across first then down

	int horizDir = 1; // 1 is for left to right
	int vertDir = 1;  // 1 top to bottom

	// check if the startx is going off the screen
	HMONITOR monitor = ::MonitorFromWindow( hWnd, MONITOR_DEFAULTTONEAREST );

	RECT monitorRect = { 0 };
    MONITORINFO monitorInfo = { 0 };
    monitorInfo.cbSize = sizeof( monitorInfo );
    GetMonitorInfo( monitor, &monitorInfo );
    monitorRect = monitorInfo.rcMonitor;

	// get the start positions
	GetStartPosition( m_defPositionMode, &startX, &startY );

	// work out how the position is being set
	if( ( m_defPositionMode == POS_TOPLEFTDOWN ) || ( m_defPositionMode == POS_TOPRIGHTDOWN ) || 
		( m_defPositionMode == POS_BOTTOMLEFTUP ) || ( m_defPositionMode == POS_BOTTOMRIGHTUP ) )
	{
		// moving up or down first
		acrossDown = false;
	}

	
	if( ( m_defPositionMode == POS_TOPRIGHTLEFT ) || ( m_defPositionMode == POS_BOTTOMRIGHTLEFT ) ||
		( m_defPositionMode == POS_TOPRIGHTDOWN ) || ( m_defPositionMode == POS_BOTTOMRIGHTUP ) )
	{
		horizDir = -1;
	}

	if( ( m_defPositionMode == POS_BOTTOMLEFTRIGHT ) || ( m_defPositionMode == POS_BOTTOMRIGHTLEFT ) ||
		( m_defPositionMode == POS_BOTTOMLEFTUP ) || ( m_defPositionMode == POS_BOTTOMRIGHTUP ) )
	{
		// moving up or down first
		vertDir = -1;
	}


	if( ( m_availableX == 0 ) && ( m_availableY == 0 ) )
	{
		m_availableX = startX;
		m_availableY = startY;
	}
	else
	{
		// across - down
		if( acrossDown )
		{
			m_availableX += ( horizDir * ( m_defThumbSize + m_thumbGap ) );

			if( ( ( m_availableX + ( m_defThumbSize / 2 ) ) > monitorRect.right ) ||
				( ( m_availableX - ( m_defThumbSize / 2 ) ) < monitorRect.left ) )
			{
				m_availableY += ( vertDir * ( m_defThumbSize + m_thumbGap ) );
				m_availableX = startX;

				if( ( ( m_availableY + ( m_defThumbSize / 2 ) ) > monitorRect.bottom ) ||
					( ( m_availableY - ( m_defThumbSize / 2 ) ) < monitorRect.top ) )
				{
					// ok, so the screen is full, lets try start again
					m_availableY = startY;
					return;
				}
			}
		}
		else
		{
			m_availableY += ( vertDir * ( m_defThumbSize + m_thumbGap ) );

			// down - across
			if( ( ( m_availableY + ( m_defThumbSize / 2 ) ) > monitorRect.bottom ) ||
				( ( m_availableY - ( m_defThumbSize / 2 ) ) < monitorRect.top ) )
			{
				m_availableX += ( horizDir * ( m_defThumbSize + m_thumbGap ) );
				m_availableY = startY;
				
				if( ( ( m_availableX + ( m_defThumbSize / 2 ) ) > monitorRect.right ) ||
					( ( m_availableX - ( m_defThumbSize / 2 ) ) < monitorRect.left ) )
				{
					// ok, so the screen is full, lets try start again
					m_availableX = startX;	
					return;
				}
			}
		}
	}

	(*x) = m_availableX;
	(*y) = m_availableY;

	return;
}


void GetStartPosition( int mode, int* startx, int* starty )
{
	// this function will give the starting position
	// the values are based on various conditions
	// initially this is just the location and a small
	// margin. Later it will use a customized margin
	int calculatedPos = m_thumbMargin + ( m_defThumbSize / 2 );

	switch( mode )
	{
		case POS_TOPLEFTRIGHT:
		case POS_TOPLEFTDOWN:
			{
				(*startx) = _screenRect.left + calculatedPos;
				(*starty) = _screenRect.top + calculatedPos;
			}
			break;
		case POS_TOPRIGHTLEFT:
		case POS_TOPRIGHTDOWN:
			{
				(*startx) = _screenRect.right - calculatedPos;
				(*starty) = _screenRect.top + calculatedPos;
			}
			break;
		case POS_BOTTOMLEFTRIGHT:
		case POS_BOTTOMLEFTUP:
			{
				(*startx) = _screenRect.left + calculatedPos;
				(*starty) = _screenRect.bottom - calculatedPos;
			}
			break;
		case POS_BOTTOMRIGHTLEFT:
		case POS_BOTTOMRIGHTUP:
			{
				(*startx) = _screenRect.right - calculatedPos;
				(*starty) = _screenRect.bottom - calculatedPos;
			}
			break;
	}
}

bool CheckForHelp( )
{
	bool retval = false;

	FILE *stream;

	TCHAR filepath[MAX_PATH];

	_tcscpy( filepath, ( LPCTSTR )_deskconPath );

	if( PathRemoveFileSpec( filepath ) )
	{
		_tcscat( filepath, _T( "\\help\\index.html" ) );

		// try open the index html, if we can it exists
		if( ( stream = _tfopen( (LPCTSTR)filepath, _T("r") ) ) != NULL )
		{
			retval = true;
			fclose( stream );
		}
	}

	return retval;
}


BOOL ChangeWindowMessageFilter(UINT message, DWORD dwFlag)
{
	BOOL retval = FALSE;

	HINSTANCE hUser32 = (HINSTANCE)::LoadLibrary(_T("user32.dll"));

	if( hUser32 != NULL )
	{
		fnChangeWindowMessageFilter pfnFilter = (fnChangeWindowMessageFilter)::GetProcAddress(hUser32, _T("ChangeWindowMessageFilter"));
		
		if( pfnFilter != NULL )
		{
			retval = pfnFilter(message, dwFlag);
		}

		::FreeLibrary(hUser32);
	}

	return retval;
}

public:
//
// IDeskconWindow
//

STDMETHODIMP_( HWND )Initialize( )
{
	HWND wnd = Create( NULL, CWindow::rcDefault, _T(""), WS_POPUP, WS_EX_TOOLWINDOW );

	LoadSettings( );

	m_helpExists = CheckForHelp( );

	int i = LoadExclusionList( &_exclusionList );

	if( m_showNotifyIcon )
	{
		SetupNotifyIcon( );
	}

	// Create our task hide window, which we use to take focus when all windows are minimized
	CreateTaskHideWindow( &m_taskbarHideWindow );

	m_taskbarHideWindow->Initialize( m_hWnd );

	// lets try restore all windows
	//RestoreAllWindows( );

	if( LoadHook( ) )
	{
		ChangeNotifyIcon( false );
	}

	// now re-minimize previously minimized windows

	// start a timer that we can use to get rid of dead windows
	::SetTimer( m_hWnd, REFRESHTIMER, 2000, NULL );

	return wnd;
}

STDMETHODIMP_( void )Close ( void )
{
	PostMessage( WM_CLOSE, 0, 0 );
}
STDMETHODIMP_( HWND )GetHWND( void )
{
	return m_hWnd;
}


BOOL MyInfoEnumProc( HMONITOR hMonitor,  // handle to display monitor
					 HDC hdcMonitor,     // handle to monitor DC
					 LPRECT lprcMonitor, // monitor intersection rectangle
					 LPARAM dwData       )// data
{
	m_monitorCount++;

	if( m_monitorCount == m_monitorNum )
	{
		// set the startX and Y here
		_screenRect = (*lprcMonitor);

		return FALSE;
	}

	return TRUE;
}

BOOL RestoreWindowEWP( HWND hwnd )
{
	if( IsAppWindow( hwnd ) )
	{
		if( ::IsIconic( hwnd ) )
		{
			::ShowWindow( hwnd, SW_RESTORE );
		}
	}

	return TRUE;
}

BOOL MinimizeAllEWP( HWND hwnd )
{
	// minimize the window
	if( IsAppWindow( hwnd ) )
	{
		if( m_minimizeAllMode == SW_MINIMIZE )
		{
			if( !::IsIconic( hwnd ) )
			{
				::ShowWindow( hwnd, m_minimizeAllMode );
			}
		}
		/*else if( m_minimizeAllMode == SW_RESTORE )
		{
			if( ::IsIconic( hwnd ) )
			{

			}
		}*/

	}

	return TRUE;
}

};

//
// Callback functions
//
BOOL CALLBACK MinimizeAllEWPP( HWND hwnd, LPARAM lParam )
{
	return ( ( CDeskconWindow* )lParam )->MinimizeAllEWP( hwnd );
}

BOOL CALLBACK RestoreWindowEWPP( HWND hwnd, LPARAM lParam )
{
	return ( ( CDeskconWindow* )lParam )->RestoreWindowEWP( hwnd );
}

BOOL CALLBACK MyInfoEnumProcProxy( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
	return ( ( CDeskconWindow* )dwData )->MyInfoEnumProc( hMonitor, hdcMonitor, lprcMonitor, dwData );
}

//
// Creation Function
//
HRESULT CreateDeskconWindow( IDeskconWindow **ppDeskconWindow )
{
	return CComCreator< CComObject< CDeskconWindow > >::CreateInstance( NULL, __uuidof( IDeskconWindow ), ( void** ) ppDeskconWindow );
}
