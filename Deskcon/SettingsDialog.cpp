//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// SettingsDialog.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"
#include "resource.h"       // main symbols
#include "deskcon.h"
#include "deskconinternal.h"

// CSettingsDialog
class CSettingsDialog : public CDialogImpl<CSettingsDialog>,
						public CComObjectRootEx<CComSingleThreadModel>,
						public ISettingsDialog
{
private:
	HWND m_currentTabPage;
	
public:
	CSettingsDialog()
	{
		m_currentTabPage = NULL;
	}

	~CSettingsDialog()
	{
	}

//
// IUnknown
//
BEGIN_COM_MAP(CSettingsDialog)
	COM_INTERFACE_ENTRY(ISettingsDialog)
END_COM_MAP()

enum { IDD = IDD_SETTINGS };

BEGIN_MSG_MAP(CSettingsDialog)
	MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
	COMMAND_HANDLER( IDOK, BN_CLICKED, OnClickedOK )
	NOTIFY_HANDLER( IDC_SETTINGSTAB, TCN_SELCHANGE , OnTabSelChange )
END_MSG_MAP()

// private functions
LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetupTabControl( );

	CentreWindow( );

	SetWindowPos( HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	
	return 1;  // Let the system set the focus
}

LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if( m_currentTabPage )
	{
		::ShowWindow( m_currentTabPage, SW_HIDE );

		::SendMessage( m_currentTabPage, WM_CLOSE, NULL, NULL );

		m_currentTabPage = NULL;
	}

	EndDialog(wID);
	return 0;
}

LRESULT OnTabSelChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	ChangeTabSel( );
	return 0;
}

public:
	STDMETHODIMP_( BOOL )ShowModal( HWND hWndParent )
	{
		return DoModal( hWndParent );
	}

	STDMETHODIMP_( void )BringToFront( void )
	{
		SetForegroundWindow( m_hWnd );
	}


private:

void SetupTabControl( )
{
	HWND hTabControl = GetDlgItem( IDC_SETTINGSTAB );
	TCITEM tci = { 0 };

	tci.mask = TCIF_TEXT; 
    tci.pszText = _T( "General" );

	TabCtrl_InsertItem( hTabControl, SP_GENERALSETTINGS, &tci );

	tci.pszText = _T( "Thumbnails" );

	TabCtrl_InsertItem( hTabControl, SP_THUMBNAILSETTINGS, &tci );

	tci.pszText = _T( "Position" );

	TabCtrl_InsertItem( hTabControl, SP_THUMBNAILPOSITION, &tci );
	
	tci.pszText = _T( "Hotkeys" );

	TabCtrl_InsertItem( hTabControl, SP_HOTKEYS, &tci );

	tci.pszText = _T( "Exclusions" );

	TabCtrl_InsertItem( hTabControl, SP_EXCLUSIONLIST, &tci );

	ChangeTabSel( );
}

void ChangeTabSel( )
{
	int currentTabVal = 0;
	CComPtr< ITabPage > tabPage;

	DWORD dwDlgBase = GetDialogBaseUnits(); 

	int cxMargin = LOWORD(dwDlgBase) / 4; 
    int cyMargin = HIWORD(dwDlgBase) / 8; 

	HWND hTabControl = GetDlgItem( IDC_SETTINGSTAB );

	currentTabVal = TabCtrl_GetCurSel( hTabControl );

	if( m_currentTabPage )
	{
		::ShowWindow( m_currentTabPage, SW_HIDE );

		::SendMessage( m_currentTabPage, WM_CLOSE, NULL, NULL );

		m_currentTabPage = NULL;
	}

	switch( currentTabVal )
	{
		case SP_GENERALSETTINGS:
			{
				CreateGeneralSettingsTabPage( &tabPage );
			}
			break;
		case SP_THUMBNAILSETTINGS:
			{
				CreateThumbSettingsTabPage( &tabPage );
			}
			break;
		case SP_THUMBNAILPOSITION:
			{
				CreateThumbPositionTabPage( &tabPage );
			}
			break;
		case SP_HOTKEYS:
			{
				CreateHotkeysTabPage( &tabPage );
			}
			break;
		case SP_EXCLUSIONLIST:
			{
				CreateExclusionsTabPage( &tabPage );
			}
			break;
	}

	if( tabPage )
	{
		RECT r = { 0 };

		::GetWindowRect( hTabControl, &r );

		OffsetRect(&r, cxMargin - r.left, cyMargin - r.top);

		r.top += 20;

		tabPage->Show( hTabControl, r );

		m_currentTabPage = tabPage->GetHWND( );
	}
}
void CentreWindow( )
{
	// wanna put it in the center of the screen
	HMONITOR monitor = ::MonitorFromWindow( m_hWnd, MONITOR_DEFAULTTONEAREST );
	RECT monitorRect = { 0 };
    MONITORINFO monitorInfo = { 0 };

    monitorInfo.cbSize = sizeof( monitorInfo );

    GetMonitorInfo( monitor, &monitorInfo );

    monitorRect = monitorInfo.rcMonitor;

	RECT r = { 0 };

	GetWindowRect( &r );
	
	int x = ( ( monitorRect.right - monitorRect.left ) / 2 ) - ( ( r.right - r.left ) / 2 ); 
	int y = ( ( monitorRect.bottom - monitorRect.top ) / 2 ) - ( ( r.bottom - r.top ) / 2 ); 

	SetWindowPos( HWND_TOP, x, y, 0, 0, SWP_NOSIZE );
}
};

HRESULT CreateSettingsDialog( ISettingsDialog **ppSettingsDialog )
{
	return CComCreator< CComObject< CSettingsDialog > >::CreateInstance( NULL, __uuidof( ISettingsDialog ), ( void** ) ppSettingsDialog );
}