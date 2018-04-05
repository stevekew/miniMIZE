//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// VistaThumbnail.cpp
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
#include "ThumbnailBase.h"

#include <dwmapi.h>

class CVistaThumbnail : public CThumbnailBase<CVistaThumbnail>
{
	HTHUMBNAIL m_thumbnail;
public:
	CVistaThumbnail( )
	{
		m_thumbnail = NULL; 
	}

	~CVistaThumbnail( )
	{
		if( m_thumbnail != NULL )
		{
			DwmUnregisterThumbnail( m_thumbnail );

			m_thumbnail = NULL;
		}
	}

public:
STDMETHODIMP_( BOOL )Show( void )
{
	BOOL retval = FALSE;

	if( ShowWindow( SW_SHOW ) )
	{
		// lets just set our window level again for good measure
		SetWindowLevel( m_windowLevel );

		retval = TRUE;
	}

	return retval;
}

//
// Overrides
//
protected:
virtual bool OnWindowMaximized()
{
	DWM_THUMBNAIL_PROPERTIES dskThumbProps;
	dskThumbProps.dwFlags = DWM_TNP_VISIBLE;
	
	//use window frame and client area
	dskThumbProps.fVisible = FALSE;

	//display the thumbnail
	HRESULT hr = DwmUpdateThumbnailProperties(m_thumbnail,&dskThumbProps);

	if (SUCCEEDED(hr))
	{
		//do more things
		return true;
	}

	return false;
}

virtual bool OnWindowMinimized()
{
	//Set thumbnail properties for use
	DWM_THUMBNAIL_PROPERTIES dskThumbProps;
	dskThumbProps.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE;
	
	RECT dest = {0,0,m_width,m_height};

	//use window frame and client area
	dskThumbProps.fVisible = TRUE;
	dskThumbProps.rcDestination = dest;

	//display the thumbnail
	HRESULT hr = DwmUpdateThumbnailProperties(m_thumbnail,&dskThumbProps);

	if (SUCCEEDED(hr))
	{
		//do more things
		return true;
	}
	
	return false;
}

virtual bool OnInitialized()
{
	HRESULT hr = S_OK;

	//Register Thumbnail
	hr = DwmRegisterThumbnail(m_hWnd, m_thumbnailWindow, &m_thumbnail);

	if (SUCCEEDED(hr))
	{
		//display thumbnail using DwmUpdateThumbnailProperties
		//destination rectangle size
		RECT dest = {0,0,m_width,m_height};

		//Set thumbnail properties for use
		DWM_THUMBNAIL_PROPERTIES dskThumbProps;
		dskThumbProps.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE | DWM_TNP_SOURCECLIENTAREAONLY;
		
		//use window frame and client area
		dskThumbProps.fSourceClientAreaOnly = FALSE;
		dskThumbProps.fVisible = TRUE;
		dskThumbProps.opacity = m_opacity * 2.55;
		dskThumbProps.rcDestination = dest;

		//display the thumbnail
		hr = DwmUpdateThumbnailProperties(m_thumbnail,&dskThumbProps);

		if (SUCCEEDED(hr))
		{
			//do more things
			return true;
		}

		DwmUnregisterThumbnail( m_thumbnail );
		
		m_thumbnail = NULL;
	}

	return false;
}

};

HRESULT CreateVistaThumbWindow( IThumbWindow **ppThumbWindow )
{
	return CComCreator< CComObject< CVistaThumbnail > >::CreateInstance( NULL, __uuidof( IThumbWindow ), ( void** ) ppThumbWindow );
}
