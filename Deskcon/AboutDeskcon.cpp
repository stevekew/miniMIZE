//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// AboutDeskcon.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"
#include "resource.h"       // main symbols
#include "deskcon.h"
#include "..\version.h"

// CAboutDeskcon

class CAboutDeskcon : public CDialogImpl<CAboutDeskcon>,
						public CComObjectRootEx<CComSingleThreadModel>,
						public IAboutDeskcon
{
private:
	CComPtr< IBmpBar > bmpBar;
public:
	CAboutDeskcon()
	{
		bmpBar = NULL;
	}

	~CAboutDeskcon()
	{
		if( bmpBar != NULL )
		{
			//bmpBar->Close( );
			bmpBar = NULL;
		}
	}

//
// IUnknown
//
BEGIN_COM_MAP(CAboutDeskcon)
	COM_INTERFACE_ENTRY(IAboutDeskcon)
END_COM_MAP()

	enum { IDD = IDD_ABOUTDESKCON };

BEGIN_MSG_MAP(CAboutDeskcon)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

// private functions
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//CDialogImpl<CAboutDeskcon>::OnInitDialog(uMsg, wParam, lParam, bHandled);
		// set the version number
		CreateBmpBar( &bmpBar );

		RECT r = { 0 };

		::GetWindowRect( m_hWnd, &r );

		if( bmpBar->Initialize( _T( "\\images\\about.png" ) ) )
		{
			bmpBar->Show( m_hWnd, r );
		}

		HWND versionHwnd = GetDlgItem( IDC_VERSIONSTRING );

		if( versionHwnd )
		{
			::SendMessage( versionHwnd, WM_SETTEXT, NULL, (LPARAM)VERSION_STRING );
		}

		CentreWindow( );

		return 1;  // Let the system set the focus
	}

	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}



public:
	STDMETHODIMP_( BOOL )ShowModal( HWND hWndParent )
	{
		return DoModal( hWndParent );
	}

	LRESULT OnStnClickedVersionstring(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

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

HRESULT CreateAboutDeskconWindow( IAboutDeskcon **ppAboutWindow )
{
	return CComCreator< CComObject< CAboutDeskcon > >::CreateInstance( NULL, __uuidof( IAboutDeskcon ), ( void** ) ppAboutWindow );
}

