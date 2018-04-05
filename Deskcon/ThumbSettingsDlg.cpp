//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ThumbSettingsDlg.cpp
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

extern HWND _mainHwnd;
extern UINT UWM_SETTINGCHANGED;

static COLORREF acrCustClr[16]; // array of custom colors 

class CThumbSettingsDlg : public CDialogImpl<CThumbSettingsDlg>,
							public CComObjectRootEx<CComSingleThreadModel>,
							public ITabPage
{
private:
	COLORREF m_flashColorBackground;

public:
	CThumbSettingsDlg( )
	{
	}

	~CThumbSettingsDlg( )
	{
	}

//
// IUnknown
//
BEGIN_COM_MAP(CThumbSettingsDlg)
	COM_INTERFACE_ENTRY(ITabPage)
END_COM_MAP()

enum { IDD = IDD_THUMBSETTINGS };

BEGIN_MSG_MAP(CThumbSettingsDlg)
	MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
	MESSAGE_HANDLER( WM_HSCROLL, OnSliderChange )
	MESSAGE_HANDLER( WM_CTLCOLORSTATIC, OnCtlColorStatic )
	COMMAND_HANDLER( IDC_SHOWICONS, BN_CLICKED, OnShowIconsClicked )
	COMMAND_HANDLER( IDC_THUMBSIZE, EN_KILLFOCUS, OnThumbSizeLooseFocus )
	COMMAND_HANDLER( IDC_FLASHCOLOR, BN_CLICKED, OnFlashColorClick )
	COMMAND_HANDLER( IDC_SIZEMODE, CBN_SELCHANGE, OnSizeModeChanged )
	COMMAND_HANDLER( IDC_WINDOWLEVEL, CBN_SELCHANGE, OnWindowLevelChanged )
	COMMAND_HANDLER( IDC_CUSTOMSHADOWS, BN_CLICKED, OnCustomShadowsClicked )
	COMMAND_HANDLER( IDC_USESNAPTO, BN_CLICKED, OnUseSnaptoClicked )
	MESSAGE_HANDLER( WM_CLOSE, OnClose )
END_MSG_MAP()

LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		long val = AddRef( );

		LPTSTR value = NULL;

		TCHAR buffer[10];
		HWND hControl;

		DWORD thumbSize;

		// Size
		GetDWORDSetting( _T( "Thumbnail Size" ), &thumbSize, 100 );

		/*if( thumbSize > 200 )
			thumbSize = 200;*/

		if( thumbSize < 10 )
			thumbSize = 10;

		_stprintf( buffer, _T( "%d\0" ), thumbSize );
		
		hControl = GetDlgItem( IDC_THUMBSIZE );

		::SendMessage( hControl, WM_SETTEXT, (WPARAM)NULL, (LPARAM)buffer );

		// sizing mode 0 = width, 1 = height, 2 = smart
		DWORD sizeMode;

		GetDWORDSetting( _T( "Sizing Mode" ), &sizeMode, 0 );

		if( sizeMode < 0 ) sizeMode = 0;
		if( sizeMode > 2 ) sizeMode = 2;

		hControl = GetDlgItem( IDC_SIZEMODE );

		::SendMessage( hControl, CB_INSERTSTRING, -1, (LPARAM)"Use Width" );

		::SendMessage( hControl, CB_INSERTSTRING, -1, (LPARAM)"Use Height" );

		::SendMessage( hControl, CB_INSERTSTRING, -1, (LPARAM)"Smart Sizing" );

		::SendMessage( hControl, CB_SETCURSEL, sizeMode, NULL );

		// window level 1 = bottom, 2 = normal, 3 = top
		DWORD windowLevel;

		GetDWORDSetting( _T( "Thumbnail Level" ), &windowLevel, 1 );
        
		windowLevel--; // the window level in the reg - 1 is the value in the combo

		if( windowLevel < 0 ) 
			windowLevel = 0;

		if( windowLevel > 2 ) 
			windowLevel = 2;

		hControl = GetDlgItem( IDC_WINDOWLEVEL );

		::SendMessage( hControl, CB_INSERTSTRING, -1, (LPARAM)"Pin To Desktop" );

		::SendMessage( hControl, CB_INSERTSTRING, -1, (LPARAM)"Normal" );

		::SendMessage( hControl, CB_INSERTSTRING, -1, (LPARAM)"Pin to Top" );

		::SendMessage( hControl, CB_SETCURSEL, windowLevel, NULL );

		// opacity
        DWORD globalOpacity;

		GetDWORDSetting( _T( "Global Opacity" ), &globalOpacity, 100 );

		if( globalOpacity > 100 )
			globalOpacity = 100;

		if( globalOpacity < 10 )
			globalOpacity = 10;

		globalOpacity = FindNearestTen( globalOpacity );

		_stprintf( buffer, _T( "%d%%\0" ), globalOpacity );
		
		hControl = GetDlgItem( IDC_OPACITYPERCENT );

		::SendMessage( hControl, WM_SETTEXT, (WPARAM)NULL, (LPARAM)buffer );

		hControl = GetDlgItem( IDC_OPACITYSLIDER );

		::SendMessage( hControl, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG ( 10, 100 ) );
		
		::SendMessage( hControl, TBM_SETTICFREQ, (WPARAM)10, NULL );

		::SendMessage( hControl, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)globalOpacity );

		// icons
		GetStringSetting( _T( "Show Icons" ), &value, _T( "True" ) );

		if( value )
		{
			if( _tcsicmp( value, _T( "true" ) ) == 0 )
			{
				hControl = GetDlgItem( IDC_SHOWICONS );
				
				::SendMessage( hControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );
			}

			delete [] value;
			value = NULL;
		}

		// flash color
		GetDWORDSetting( _T( "Flash Color" ), &m_flashColorBackground, RGB( 255, 0, 0 ) );

		// custom shadows
		GetStringSetting( _T( "Custom Shadows" ), &value, _T( "True" ), false );

		if( value )
		{
			if( _tcsicmp( value, _T( "true" ) ) == 0 )
			{
				hControl = GetDlgItem( IDC_CUSTOMSHADOWS );
				
				::SendMessage( hControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );
			}

			delete [] value;
			value = NULL;
		}

		// snap to
		GetStringSetting( _T( "Snapto" ), &value, _T( "True" ) );

		if( value )
		{
			if( _tcsicmp( value, _T( "true" ) ) == 0 )
			{
				hControl = GetDlgItem( IDC_USESNAPTO );
				
				::SendMessage( hControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );
			}

			delete [] value;
			value = NULL;
		}

		return 1;  // Let the system set the focus
	}
LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	HWND hControl = GetDlgItem( IDC_FLASHCOLOR );

	if( ( ( HWND )lParam ) == hControl )
	{
		SetDCBrushColor((HDC)wParam, m_flashColorBackground);
		return (LONG)GetStockObject( DC_BRUSH );
	}

	return FALSE;
}

LRESULT OnShowIconsClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TCHAR val[6];
	HWND hControl = GetDlgItem( IDC_SHOWICONS );

	if( ::SendMessage( hControl, BM_GETCHECK, NULL, NULL ) == BST_CHECKED )
	{
		_stprintf( val, _T( "True\0" ) );
	}
	else
	{
		_stprintf( val, _T( "False\0" ) );
	}

	SetStringSetting( _T( "Show Icons" ), val );
	UpdateSettingsString( SETTING_SHOWICONS, (LPCTSTR)&val );
	return 0;
}


LRESULT OnSliderChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TCHAR buffer[10];

	HWND hControl = NULL;

	hControl = GetDlgItem( IDC_OPACITYSLIDER );

	// get the new value
	DWORD val = (DWORD)::SendMessage( hControl, TBM_GETPOS, NULL, NULL );

	if( val > 100 )
		val = 100;

	if( val < 10 )
		val = 10;

	// pull it to the nearest 10 percent
	val = FindNearestTen( val );
	
	::SendMessage( hControl, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)val );

	// update the percentage display
	_stprintf( buffer, _T( "%d%%\0" ), val );
	
	hControl = GetDlgItem( IDC_OPACITYPERCENT );

	::SendMessage( hControl, WM_SETTEXT, (WPARAM)NULL, (LPARAM)buffer );
	
	// set the new value
	SetDWORDSetting( _T( "Global Opacity" ), val );
	UpdateSettingsDWORD( SETTING_OPACITY, val );

	return 0;
}


LRESULT OnThumbSizeLooseFocus(UINT uMsg, WPARAM wParam, HWND hControl, BOOL& bHandled)
{
	TCHAR buffer[20];
	DWORD val;

	int count = (int)::SendMessage( hControl, WM_GETTEXT, (WPARAM)20, (LPARAM)buffer );
	
	if( count > 0 )
	{
		val = _ttoi( buffer );

		/*if( val > 200 )
			val = 200;*/

		if( val < 10 )
			val = 10;

		// set the new value
		SetDWORDSetting( _T( "Thumbnail Size" ), val );
		UpdateSettingsDWORD( SETTING_THUMBNAILSIZE, val );
	}

	return 0;
}

LRESULT OnFlashColorClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HWND hControl = GetDlgItem( IDC_FLASHCOLOR );

	CHOOSECOLOR cc;                 // common dialog box structure 

	// Initialize CHOOSECOLOR 
	ZeroMemory( &cc, sizeof( cc ) );
	cc.lStructSize = sizeof( cc );

	cc.hwndOwner = m_hWnd;
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult = m_flashColorBackground;
	cc.Flags = CC_RGBINIT;//CC_FULLOPEN | CC_RGBINIT;
		
	if( ChooseColor( &cc ) == TRUE )
	{
		//hbrush = CreateSolidBrush(cc.rgbResult);
		m_flashColorBackground = cc.rgbResult;
		::InvalidateRect( hControl, NULL, TRUE );

		SetDWORDSetting( _T( "Flash Color" ), m_flashColorBackground );
		UpdateSettingsDWORD( SETTING_FLASHCOLOR, m_flashColorBackground );
	}

	return 0;
}







LRESULT OnClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	DestroyWindow( );

	return 0;  // Let the system set the focus
}
LRESULT OnSizeModeChanged(UINT uMsg, WPARAM wParam, HWND hControl, BOOL& bHandled)
{
	// get the new default monitor
	DWORD sel = (DWORD)::SendMessage( hControl, CB_GETCURSEL, NULL, NULL );

	if( sel != CB_ERR )
	{
		// set the new value
		SetDWORDSetting( _T( "Sizing Mode" ), sel);
		UpdateSettingsDWORD( SETTING_SIZINGMODE, sel );
	}

	return 0;
}
LRESULT OnWindowLevelChanged(UINT uMsg, WPARAM wParam, HWND hControl, BOOL& bHandled)
{
	// get the new default monitor
	DWORD sel = (DWORD)::SendMessage( hControl, CB_GETCURSEL, NULL, NULL );

	if( sel != CB_ERR )
	{
		// set the new value
		SetDWORDSetting( _T( "Thumbnail Level" ), sel + 1 );
		UpdateSettingsDWORD( SETTING_WINDOWLEVEL, sel + 1 );
	}

	return 0;
}
LRESULT OnCustomShadowsClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TCHAR val[6];
	HWND hControl = GetDlgItem( IDC_CUSTOMSHADOWS );

	if( ::SendMessage( hControl, BM_GETCHECK, NULL, NULL ) == BST_CHECKED )
	{
		_stprintf( val, _T( "True\0" ) );
	}
	else
	{
		_stprintf( val, _T( "False\0" ) );
	}

	SetStringSetting( _T( "Custom Shadows" ), val );
	UpdateSettingsString( SETTING_CUSTOMSHADOWS, (LPCTSTR)&val );

	return 0;
}

LRESULT OnUseSnaptoClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TCHAR val[6];
	HWND hControl = GetDlgItem( IDC_USESNAPTO );

	if( ::SendMessage( hControl, BM_GETCHECK, NULL, NULL ) == BST_CHECKED )
	{
		_stprintf( val, _T( "True\0" ) );
	}
	else
	{
		_stprintf( val, _T( "False\0" ) );
	}

	SetStringSetting( _T( "Snapto" ), val );
	UpdateSettingsString( SETTING_SNAPTO, (LPCTSTR)&val );

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

HRESULT CreateThumbSettingsTabPage( ITabPage **ppTabPage )
{
	return CComCreator< CComObject< CThumbSettingsDlg > >::CreateInstance( NULL, __uuidof( ITabPage ), ( void** ) ppTabPage );
}