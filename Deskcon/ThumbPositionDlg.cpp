//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ThumbPositionDlg.cpp
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
#include <gdiplus.h>

extern HWND _mainHwnd;
extern UINT UWM_SETTINGCHANGED;
extern int _monitorCount;
extern TCHAR _deskconPath[MAX_PATH];
extern CComPtr< IDebugWindow > _debugWindow;

class CThumbPositionDlg : public CDialogImpl<CThumbPositionDlg>,
							public CComObjectRootEx<CComSingleThreadModel>,
							public ITabPage
{
private:
	//CComPtr< IBmpBar > bmpBar;
	Gdiplus::Bitmap *m_barImage;

public:
	CThumbPositionDlg( )
	{
	}

	~CThumbPositionDlg( )
	{
	}

//
// IUnknown
//
BEGIN_COM_MAP(CThumbPositionDlg)
	COM_INTERFACE_ENTRY(ITabPage)
END_COM_MAP()

enum { IDD = IDD_POSITIONSETTINGS };

BEGIN_MSG_MAP(CThumbPositionDlg)
	MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
	MESSAGE_HANDLER( WM_DRAWITEM, OnDrawItem )
	MESSAGE_HANDLER( WM_CLOSE, OnClose )
	//COMMAND_HANDLER( IDC_MONITORLIST, CBN_SELCHANGE, OnMonitorChanged )
	COMMAND_HANDLER( IDC_TOPLEFTRIGHT, BN_CLICKED, OnPositionChanged )
	COMMAND_HANDLER( IDC_TOPRIGHTLEFT, BN_CLICKED, OnPositionChanged )
	COMMAND_HANDLER( IDC_BOTTOMLEFTRIGHT, BN_CLICKED, OnPositionChanged )
	COMMAND_HANDLER( IDC_BOTTOMRIGHTLEFT, BN_CLICKED, OnPositionChanged )
	COMMAND_HANDLER( IDC_TOPLEFTDOWN, BN_CLICKED, OnPositionChanged )
	COMMAND_HANDLER( IDC_TOPRIGHTDOWN, BN_CLICKED, OnPositionChanged )
	COMMAND_HANDLER( IDC_BOTTOMLEFTUP, BN_CLICKED, OnPositionChanged )
	COMMAND_HANDLER( IDC_BOTTOMRIGHTUP, BN_CLICKED, OnPositionChanged )
	COMMAND_HANDLER( IDC_POSMARGIN, EN_KILLFOCUS, OnPositionMarginLooseFocus )
END_MSG_MAP()

LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	AddRef( );

	LPTSTR value = NULL;

	TCHAR buffer[10];
	HWND hControl;

	LoadPicture( _T( "\\images\\position.png" ) ); 

	DWORD dwValue;

	GetDWORDSetting( _T( "Positioning" ), &dwValue, POS_TOPLEFTRIGHT );

	// check the registry value
	if( dwValue < POS_TOPLEFTRIGHT )
		dwValue = POS_TOPLEFTRIGHT;

	if( dwValue > POS_BOTTOMRIGHTUP )
		dwValue = POS_BOTTOMRIGHTUP;

	hControl = GetHWNDForPosition( dwValue );

	::SendMessage( hControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );
	
	GetDWORDSetting( _T( "Margin" ), &dwValue, 20 );

	if( dwValue < 0 )
		dwValue = 0;

	_stprintf( buffer, _T( "%d" ), dwValue );

	hControl = GetDlgItem( IDC_POSMARGIN );

	::SetWindowText( hControl, buffer );
	
	return 1;  // Let the system set the focus
}

LRESULT OnDrawItem( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	LRESULT retval = 0;

	if( wParam == IDC_SCREENPIC )
	{
		LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) lParam;

		// draw our bitmap here
		Gdiplus::Graphics g( lpDrawItem->hDC );

		g.DrawImage( m_barImage, 0, 0, m_barImage->GetWidth( ), m_barImage->GetHeight( ) );

		retval = 1;
	}

	return retval;
}


LRESULT OnClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	DestroyWindow( );

	return 0;  // Let the system set the focus
}

//LRESULT OnMonitorChanged(UINT uMsg, WPARAM wParam, HWND hControl, BOOL& bHandled)
//{
//	// get the new default monitor
//	DWORD sel = (DWORD)::SendMessage( hControl, CB_GETCURSEL, NULL, NULL );
//
//	if( sel != CB_ERR )
//	{
//		// set the new value
//		SetDWORDSetting( _T( "Default Monitor" ), sel + 1 );
//		UpdateSettingsDWORD( SETTING_DEFAULTMONITOR, sel+1 );
//	}
//
//	return 0;
//}
//
LRESULT OnPositionChanged( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	int pos = GetPositionForNID( wID );
	// store the value and update the setting
	SetDWORDSetting( _T( "Positioning" ), pos );

	UpdateSettingsDWORD( SETTING_POSITION, pos );

	return 0;
}

LRESULT OnPositionMarginLooseFocus(UINT uMsg, WPARAM wParam, HWND hControl, BOOL& bHandled)
{
	TCHAR buffer[20];
	DWORD val;

	int count = (int)::SendMessage( hControl, WM_GETTEXT, (WPARAM)20, (LPARAM)buffer );
	
	if( count > 0 )
	{
		val = _ttoi( buffer );

		if( val < 0 )
			val = 0;

		// set the new value
		SetDWORDSetting( _T( "Margin" ), val );
		UpdateSettingsDWORD( SETTING_MARGIN, val );
	}

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

BOOL LoadPicture( LPCTSTR filename )
{
	TCHAR filepath[MAX_PATH];

	_tcscpy( filepath, ( LPCTSTR )_deskconPath );

	if( PathRemoveFileSpec( filepath ) )
	{
		_tcscat( filepath, filename );

		// first try to load the image
#ifdef UNICODE
		m_barImage = new Gdiplus::Bitmap( filepath );
#else
		m_barImage = new Gdiplus::Bitmap( CA2W( filepath ) );
#endif
		if( m_barImage )
			return TRUE;
	}

	return FALSE;
}


int GetPositionForNID( int nID )
{
	int pos = POS_TOPLEFTRIGHT;

	switch( nID )
	{
		case IDC_TOPLEFTRIGHT:
			pos = POS_TOPLEFTRIGHT;
			break;
		case IDC_TOPRIGHTLEFT:
			pos = POS_TOPRIGHTLEFT;
			break;
		case IDC_BOTTOMLEFTRIGHT:
			pos = POS_BOTTOMLEFTRIGHT;
			break;
		case IDC_BOTTOMRIGHTLEFT:
			pos = POS_BOTTOMRIGHTLEFT;
			break;
		case IDC_TOPLEFTDOWN:
			pos = POS_TOPLEFTDOWN;
			break;
		case IDC_TOPRIGHTDOWN:
			pos = POS_TOPRIGHTDOWN;
			break;
		case IDC_BOTTOMLEFTUP:
			pos = POS_BOTTOMLEFTUP;
			break;
		case IDC_BOTTOMRIGHTUP:
			pos = POS_BOTTOMRIGHTUP;
			break;
	}

	return pos;
}

HWND GetHWNDForPosition( int position )
{
	HWND control = NULL;
	int nID = IDC_TOPLEFTRIGHT;

	switch( position )
	{
		case POS_TOPLEFTRIGHT:
			nID = IDC_TOPLEFTRIGHT;
			break;
		case POS_TOPRIGHTLEFT:
			nID = IDC_TOPRIGHTLEFT;
			break;
		case POS_BOTTOMLEFTRIGHT:
			nID = IDC_BOTTOMLEFTRIGHT;
			break;
		case POS_BOTTOMRIGHTLEFT:
			nID = IDC_BOTTOMRIGHTLEFT;
			break;
		case POS_TOPLEFTDOWN:
			nID = IDC_TOPLEFTDOWN;
			break;
		case POS_TOPRIGHTDOWN:
			nID = IDC_TOPRIGHTDOWN;
			break;
		case POS_BOTTOMLEFTUP:
			nID = IDC_BOTTOMLEFTUP;
			break;
		case POS_BOTTOMRIGHTUP:
			nID = IDC_BOTTOMRIGHTUP;
			break;
	}

	control = GetDlgItem( nID );

	return control;
}
};

HRESULT CreateThumbPositionTabPage( ITabPage **ppTabPage )
{
	return CComCreator< CComObject< CThumbPositionDlg > >::CreateInstance( NULL, __uuidof( ITabPage ), ( void** ) ppTabPage );
}