#include "stdafx.h"

#include <assert.h>

#include "SplitColorToLayer.hpp"


void diffuseDistances(signed _width, signed _height, 
					  std::vector<signed>& nearestDistance, std::vector<unsigned>& nearestPixel)
{
	// first pass
	for(signed j=0; j<_height; ++j)
	{
		for(signed i=0; i<_width; i++)
		{
			signed offset = j*_width + i;
			signed distTmp, offsetTmp;
			if((j-1)>=0 && (i-1)>=0
				&& (distTmp=nearestDistance[offsetTmp = ((j-1)*_width+(i-1))]+4)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }

			if((j-1)>=0 && (i)>=0
				&& (distTmp=nearestDistance[offsetTmp = ((j-1)*_width+i)]+3)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
			
			if((j-1)>=0 && (i+1)<_width 
				&& (distTmp=nearestDistance[offsetTmp = ((j-1)*_width+(i+1))]+4)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
			
			if((j)>=0 && (i-1)>=0 
				&& (distTmp=nearestDistance[offsetTmp = ((j)*_width+(i-1))]+3)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
		}
	}
	// second pass
	for(signed j=(_height-1); j>=0; --j)
	{
		for(signed i=(_width-1); i>=0; --i)
		{
			signed offset = j*_width + i;
			signed distTmp, offsetTmp;
			if((j+1)<_height && (i-1)>=0 
				&& (distTmp=nearestDistance[offsetTmp = ((j+1)*_width+(i-1))]+4)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
			
			if((j+1)<_height && (i)>=0 
				&& (distTmp=nearestDistance[offsetTmp = ((j+1)*_width+i)]+3)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
			
			if((j+1)<_height && (i+1)<_width 
				&& (distTmp=nearestDistance[offsetTmp = ((j+1)*_width+(i+1))]+4)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
			
			if((j)>=0 && (i+1)<_width 
				&& (distTmp=nearestDistance[offsetTmp = ((j)*_width+(i+1))]+3)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
		}
	}	
}

void splitBuffer(uint8_t* _outColorsRGBA8, const unsigned _widthLR, const unsigned _heightLR,
				 const std::set<unsigned>& _layer, 
				 const std::vector<unsigned>& _clusterIdHR, const uint8_t* _inColorRGBA8HR, const unsigned _widthHR, const unsigned _heightHR)
{
	unsigned _stride = _widthHR;
	unsigned widthFactor = _widthHR / _widthLR;
	unsigned heightFactor = _heightHR / _heightLR;

	// offset + distanceSq
	std::vector<unsigned> nearestPixel;
	std::vector<signed> nearestDistance;
	nearestPixel.resize(_widthHR*_heightHR, ~0);
	nearestDistance.resize(_widthHR*_heightHR, SHRT_MAX); // UINT_MAX may generate overflow

	// init values: 0 for internal values
	unsigned pixelCount = 0;
	for(unsigned j=0; j<_heightHR; ++j)
	{
		for(unsigned i=0; i<_widthHR; i++)
		{
			unsigned offset = ((j * _stride) + i);
			unsigned cluster = _clusterIdHR[offset];
			if(_layer.find(cluster) != _layer.end())
			{
				nearestPixel[j*_widthHR+i] = j*_stride+i;
				nearestDistance[j*_widthHR+i] = 0;
				pixelCount++;
			}
		}
	}

	if(pixelCount == 0)
		return;

	/*if(pixelCount == _widthHR*_heightHR)
	{
		unsigned offset = (_height/2)*_stride + (_width/2);
		_color[0] = _colorRGBA8Local[offset*4+0];
		_color[1] = _colorRGBA8Local[offset*4+1];
		_color[2] = _colorRGBA8Local[offset*4+2];
		_color[3] = 0xff;
		return;
	}*/

	diffuseDistances(_widthHR, _heightHR, 
		nearestDistance, nearestPixel);

	signed dStep = std::min(widthFactor, heightFactor);
	signed oStep = std::max(widthFactor, heightFactor) - dStep;
	signed distanceMax =  4 * dStep + 3 * oStep;

	for(unsigned j=0; j<_heightLR; ++j)
	{
		for(unsigned i=0; i<_widthLR; ++i)
		{
			unsigned outOffset = j*_widthLR+i;

			unsigned inOffsetHR = (j*_stride*heightFactor)+i*widthFactor;
			//const uint8_t* localPixels = &_inColorRGBA8HR[inOffsetHR*4];
			//const unsigned* localClusterIds = &_clusterIdHR[inOffsetHR];

			unsigned distance = nearestDistance[inOffsetHR];
			unsigned pixelOffset = nearestPixel[inOffsetHR];

			_outColorsRGBA8[outOffset*4 + 0] = _inColorRGBA8HR[pixelOffset*4+0];
			_outColorsRGBA8[outOffset*4 + 1] = _inColorRGBA8HR[pixelOffset*4+1];
			_outColorsRGBA8[outOffset*4 + 2] = _inColorRGBA8HR[pixelOffset*4+2];

			distance = ((distance) * 0xff / (distanceMax)); //normalize
			distance = distance>0xff ? 0xff : distance; // clamp
			distance = 0xff - distance; // invert
			_outColorsRGBA8[outOffset*4 + 3] = distance;
		}
	}
}

bool splitBuffer(std::vector<uint8_t*>& _outColorsRGBA8, 
				 const std::vector<std::set<unsigned>>& _layers, 
				 const unsigned _widthLR, const unsigned _heightLR,
				 const std::vector<unsigned>& _clusterIdHR, const uint8_t* _inColorRGBA8HR, const unsigned _widthHR, const unsigned _heightHR)
{
	if(_outColorsRGBA8.size() != _layers.size())
		return false;

	for(unsigned layerId = 0; layerId<_layers.size(); layerId++)
	{
		const std::set<unsigned>& layer = _layers[layerId];
		uint8_t* outbuffer = _outColorsRGBA8[layerId];

		splitBuffer(outbuffer, _widthLR, _heightLR,
			layer,
			_clusterIdHR, _inColorRGBA8HR, _widthHR, _heightHR);
	}
	return true;
}



