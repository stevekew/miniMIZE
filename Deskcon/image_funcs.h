// filename:		layer.cpp
// Description:		
// Author:			Andreas Verhoeven
// Last modified:	19 January 2004, 20:00
// Version:			1
// Comments:
//					<none>
// 
//	(c) copyright Andreas Verhoeven 2004
//	Do not modify, reproduce or use this file without my permission!

#ifndef IMAGE_FUNCS__H_201039459
#define IMAGE_FUNCS__H_201039459

#include <math.h>
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

Bitmap* ConvoluteDim(Bitmap * pSrc, bool bHorizontal,int* filterVector, UINT dim, int denominator);
Bitmap* GetBlur(Bitmap* pSrc, REAL radius);

#endif//IMAGE_FUNCS__H_201039459
