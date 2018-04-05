//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// Deskcon.h
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

#pragma once

#include "resource.h"

//
// Interface definitions
//

struct __declspec( uuid( "6CDFD8EC-CDAE-47E0-9667-77773CAC9C9A" ) ) IDebugWindow : IUnknown
{
	STDMETHOD_( void, WriteLine )( LPCTSTR buffer ) = 0;
	STDMETHOD_( void, Write )( LPCTSTR buffer ) = 0;
	STDMETHOD_( BOOL, CloseWindow )( void ) = 0;
	STDMETHOD_( BOOL, OpenWindow )( void ) = 0;
};

HRESULT CreateDebugWindow( IDebugWindow **ppDebugWindow );

struct _declspec( uuid( "3F60B923-8059-47ee-9ED4-1656084BA4BD" ) ) IAboutDeskcon : IUnknown
{
	STDMETHOD_( BOOL, ShowModal )( HWND hWndParent ) = 0;
};

HRESULT CreateAboutDeskconWindow( IAboutDeskcon **ppAboutWindow );

struct _declspec( uuid( "290BA159-18CB-4c33-BD1E-CEC30ECBC2DB" ) ) IDeskconWindow : IUnknown
{
	STDMETHOD_( HWND, Initialize ) ( void ) = 0;
	STDMETHOD_( void, Close ) ( void ) = 0;
	STDMETHOD_( HWND, GetHWND ) ( void ) = 0;
};

HRESULT CreateDeskconWindow( IDeskconWindow **ppDeskconWindow );

struct _declspec( uuid( "44A6736A-A227-43DC-A9D1-1F76A577E37B" ) ) INotifyIcon : IUnknown
{
	STDMETHOD_( BOOL, Show ) ( BOOL showIcon ) = 0;
	STDMETHOD_( void, SetText ) ( LPCTSTR tooltipText ) = 0;
	STDMETHOD_( void, SetIcon ) ( HICON icon ) = 0;
	STDMETHOD_( void, ShowBalloon ) ( LPCTSTR balloonTitle, LPCTSTR balloonText, DWORD style ) = 0;
	STDMETHOD_( void, Initialize ) ( HWND mainWindow, UINT callbackMessage, HICON icon, LPCTSTR tooltipText ) = 0;
};

HRESULT CreateNotifyIcon( INotifyIcon **ppNotifyIcon );

struct _declspec( uuid( "D025B07B-0239-43E6-B98E-4C2A7371C596" ) ) ISettingsDialog : IUnknown
{
	STDMETHOD_( BOOL, ShowModal )( HWND hWndParent ) = 0;
	STDMETHOD_( void, BringToFront )( void ) = 0;
};

HRESULT CreateSettingsDialog( ISettingsDialog **ppSettingsDialog );

struct _declspec( uuid( "E0C0E9CF-B660-450E-9CE6-613BE0F96184" ) ) IThumbWindow : IUnknown
{
	STDMETHOD_( BOOL, Initialize ) ( HWND hWndView, int x, int y, bool ignoreBlack, bool fullInit ) = 0;
	STDMETHOD_( BOOL, Initialize ) ( HWND hWndView, int x, int y, bool ignoreBlack ) = 0;
	STDMETHOD_( bool, IgnoreBlack ) ( void ) = 0;
	STDMETHOD_( BOOL, Show )( void ) = 0;
	STDMETHOD_( void, Hide ) ( void ) = 0;
	STDMETHOD_( BOOL, IsVisible ) ( void ) = 0;
	STDMETHOD_( void, Close ) ( void ) = 0;
	STDMETHOD_( void, GetLocation ) ( int *x, int *y ) = 0;
	STDMETHOD_( void, SetLocation ) ( int x, int y ) = 0;
	STDMETHOD_( HWND, GetHWND ) ( void ) = 0;
	STDMETHOD_( HWND, GetMonitoredWindow ) ( void ) = 0;
	STDMETHOD_( void, Flash ) ( void ) = 0;
	STDMETHOD_( void, SetSize ) ( int size ) = 0;
	STDMETHOD_( void, SetOpacity ) ( int opacity ) = 0;
	STDMETHOD_( void, SetShowIcons ) ( bool show ) = 0;
	STDMETHOD_( void, SetSizeMode ) ( int sizemode ) = 0;
	STDMETHOD_( void, SetWindowLevel ) ( int level ) = 0;
	STDMETHOD_( void, SetFlashColor ) ( COLORREF color ) = 0;
	STDMETHOD_( void, SetDelphiApp ) ( bool delphiApp ) = 0;
	STDMETHOD_( void, SetClickStyle ) ( int clickstyle ) = 0;
	STDMETHOD_( void, SetUseCtrlDrag ) ( bool ctrldrag ) = 0;
	STDMETHOD_( void, SetDrawCustomShadows ) ( bool custom ) = 0;
	STDMETHOD_( void, ShowTaskbarButton ) ( bool show ) = 0;
	STDMETHOD_( RECT, GetThumbnailRect ) ( void ) = 0;
};

HRESULT CreateThumbWindow( IThumbWindow **ppThumbWindow );

struct _declspec( uuid( "57A02B70-6FB9-4F1A-8520-B800D64B6DC8" ) ) IPassportCtl : IUnknown
{
	STDMETHOD_( BOOL, Initialize ) ( UINT pollrate ) = 0;
	STDMETHOD_( BOOL, Initialize ) ( void ) = 0;
	STDMETHOD_( BOOL, Start ) ( void ) = 0;
	STDMETHOD_( BOOL, Stop )( void ) = 0;
	STDMETHOD_( BOOL, SetPollRate ) ( UINT pollrate ) = 0;
};

HRESULT CreatePassportControl( IPassportCtl **ppPassportCtl );

struct _declspec( uuid( "BF2551F9-C11F-484A-AA33-72A5D0E1FAC5" ) ) ITabPage : IUnknown
{
	STDMETHOD_( void, Show ) ( HWND parent, RECT r ) = 0;
	STDMETHOD_( HWND, GetHWND ) ( void ) = 0;
};

HRESULT CreateGeneralSettingsTabPage( ITabPage **ppTabPage );
HRESULT CreateThumbSettingsTabPage( ITabPage **ppTabPage );
HRESULT CreateThumbPositionTabPage( ITabPage **ppTabPage );
HRESULT CreateExclusionsTabPage( ITabPage **ppTabPage );
HRESULT CreateHotkeysTabPage( ITabPage **ppTabPage );

struct _declspec( uuid( "C59EF963-EAF9-4545-B9F5-205A67A9E517" ) ) ITaskHideWindow : IUnknown
{
	STDMETHOD_( HWND, Initialize ) ( HWND parent ) = 0;
	STDMETHOD_( void, Close ) ( void ) = 0;
};

HRESULT CreateTaskHideWindow( ITaskHideWindow **ppTaskHideWindow );

struct _declspec( uuid( "27A333C2-058E-46C2-A371-5F86CB4E615E" ) ) ISplashWindow : IUnknown
{
	STDMETHOD_( BOOL, Initialize )( LPCTSTR filename ) = 0;
	STDMETHOD_( BOOL, Show )( void ) = 0;
	STDMETHOD_( BOOL, Showing )( void ) = 0;
};

HRESULT CreateSplashWindow( ISplashWindow **ppSplashWindow );

struct _declspec( uuid( "D0EEC1AF-1089-4FC8-BFD0-33B4FECF842E" ) ) IBmpBar : IUnknown
{
	STDMETHOD_( BOOL, Initialize )( LPCTSTR filename ) = 0;
	STDMETHOD_( BOOL, Show )( HWND parent, RECT r ) = 0;
	STDMETHOD_( void, SetPosition )( int x, int y ) = 0;

};

HRESULT CreateBmpBar( IBmpBar **ppBmpBar );

struct _declspec( uuid( "9F54B01A-950D-4c19-9371-6600FAAB7104" ) ) ISettings : IUnknown
{
	STDMETHOD_( bool, GetBoolSetting )( LPCTSTR settingName, bool defaultValue  ) = 0;
	STDMETHOD_( int, GetIntSetting )( LPCTSTR settingName, int defaultValue  ) = 0;
	STDMETHOD_( LPTSTR, GetStringSetting )( LPCTSTR settingName, LPTSTR defaultValue  ) = 0;
	STDMETHOD_( double, GetDoubleSetting )( LPCTSTR settingName, double defaultValue  ) = 0;
	STDMETHOD_( void, RefreshSettings )( void ) = 0;
};

HRESULT CreateRegistrySettings( ISettings **ppSettingsClass );

struct _declspec( uuid( "F3D78AFF-F365-44e7-B6BA-79A5CA5B3F08" ) ) IService : IUnknown
{
};

//struct _declspec( uuid( "C0100225-B29B-4346-AD32-0A4A5309E561" ) ) IServiceCollection : IUnknown
//{
//	STDMETHOD_( void, RegisterService )( IService* ppService ) = 0;
//	template<class T> STDMETHOD_( T*, GetService ) ( void ) = 0;
//};