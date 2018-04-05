//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// SplashWindow.cpp
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
#include "..\version.h"

#include <gdiplus.h>
// CAboutDeskcon

extern TCHAR _deskconPath[MAX_PATH];
extern CComPtr< IDebugWindow > _debugWindow;

#define SPLASHTIMER 100

class CSplashWindow : public CWindowImpl<CSplashWindow>,
						public CComObjectRootEx<CComSingleThreadModel>,
						public ISplashWindow
{
private:
	Gdiplus::Bitmap *m_splashImage;
	bool m_showing;

public:
	CSplashWindow()
	{
		m_splashImage = NULL;
		m_showing = true;
	}

	~CSplashWindow()
	{
		if( m_splashImage != NULL )
		{
			delete m_splashImage;

			m_splashImage = NULL;
		}
	}

	DECLARE_WND_CLASS( _T( "miniMIZE_Splash_Class" ) )

//
// IUnknown
//
BEGIN_COM_MAP(CSplashWindow)
	COM_INTERFACE_ENTRY(ISplashWindow)
END_COM_MAP()

BEGIN_MSG_MAP(CSplashWindow)
	MESSAGE_HANDLER( WM_NCCREATE, OnNCCreate )
	MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
	MESSAGE_HANDLER( WM_CLOSE, OnClose )
	MESSAGE_HANDLER( WM_PAINT, OnPaint )
	MESSAGE_HANDLER( WM_TIMER, OnTimer )
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

LRESULT OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	PAINTSTRUCT ps; 
	HDC hdc;

	hdc = BeginPaint( &ps ); 

	// draw our bitmap here
	Gdiplus::Graphics g( hdc );

	g.DrawImage( m_splashImage, 0, 0, m_splashImage->GetWidth( ), m_splashImage->GetHeight( ) );
		
	EndPaint( &ps );

	return 0;
}

LRESULT OnTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	SAFEDEBUGWRITELINE( _debugWindow, _T( "Splash Timer" ) )

	if( wParam == SPLASHTIMER )
	{
		::KillTimer( m_hWnd, SPLASHTIMER );

		SendMessage( WM_CLOSE );

		m_showing = false;
	}

    return 0;
}

virtual void OnFinalMessage( HWND hWnd )
{
	Release( );
}

public:
	STDMETHODIMP_( BOOL )Initialize( LPCTSTR filename )
	{
		TCHAR filepath[MAX_PATH];

		_tcscpy( filepath, ( LPCTSTR )_deskconPath );

		if( PathRemoveFileSpec( filepath ) )
		{

			_tcscat( filepath, filename );

			// first try to load the image
#ifdef UNICODE
			m_splashImage = new Gdiplus::Bitmap( filepath );
#else
			m_splashImage = new Gdiplus::Bitmap( CA2W( filepath ) );
#endif
			if( m_splashImage )
				return TRUE;
		}

		return FALSE;
	}

	STDMETHODIMP_( BOOL )Show( void )
	{
		if( m_splashImage != NULL )
		{
			RECT r;

			r.top=10000;
			r.left=10000;
			r.bottom=10000 + m_splashImage->GetHeight( );
			r.right=10000 + m_splashImage->GetWidth( );

			HWND h = Create( NULL, r, _T("miniMIZE_Splash"), WS_POPUPWINDOW , WS_EX_TOOLWINDOW );//, WS_POPUP, WS_EX_TOOLWINDOW );

			ShowWindow( SW_SHOW );

			RedrawWindow( NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW );

			CenterWindow( );

			SetAlwaysOnTop( true );

			::SetTimer( m_hWnd,  SPLASHTIMER, 2000, NULL ); 

			return TRUE;
		}

		return FALSE;
	}

	STDMETHODIMP_( BOOL )Showing( void )
	{
		return m_showing;
	}

void Shutdown( )
{
	SAFEDEBUGWRITELINE( _debugWindow, _T( "Shutting Down" ) )

	DestroyWindow( ); 
}

void SetAlwaysOnTop( bool set )
{
	HWND pos = HWND_NOTOPMOST;


	if( set )
	{
		pos = HWND_TOPMOST;
	}
	
	SetWindowPos( pos, &CWindow::rcDefault, SWP_NOSIZE | SWP_NOMOVE );
}

};

HRESULT CreateSplashWindow( ISplashWindow **ppSplashWindow )
{
	return CComCreator< CComObject< CSplashWindow > >::CreateInstance( NULL, __uuidof( ISplashWindow ), ( void** ) ppSplashWindow );
}

