//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ServiceCollection.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"
#include "deskcon.h"
#include "deskconinternal.h"
#include "fabric.h"

#include <vector>

extern CComPtr< IDebugWindow > _debugWindow;
extern HINSTANCE _hInstance;
extern HWND _mainHwnd;

ServiceCollection::ServiceCollection()
{
}

ServiceCollection::~ServiceCollection()
{
}

	
void ServiceCollection::RegisterService( IService* ppService )
{
	serviceVector.push_back( ppService );
}

template<class T>
T* ServiceCollection::GetService( void )
{
	T* retService = NULL;

	vector<IService*>::iterator it;

	for( it = serviceVector.begin( ); it != serviceVector.end( ); it++ )
	{
		retService = dynamic_cast<T *>( it );

		if( retService != NULL )
		{
			return retService;
		}
	}

	return retService;
}