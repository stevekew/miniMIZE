//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// NotifyIcon.cpp
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

#include "stdafx.h"       // main symbols
#include "Deskcon.h"

#include <shellapi.h>

// CDebugWindow
class CNotifyIcon : public CComObjectRootEx<CComMultiThreadModel>,
					public INotifyIcon
{
private:
	// data variables
	HWND m_mainWindow;
	HICON m_icon;
	char m_tooltip[64];
	UINT m_callback;

public:
	CNotifyIcon()
	{
		m_mainWindow = NULL;
		//m_tooltip = NULL;
		m_icon = NULL;
		m_callback = NULL;
	}
	
	virtual ~CNotifyIcon( )
	{
		m_mainWindow = NULL;
	}

//
// IUnknown
//
BEGIN_COM_MAP(CNotifyIcon)
	COM_INTERFACE_ENTRY(INotifyIcon)
END_COM_MAP()

public:
	//
	// INotifyIcon
	//

	STDMETHODIMP_( BOOL )Show( BOOL showIcon )
	{
		NOTIFYICONDATA niData; 
		ZeroMemory(&niData,sizeof(NOTIFYICONDATA));

		// this is standard
		niData.cbSize = sizeof(NOTIFYICONDATA);
		niData.hWnd = m_mainWindow;
		niData.uID = 0;
		
		if( showIcon )
		{
			niData.uCallbackMessage = m_callback;	
			niData.uFlags = NIF_MESSAGE;

			if( m_icon )
			{
				niData.hIcon = m_icon;
				niData.uFlags |= NIF_ICON;
			}

			if( _tcslen( ( LPCTSTR ) m_tooltip ) > 0 )
			{
				_tcscpy( niData.szTip, ( LPCTSTR ) m_tooltip );
				niData.uFlags |= NIF_TIP;
			}

			Shell_NotifyIcon( NIM_ADD, &niData );
		}
		else
		{
			Shell_NotifyIcon( NIM_DELETE, &niData );
		}
	
		return TRUE;
	}

	STDMETHODIMP_( void )SetText( LPCTSTR tooltipText )
	{

	}

	STDMETHODIMP_( void )SetIcon( HICON icon )
	{
		NOTIFYICONDATA niData; 
		ZeroMemory(&niData,sizeof(NOTIFYICONDATA));

		// this is standard
		niData.cbSize = sizeof(NOTIFYICONDATA);
		niData.hWnd = m_mainWindow;
		niData.uID = 0;
		
		if( icon )
		{
			m_icon = icon;
			
			niData.hIcon = m_icon;
			niData.uFlags |= NIF_ICON;

			Shell_NotifyIcon( NIM_MODIFY, &niData );
		}
	}

	STDMETHODIMP_( void )ShowBalloon( LPCTSTR balloonTitle, LPCTSTR balloonText, DWORD style )
	{
	}

	STDMETHODIMP_( void )Initialize( HWND mainWindow, UINT callbackMessage, HICON icon, LPCTSTR tooltipText )
	{
		m_mainWindow = mainWindow;

		m_icon = icon;
		
		if( tooltipText != NULL )//m_tooltip = new LPWSTR
			_tcsncpy( ( LPTSTR ) m_tooltip, tooltipText, 64 );

		m_callback = callbackMessage;
	}

};

HRESULT CreateNotifyIcon( INotifyIcon **ppNotifyIcon )
{
	return CComCreator< CComObject< CNotifyIcon > >::CreateInstance( NULL, __uuidof( INotifyIcon ), ( void** ) ppNotifyIcon );
}
