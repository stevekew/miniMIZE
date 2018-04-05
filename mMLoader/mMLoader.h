//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// mMLoader.h
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

#include "resource.h"

#define UWM_SHUTDOWN_MMLOADER_MSG _T( "UWM_SHUTDOWN_MMLOADER_MSG-A5B6F7F9-6BFD-4e4a-84F7-71626DFEFA27" )

struct _declspec( uuid( "447C5A4D-EAF1-4534-84AF-8B1F9432E55D" ) ) IMMLoaderWindow : IUnknown
{
	STDMETHOD_( HWND, Initialize ) ( void ) = 0;
	STDMETHOD_( void, Close ) ( void ) = 0;
	STDMETHOD_( HWND, GetHWND ) ( void ) = 0;
};

HRESULT CreateLoaderWindow( IMMLoaderWindow **ppLoaderWindow );