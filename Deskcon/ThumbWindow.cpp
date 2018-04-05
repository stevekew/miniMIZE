//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ThumbWindow.cpp
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

class CThumbWindow : public CThumbnailBase<CThumbWindow>
{
private:
bool CaptureWindow( HWND window, Gdiplus::Bitmap *bitmap )
{
	Gdiplus::Graphics graphics( bitmap );

	HDC hdc = graphics.GetHDC( );

	PrintWindow( window, hdc, NULL );

	graphics.ReleaseHDC( hdc );

	return true;
}

bool BltWindow( HWND window, Gdiplus::Bitmap *bitmap )
{
	HDC hDC = ::GetWindowDC( window );
	Gdiplus::Graphics bltGraphics( bitmap );

	RECT rect = {0};
	::GetClientRect(window, &rect);

	TITLEBARINFO tbi = {0};
	tbi.cbSize = sizeof( tbi );

	GetTitleBarInfo( window, &tbi );

	int y = 0;

	if( ( tbi.rcTitleBar.bottom > 0 ) && (tbi.rcTitleBar.bottom < rect.bottom ) )
		y = tbi.rcTitleBar.bottom;

	HDC bltDC = bltGraphics.GetHDC( );

	::BitBlt(bltDC, rect.left, y, rect.right - rect.left, rect.bottom - rect.top, hDC, rect.left, rect.top, SRCCOPY );

	bltGraphics.ReleaseHDC( bltDC );

	::ReleaseDC( window, hDC );

	return true;
}


//
// create thumbnail
//
void TakeWindowSnapshot( HWND hWndView )
{
	if( m_windowBitmap )
		delete m_windowBitmap;

	HDC hDCMem = NULL;
	int sizeMode = m_sizeMode;

	RECT rect;
	HBITMAP hBmp;

	::GetWindowRect(hWndView, & rect);

	Gdiplus::Bitmap newBmp( rect.right - rect.left, rect.bottom - rect.top, PixelFormat32bppARGB );

	CaptureWindow( hWndView, &newBmp );

	if( !m_ignoreBlack )
	{
		if( IsBitmapAvgColorBlack( &newBmp ) )
		{
			SAFEDEBUGWRITELINE( _debugWindow, _T( "Too Much Black" ) )

			BltWindow( hWndView, &newBmp );
		}
	}

	m_windowBitmap = new Gdiplus::Bitmap( m_width, m_height, PixelFormat32bppARGB);

	Gdiplus::Graphics g( m_windowBitmap );

	g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

	g.SetInterpolationMode( Gdiplus::InterpolationModeHighQualityBicubic );
	
	g.DrawImage( &newBmp , 0, 0, m_width, m_height );

	// if we're not drawing icons, the iconBmp will be NULL
	if( iconBmp != NULL )
	{
		g.DrawImage( iconBmp, m_width - 35, m_height - 35 );
	}

	m_snapshotTaken = true;
}

bool IsBitmapAvgColorBlack( Gdiplus::Bitmap *bitmap )
{
	int iWidth = bitmap->GetWidth();
	int iHeight = bitmap->GetHeight();
	int numPixels = iWidth * iHeight;
	int iBlackCount = 0;
	bool retval = true;
	
	Gdiplus::BitmapData* bitmapData = new Gdiplus::BitmapData;
	Gdiplus::Color color;
	
	Gdiplus::Rect rect(0, 0, iWidth, iHeight);

	// Lock a 5x3 rectangular portion of the bitmap for reading.
	bitmap->LockBits(
		&rect,
		Gdiplus::ImageLockModeRead,
		PixelFormat32bppARGB,
		bitmapData);

	int iStride = bitmapData->Stride;

	// Display the hexadecimal value of each pixel in the 5x3 rectangle.
	UINT* pixels = (UINT*)bitmapData->Scan0;

	for(UINT row = 0; row < iHeight; ++row)
	{
		for(UINT col = 0; col < iWidth; ++col)
		{
			if( pixels[row * iStride / 4 + col] == 0xff000000)
			{
				iBlackCount++;
			}
		}
	}

	bitmap->UnlockBits(bitmapData);
	
	delete bitmapData;
	
	if( ( numPixels <= 0 ) || ( ( ( iBlackCount * 100 ) / numPixels ) < 60  ) )
	{
		retval = false;
	}

	return retval;
}


//
// Overrides
//
protected:
virtual bool OnWindowMinimized()
{
	TakeWindowSnapshot( m_thumbnailWindow );

	return true;
}

virtual bool OnInitialized()
{
	TakeWindowSnapshot( m_thumbnailWindow );

	return true;
}

};

HRESULT CreateThumbWindow( IThumbWindow **ppThumbWindow )
{
	return CComCreator< CComObject< CThumbWindow > >::CreateInstance( NULL, __uuidof( IThumbWindow ), ( void** ) ppThumbWindow );
}