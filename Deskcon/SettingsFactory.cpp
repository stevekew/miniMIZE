//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// SettingsFactory.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"
#include "deskcon.h"
#include "deskconinternal.h"

HRESULT CreateSettingsClass( ISettings **ppSettingsClass, int classType )
{
	switch( classType )
	{
		case SETTINGS_CLASS_REGISTRY:
			return CreateRegistrySettings( ppSettingsClass );
			break;
	};
	
	return E_FAIL;
}