#include "stdafx.h"

#include "Shrink2DBuffer.hpp"


bool shrink2DBuffer4UB(
	uint8_t* _outColorRGBA8, 
	const unsigned _outWidth, const unsigned _outHeight,
	const uint8_t* _inColorRGBA8, 
	const unsigned _inWidth, const unsigned _inHeight)
{
	unsigned widthFactor = _inWidth / _outWidth;
	if(_outWidth*widthFactor!=_inWidth) return false;
	unsigned heightFactor = _inHeight / _outHeight;
	if(_outHeight*heightFactor!=_inHeight) return false;

	for(unsigned j=0; j<_outHeight; ++j)
	{
		for(unsigned i=0; i<_outWidth; ++i)
		{
			unsigned inOffset = (j*heightFactor) * _inWidth + (i* widthFactor);
			unsigned outOffset = j * _outWidth + i ;
			_outColorRGBA8[outOffset*4 + 0] = _inColorRGBA8[inOffset*4 + 0];
			_outColorRGBA8[outOffset*4 + 1] = _inColorRGBA8[inOffset*4 + 1];
			_outColorRGBA8[outOffset*4 + 2] = _inColorRGBA8[inOffset*4 + 2];
			_outColorRGBA8[outOffset*4 + 3] = _inColorRGBA8[inOffset*4 + 3];
		}
	}
	return true;
}

bool shrink2DBufferUI(
	unsigned* _outValues, 
	const unsigned _outWidth, const unsigned _outHeight,
	const unsigned* _inValues, 
	const unsigned _inWidth, const unsigned _inHeight)
{
	unsigned widthFactor = _inWidth / _outWidth;
	if(_outWidth*widthFactor!=_inWidth) return false;
	unsigned heightFactor = _inHeight / _outHeight;
	if(_outHeight*heightFactor!=_inHeight) return false;

	for(unsigned j=0; j<_outHeight; ++j)
	{
		for(unsigned i=0; i<_outWidth; ++i)
		{
			unsigned inOffset = (j*heightFactor) * _inWidth + (i* widthFactor);
			unsigned outOffset = j * _outWidth + i ;
			_outValues[outOffset] = _inValues[inOffset];
		}
	}
	return true;
}

