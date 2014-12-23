#include "stdafx.h"

#include <assert.h>

#include "SplitColorToLayer.hpp"


void diffuseDistances(unsigned _width, unsigned _height, 
					  std::vector<signed>& nearestDistance, std::vector<unsigned>& nearestPixel)
{
	// first pass
	for(unsigned j=0; j<_height; ++j)
	{
		for(unsigned i=0; i<_width; i++)
		{
			unsigned offset = j*_width + i, offsetTmp;
			signed distTmp;
			if(j>0 && i>0
				&& (distTmp=nearestDistance[offsetTmp = ((j-1)*_width+(i-1))]+4)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }

			if(j>0
				&& (distTmp=nearestDistance[offsetTmp = ((j-1)*_width+i)]+3)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
			
			if(j>0 && (i+1)<_width 
				&& (distTmp=nearestDistance[offsetTmp = ((j-1)*_width+(i+1))]+4)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
			
			if(i>0 
				&& (distTmp=nearestDistance[offsetTmp = ((j)*_width+(i-1))]+3)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
		}
	}
	// second pass
	for(unsigned j=(_height-1); j!=UINT_MAX; --j)
	{
		for(unsigned i=(_width-1); i!=UINT_MAX; --i)
		{
			unsigned offset = j*_width + i, offsetTmp;
			signed distTmp;
			if((j+1)<_height && i>0 
				&& (distTmp=nearestDistance[offsetTmp = ((j+1)*_width+(i-1))]+4)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
			
			if((j+1)<_height 
				&& (distTmp=nearestDistance[offsetTmp = ((j+1)*_width+i)]+3)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
			
			if((j+1)<_height && (i+1)<_width 
				&& (distTmp=nearestDistance[offsetTmp = ((j+1)*_width+(i+1))]+4)<nearestDistance[offset])
			{nearestDistance[offset] = distTmp; nearestPixel[offset] = nearestPixel[offsetTmp]; }
			
			if((i+1)<_width 
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

	std::vector<signed> nearestInsideDistance;
	nearestInsideDistance.resize(_widthHR*_heightHR); // UINT_MAX may generate overflow

	for(unsigned j=0; j<_heightHR; ++j)
	{
		for(unsigned i=0; i<_widthHR; i++)
		{
			unsigned offset = ((j * _widthHR) + i);
			signed distance = nearestDistance[offset];
			nearestInsideDistance[offset] = distance>0 ? -distance : SHRT_MAX;				 
		}
	}
	std::vector<unsigned> trash;
	trash.resize(_widthHR*_heightHR, ~0);
	diffuseDistances(_widthHR, _heightHR, 
		nearestInsideDistance, trash);
	trash.resize(0);

	//unsigned dStep = std::min(widthFactor, heightFactor)/2;
	//unsigned oStep = std::max(widthFactor, heightFactor)/2 - dStep;
	//unsigned distanceMax =  4 * dStep + 3 * oStep;

	unsigned halfTileWidthFloor = (widthFactor)/2;
	unsigned halfTileWidthCeil = (widthFactor-1)/2;
	unsigned halfTileHeightFloor = (heightFactor)/2;
	unsigned halfTileHeightCeil = (heightFactor-1)/2;
	for(unsigned j=0; j<_heightLR; ++j)
	{
		for(unsigned i=0; i<_widthLR; ++i)
		{
			unsigned outOffset = j*_widthLR+i;

			unsigned inOffsetHRUL = (j*heightFactor+halfTileHeightFloor)*_stride+i*widthFactor+halfTileWidthFloor;
			unsigned inOffsetHRUR = (j*heightFactor+halfTileHeightFloor)*_stride+i*widthFactor+halfTileWidthCeil;
			unsigned inOffsetHRDL = (j*heightFactor+halfTileHeightCeil)*_stride+i*widthFactor+halfTileWidthFloor;
			unsigned inOffsetHRDR = (j*heightFactor+halfTileHeightCeil)*_stride+i*widthFactor+halfTileWidthCeil;

			// TODO: avg could be an error, check that
			signed distance = (nearestDistance[inOffsetHRUL] + nearestDistance[inOffsetHRUR]
				+ nearestDistance[inOffsetHRDL] + nearestDistance[inOffsetHRDR]);
			if(distance <= 3*2) // threshold is mid of 0-3
			{
				// inside
				distance = (nearestInsideDistance[inOffsetHRUL] + nearestInsideDistance[inOffsetHRUR]
					+ nearestInsideDistance[inOffsetHRDL] + nearestInsideDistance[inOffsetHRDR]);
				distance = distance/4 + 2;
			}else{
				// outside
				distance = -(distance/4) + 2;
			}

			// convert to byte range
			distance += 0x80; 
			distance = std::min(distance, 0xff);
			distance = std::max(distance, 0x00);

			unsigned pixelOffsetUL = nearestPixel[inOffsetHRUL];
			unsigned pixelOffsetUR = nearestPixel[inOffsetHRUR];
			unsigned pixelOffsetDL = nearestPixel[inOffsetHRDL];
			unsigned pixelOffsetDR = nearestPixel[inOffsetHRDR];
			_outColorsRGBA8[outOffset*4 + 0] = (_inColorRGBA8HR[pixelOffsetUL*4+0] + _inColorRGBA8HR[pixelOffsetUR*4+0]
				+ _inColorRGBA8HR[pixelOffsetDL*4+0] + _inColorRGBA8HR[pixelOffsetDR*4+0]) / 4;
			_outColorsRGBA8[outOffset*4 + 1] = (_inColorRGBA8HR[pixelOffsetUL*4+1] + _inColorRGBA8HR[pixelOffsetUR*4+1]
				+ _inColorRGBA8HR[pixelOffsetDL*4+1] + _inColorRGBA8HR[pixelOffsetDR*4+1]) / 4;
			_outColorsRGBA8[outOffset*4 + 2] = (_inColorRGBA8HR[pixelOffsetUL*4+2] + _inColorRGBA8HR[pixelOffsetUR*4+2]
				+ _inColorRGBA8HR[pixelOffsetDL*4+2] + _inColorRGBA8HR[pixelOffsetDR*4+2]) / 4;

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



