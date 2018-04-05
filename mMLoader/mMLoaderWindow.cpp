//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// mMLoaderWindow.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"
#include "resource.h"       // main symbols
#include "mMLoader.h"

//
// CMMLoaderWindow - main miniMIZE window class
//
class CMMLoaderWindow : public CWindowImpl< CMMLoaderWindow >,
						public CComObjectRootEx<CComSingleThreadModel>,
						public IMMLoaderWindow
{
private:
	UINT UWM_SHUTDOWN_MMLOADER;

public:
	CMMLoaderWindow()
	{
		UWM_SHUTDOWN_MMLOADER = ::RegisterWindowMessage( UWM_SHUTDOWN_MMLOADER_MSG );
	}

	~CMMLoaderWindow( )
	{
	}

DECLARE_WND_CLASS( _T( "miniMIZE_Loader_Window" ) )

//
// IUnknown
//
BEGIN_COM_MAP( CMMLoaderWindow )
	COM_INTERFACE_ENTRY( IMMLoaderWindow )
END_COM_MAP( )

//
// WndProc
//
BEGIN_MSG_MAP(CMMLoaderWindow)
	MESSAGE_HANDLER( WM_NCCREATE, OnNCCreate )
	MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
	MESSAGE_HANDLER( WM_CLOSE, OnClose )
	MESSAGE_HANDLER( UWM_SHUTDOWN_MMLOADER, OnShutdownLoader )
END_MSG_MAP()

LRESULT OnNCCreate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	AddRef( );
	return 1;
}
LRESULT OnDestroy( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	return 1;
}
LRESULT OnClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	Shutdown( );
	return 1;
}

LRESULT OnShutdownLoader( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	SendMessage( WM_CLOSE );
	return 1;
}

virtual void OnFinalMessage( HWND hWnd )
{
	Release( );
}
public:
//
// IDeskconWindow
//

STDMETHODIMP_( HWND )Initialize( )
{
	HWND wnd = Create( NULL, CWindow::rcDefault, _T(""), WS_POPUP, WS_EX_TOOLWINDOW );

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

private:
void Shutdown( )
{
	DestroyWindow( ); 
	PostQuitMessage(0);
}

};

HRESULT CreateLoaderWindow( IMMLoaderWindow **ppLoaderWindow )
{
	return CComCreator< CComObject< CMMLoaderWindow > >::CreateInstance( NULL, __uuidof( IMMLoaderWindow ), ( void** ) ppLoaderWindow );
}
