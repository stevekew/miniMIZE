//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// BmpBar.cpp
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

class CBmpBar : public CDialogImpl<CBmpBar>,
						public CComObjectRootEx<CComSingleThreadModel>,
						public IBmpBar
{
private:
	Gdiplus::Bitmap *m_barImage;
	int xPos;
	int yPos;

public:
	CBmpBar()
	{
		m_barImage = NULL;
		xPos = 0;
		yPos = 0;
	}

	~CBmpBar()
	{
		if( m_barImage != NULL )
		{
			delete m_barImage;

			m_barImage = NULL;
		}
	}

//
// IUnknown
//
BEGIN_COM_MAP(CBmpBar)
	COM_INTERFACE_ENTRY(IBmpBar)
END_COM_MAP()

	enum { IDD = IDD_BMPBAR };

BEGIN_MSG_MAP(CBmpBar)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_PAINT, OnPaint );
END_MSG_MAP()

// private functions
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 1;  // Let the system set the focus
	}

LRESULT OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	PAINTSTRUCT ps; 
	HDC hdc;

	hdc = BeginPaint( &ps ); 

	// draw our bitmap here
	Gdiplus::Graphics g( hdc );

	g.DrawImage( m_barImage, 0, 0, m_barImage->GetWidth( ), m_barImage->GetHeight( ) );
		
	EndPaint( &ps );

	return 0;
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
		m_barImage = new Gdiplus::Bitmap( filepath );
#else
		m_barImage = new Gdiplus::Bitmap( CA2W( filepath ) );
#endif
		if( m_barImage )
			return TRUE;
	}

	return FALSE;
}

STDMETHODIMP_( void )SetPosition( int x, int y )
{
	xPos = x;
	yPos = y;

	if( ( m_hWnd ) && ( m_barImage ) )
	{
		SetWindowPos( NULL, xPos, yPos, m_barImage->GetWidth( ), m_barImage->GetHeight( ), SWP_NOACTIVATE | SWP_NOZORDER ); 
	}
}

STDMETHODIMP_( BOOL )Show( HWND parent, RECT r )
{
	if( m_barImage != NULL )
	{
		Create( parent, r );

		ShowWindow( SW_SHOW );

		SetWindowPos( HWND_BOTTOM, xPos, yPos, m_barImage->GetWidth( ), m_barImage->GetHeight( ), SWP_NOACTIVATE );

		return TRUE;
	}

	return FALSE;
}
};

HRESULT CreateBmpBar( IBmpBar **ppBmpBar )
{
	return CComCreator< CComObject< CBmpBar > >::CreateInstance( NULL, __uuidof( IBmpBar ), ( void** ) ppBmpBar );
}

