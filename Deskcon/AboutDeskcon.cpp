//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// AboutDeskcon.cpp
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

