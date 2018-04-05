//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// GeneralSettingsDlg.cpp
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
#include "deskcon.h"
#include "deskconinternal.h"
#include "settings.h"
#include <commdlg.h>

#define WINDOWS_AUTOSTART _T( "Software\\Microsoft\\Windows\\CurrentVersion\\Run" )

extern TCHAR _deskconPath[MAX_PATH];
extern int _monitorCount;
extern HWND _mainHwnd;
extern UINT UWM_SETTINGCHANGED;

class CGeneralSettingsDlg : public CDialogImpl<CGeneralSettingsDlg>,
						public CComObjectRootEx<CComSingleThreadModel>,
						public ITabPage
{
public:
	CGeneralSettingsDlg( )
	{
	}

	~CGeneralSettingsDlg( )
	{
	}

//
// IUnknown
//
BEGIN_COM_MAP(CGeneralSettingsDlg)
	COM_INTERFACE_ENTRY(ITabPage)
END_COM_MAP()

enum { IDD = IDD_GENERALSETTINGS };

BEGIN_MSG_MAP(CGeneralSettingsDlg)
	MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
	COMMAND_HANDLER( IDC_AUTOSTART, BN_CLICKED, OnAutoStartClicked )
	COMMAND_HANDLER( IDC_SHOWNOTIFYICON, BN_CLICKED, OnShowNotifyClicked )
	COMMAND_HANDLER( IDC_MONITORLIST, CBN_SELCHANGE, OnMonitorChanged )
	COMMAND_HANDLER( IDC_CLICKSTYLE, CBN_SELCHANGE, OnClickStyleChanged )
	COMMAND_HANDLER( IDC_CTRLDRAG, BN_CLICKED, OnCtrlDragClicked )
	COMMAND_HANDLER( IDC_HIDETASKBAR, BN_CLICKED, OnHideTaskbarClicked )
	MESSAGE_HANDLER( WM_CLOSE, OnClose )
END_MSG_MAP()

LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	AddRef( );

	LPTSTR value = NULL;

	TCHAR buffer[10];
	HWND hControl;

	// autostart
	GetStringFromRegistry( HKEY_CURRENT_USER, WINDOWS_AUTOSTART, _T( "miniMIZE" ), &value, NULL, false );

	if( value )
	{
		if( _tcsicmp( value, ( LPCTSTR ) _deskconPath ) == 0 )
		{
			hControl = GetDlgItem( IDC_AUTOSTART );
			
			::SendMessage( hControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );
		}

		delete [] value;
		value = NULL;
	}

	DWORD monitor;

	GetDWORDSetting( _T( "Default Monitor" ), &monitor, 1 );

	// monitors
	if( _monitorCount > 1 )
	{
		hControl = GetDlgItem( IDC_MONITORLIST );

		for( int i = 1; i <= _monitorCount; i++ )
		{
			_stprintf( buffer, _T( "Monitor %d\0" ), i );

			::SendMessage( hControl, CB_ADDSTRING, NULL, (LPARAM)buffer );
		}

		::SendMessage( hControl, CB_SETCURSEL, (monitor - 1), NULL );
		::EnableWindow( hControl, TRUE );
	}

	// notify icon
	GetStringSetting( _T( "Show In Tray" ), &value, _T( "True" ) );

	if( value )
	{
		if( _tcsicmp( value, _T( "true" ) ) == 0 )
		{
			hControl = GetDlgItem( IDC_SHOWNOTIFYICON );
			
			::SendMessage( hControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );
		}

		delete [] value;
		value = NULL;
	}

	// Click Style - 0 = Single 1 = Double
	DWORD click = 1;

	GetDWORDSetting( _T( "Click Style" ), &click, CS_DOUBLECLICK );

	hControl = GetDlgItem( IDC_CLICKSTYLE );

	::SendMessage( hControl, CB_INSERTSTRING, -1, (LPARAM)_T( "Single Click" ) );
	::SendMessage( hControl, CB_INSERTSTRING, -1, (LPARAM)_T( "Double Click" ) );

	::SendMessage( hControl, CB_SETCURSEL, click, NULL );

	// Ctrl-Drag
	GetStringSetting( _T( "Ctrl Drag" ), &value, _T( "False" ) );

	hControl = GetDlgItem( IDC_CTRLDRAG );

	if( value )
	{
		if( _tcsicmp( value, _T( "true" ) ) == 0 )
		{
			::SendMessage( hControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );
		}

		delete [] value;
		value = NULL;
	}

	if( click == CS_SINGLECLICK )
	{
		::SendMessage( hControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );
		::EnableWindow( hControl, FALSE );
	}
	else
	{
		::EnableWindow( hControl, TRUE );
	}

	// hide taskbar buttons
	GetStringSetting( _T( "Hide Taskbar Buttons" ), &value, _T( "True" ) );

	hControl = GetDlgItem( IDC_HIDETASKBAR );

	if( value )
	{
		if( _tcsicmp( value, _T( "true" ) ) == 0 )
		{
			::SendMessage( hControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );
		}

		delete [] value;
		value = NULL;
	}

	return 1;  // Let the system set the focus
}

LRESULT OnAutoStartClicked( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	HWND hControl = NULL;

	hControl = GetDlgItem( IDC_AUTOSTART );

	if( ::SendMessage( hControl, BM_GETCHECK, NULL, NULL ) == BST_CHECKED )
	{
		WriteStringToRegistry( HKEY_CURRENT_USER, WINDOWS_AUTOSTART, _T( "miniMIZE" ), ( LPCTSTR ) _deskconPath );
	}
	else
	{
		DeleteValueFromRegistry( HKEY_CURRENT_USER, WINDOWS_AUTOSTART, _T( "miniMIZE" ) );
	}

	return 0;
}
LRESULT OnShowNotifyClicked( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	TCHAR val[6];
	HWND hControl = GetDlgItem( IDC_SHOWNOTIFYICON );

	if( ::SendMessage( hControl, BM_GETCHECK, NULL, NULL ) == BST_CHECKED )
	{
		_stprintf( val, _T( "True\0" ) );
	}
	else
	{
		_stprintf( val, _T( "False\0" ) );
	}

	SetStringSetting( _T( "Show In Tray" ), val );

	UpdateSettingsString( SETTING_NOTIFYICON, (LPCTSTR)&val );

	return 0;
}

LRESULT OnMonitorChanged(UINT uMsg, WPARAM wParam, HWND hControl, BOOL& bHandled)
{
	// get the new default monitor
	DWORD sel = (DWORD)::SendMessage( hControl, CB_GETCURSEL, NULL, NULL );

	if( sel != CB_ERR )
	{
		// set the new value
		SetDWORDSetting( _T( "Default Monitor" ), sel + 1 );
		UpdateSettingsDWORD( SETTING_DEFAULTMONITOR, sel+1 );
	}

	return 0;
}


LRESULT OnClickStyleChanged(UINT uMsg, WPARAM wParam, HWND hControl, BOOL& bHandled)
{
	LPTSTR value = NULL;

	DWORD sel = (DWORD)::SendMessage( hControl, CB_GETCURSEL, NULL, NULL );

	if( sel != CB_ERR )
	{
		// set the new value
		SetDWORDSetting( _T( "Click Style" ), sel );
		UpdateSettingsDWORD( SETTING_CLICKSTYLE, sel );

		HWND clickControl = GetDlgItem( IDC_CTRLDRAG );

		if( sel == CS_SINGLECLICK )
		{
			::SendMessage( clickControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );
			::EnableWindow( clickControl, FALSE );
		}
		else
		{
			::EnableWindow( clickControl, TRUE );

			GetStringSetting( _T( "Ctrl Drag" ), &value, _T( "False" ) );

			if( value )
			{
				if( _tcsicmp( value, _T( "true" ) ) == 0 )
				{
					::SendMessage( clickControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );
				}
				else
				{
					::SendMessage( clickControl, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)NULL );
				}

				delete [] value;
				value = NULL;
			}
		}
	}

	return 0;
}


LRESULT OnClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	DestroyWindow( );

	return 0;  // Let the system set the focus
}

LRESULT OnCtrlDragClicked( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	TCHAR val[6];

	HWND hControl = GetDlgItem( IDC_CTRLDRAG );

	if( ::SendMessage( hControl, BM_GETCHECK, NULL, NULL ) == BST_CHECKED )
	{
		_stprintf( val, _T( "True\0" ) );
	}
	else
	{
		_stprintf( val, _T( "False\0" ) );
	}

	SetStringSetting( _T( "Ctrl Drag" ), val );

	UpdateSettingsString( SETTING_CTRLDRAG, (LPCTSTR)&val );

	return 0;
}

LRESULT OnHideTaskbarClicked( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	TCHAR val[6];

	HWND hControl = GetDlgItem( IDC_HIDETASKBAR );

	if( ::SendMessage( hControl, BM_GETCHECK, NULL, NULL ) == BST_CHECKED )
	{
		_stprintf( val, _T( "True\0" ) );
	}
	else
	{
		_stprintf( val, _T( "False\0" ) );
	}

	SetStringSetting( _T( "Hide Taskbar Buttons" ), val );

	UpdateSettingsString( SETTING_HIDETASKBAR, (LPCTSTR)&val );

	return 0;
}

virtual void OnFinalMessage( HWND hWnd )
{
	Release( );
}

public:
STDMETHODIMP_( void )Show( HWND parent, RECT r  )
{
	Create( parent, r );

	ShowWindow( SW_SHOW );

	SetLocation( r );
}
STDMETHODIMP_( HWND )GetHWND( void )
{
	return m_hWnd;
}



private:
void SetLocation( RECT r )
{
	SetWindowPos( HWND_TOP, &r, SWP_NOSIZE );
}

void UpdateSettingsDWORD( int settingid, DWORD value )
{
	::SendMessage( _mainHwnd, UWM_SETTINGCHANGED, (LPARAM) settingid, (WPARAM) value );
}

void UpdateSettingsString( int settingid, LPCTSTR value )
{
	::SendMessage( _mainHwnd, UWM_SETTINGCHANGED, (LPARAM) settingid, (WPARAM) value );
}


};

HRESULT CreateGeneralSettingsTabPage( ITabPage **ppTabPage )
{
	return CComCreator< CComObject< CGeneralSettingsDlg > >::CreateInstance( NULL, __uuidof( ITabPage ), ( void** ) ppTabPage );
}