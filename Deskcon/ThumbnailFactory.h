//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// Deskcon.h
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

#include "resource.h"
#include "Deskcon.h"

// 
// Thumbnail forward definition
//
//
// Thumbnail Factory
//
HRESULT CreateThumbnail( IThumbWindow **ppThumbWindow, int thumbnailType )
{
	if( thumbnailType == VISTA_THUMBNAIL )
	{
		return CreateVistaThumbWindow(ppThumbWindow);
	}
	else if (thumbnailType == PNG_THUMBNAIL )
	{
		// do a png thumbnail
	}
	else
	{
		return CreateThumbWindow(ppThumbWindow);
	}
}