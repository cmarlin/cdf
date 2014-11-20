#include "stdafx.h"

#include <assert.h>

#include "SplitColorToLayer2.h"


enum TileCtrl
{
	TileCtrl_A=0,
	TileCtrl_B,
	TileCtrl_C,
	TileCtrl_D
};

enum ConstraintType
{
	Constraint_Below=0,
	Constraint_Upper, // 
	Constraint_Ratio, // p0/p1 => ratio
};

class Constraint
{
public:
	TileCtrl p0, p1;
	ConstraintType type;
	float value;
};

class Tile
{
public:
};

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


void getColorForSplitBuffer(uint8_t _color[4],
	const std::set<unsigned>& _layer,
	const uint8_t* _colorRGBA8Local, 
	const unsigned* _clusterIdLocal,
	const unsigned _width, const unsigned _height, const unsigned _stride)
{
	//_color[0] = _color[1] = _color[2] = 0; _color[3] = 0xff;
	//_color[0] = _color[1] = _color[2] = _color[3] = 0;
	_color[0] = _color[2] = 0xff; _color[1] = _color[3] = 0;

	// quick optim: if the cluster is the same as the "center", don't look forward

	//unsigned offset = ((((_height+1)/2)*_stride)+((_width+1)/2))*4; // center
	/*unsigned offset = 0; // top-left
	unsigned cluster = _clusterIdLocal[offset];
	if(_layer.find(cluster) != _layer.end() &&
		_clusterId == cluster)
	{
		for(int k=0; k<3; ++k)
		{
			_color[k] = _colorRGBA8Local[offset+k];
		}
		_color[3] = 0xff;
		return;
	}*/

	// offset + distanceSq
	std::vector<unsigned> nearestPixel;
	std::vector<signed> nearestDistance;
	nearestPixel.resize(_width*_height, ~0);
	nearestDistance.resize(_width*_height, SHRT_MAX); // UINT_MAX may generate overflow

	// init values: 0 for internal values
	unsigned pixelCount = 0;
	for(unsigned j=0; j<_height; ++j)
	{
		for(unsigned i=0; i<_width; i++)
		{
			unsigned offset = ((j * _stride) + i);
			unsigned cluster = _clusterIdLocal[offset];
			if(_layer.find(cluster) != _layer.end())
			{
				nearestPixel[j*_width+i] = j*_stride+i;
				nearestDistance[j*_width+i] = 0;
				pixelCount++;
			}
		}
	}

	if(pixelCount == 0)
		return;

	if(pixelCount == _width*_height)
	{
		unsigned offset = (_height/2)*_stride + (_width/2);
		_color[0] = _colorRGBA8Local[offset*4+0];
		_color[1] = _colorRGBA8Local[offset*4+1];
		_color[2] = _colorRGBA8Local[offset*4+2];
		_color[3] = 0xff;
		return;
	}

	unsigned ulOffset = ((_height-1)/2)*_width+((_width-1)/2);
	unsigned urOffset = ((_height-1)/2)*_width+((_width)/2);
	unsigned dlOffset = ((_height)/2)*_width+((_width-1)/2);
	unsigned drOffset = ((_height)/2)*_width+((_width)/2);

		//_color[0] = 0x7f;
		//_color[1] = 0x7f;
		//_color[2] = 0x7f;
		//_color[3] = 0xff;
		//return;

	diffuseDistances(_width, _height, 
		nearestDistance, nearestPixel);

	signed distance = (nearestDistance[ulOffset]
		+ nearestDistance[urOffset]
		+ nearestDistance[dlOffset]
		+ nearestDistance[drOffset]) / 4;

	//assert(distance>0);

	unsigned ulPixelOffset = nearestPixel[ulOffset];
	unsigned urPixelOffset = nearestPixel[urOffset];
	unsigned dlPixelOffset = nearestPixel[dlOffset];
	unsigned drPixelOffset = nearestPixel[drOffset];
	_color[0] = (_colorRGBA8Local[ulPixelOffset*4+0]
		+ _colorRGBA8Local[urPixelOffset*4+0]
		+ _colorRGBA8Local[dlPixelOffset*4+0]
		+ _colorRGBA8Local[drPixelOffset*4+0]) / 4;
	_color[1] = (_colorRGBA8Local[ulPixelOffset*4+1]
		+ _colorRGBA8Local[urPixelOffset*4+1]
		+ _colorRGBA8Local[dlPixelOffset*4+1]
		+ _colorRGBA8Local[drPixelOffset*4+1]) / 4;
	_color[2] = (_colorRGBA8Local[ulPixelOffset*4+2]
		+ _colorRGBA8Local[urPixelOffset*4+2]
		+ _colorRGBA8Local[dlPixelOffset*4+2]
		+ _colorRGBA8Local[drPixelOffset*4+2]) / 4;

	// if we are inside, look for the nearest border
	if(distance == 0)
	{
		// init values
		for(unsigned j=0; j<_height; ++j)
		{
			for(unsigned i=0; i<_width; i++)
			{
				unsigned offset = ((j * _stride) + i);
				unsigned cluster = _clusterIdLocal[offset];
				if(_layer.find(cluster) == _layer.end())
				{
					nearestPixel[j*_width+i] = j*_stride+i;
					nearestDistance[j*_width+i] = 0;
				}
				else
				{
					nearestPixel[j*_width+i] = ~0;
					nearestDistance[j*_width+i] = SHRT_MAX;
				}
			}
		}

		diffuseDistances(_width, _height, 
			nearestDistance, nearestPixel);

		distance = -(nearestDistance[ulOffset]
			+ nearestDistance[urOffset]
			+ nearestDistance[dlOffset]
			+ nearestDistance[drOffset]) / 4;
	}

	//unsigned colorAcc[4] = {0};
	//unsigned colorCount = 0;
	//if(nearestDistance[ulOffset] == INT_MAX)
	//	return;


	//signed res = 
	//	(_width & 0x1) ? 
	//		((_height & 0x1) ? 0 : 3) 
	//		: ((_height & 0x1) ? 3 : 4);

	//signed distance = std::min(nearestDistance[ulOffset]+res,
	//	+ std::min(nearestDistance[urOffset]+res,
	//	+ std::min(nearestDistance[dlOffset]+res,
	//	+ nearestDistance[drOffset]+res)));

	signed dStep = std::min(_width-1, _height-1);
	signed oStep = std::max(_width-1, _height-1) - dStep;
	signed distanceMax =  (4 * dStep + 3 * oStep)/2;
	//signed halfPix = std::max((0+3+3+0)/2, (4+3+3+0)/2);
	//distanceMax -= halfPix;
	//signed distanceMax =  (8 + 7 + 7 + 4)/4; 
	//_color[3] = 0xff - ((distance) * 0xff / (distanceMax));
	signed distanceMin = -distanceMax;
	assert(distance<=distanceMax && distance>=distanceMin);
	_color[3] = 0xff - ((distance-distanceMin) * 0xff / (distanceMax-distanceMin));
	//signed tmp = (distance-distanceMin) * 0xff / (distanceMax-distanceMin);
	//_color[3] = tmp;
}


bool splitBuffer(std::vector<uint8_t*>& _outColorsRGBA8, 
				 const std::vector<std::set<unsigned>>& _layers, 
				 const unsigned _widthLR, const unsigned _heightLR,
				 const std::vector<unsigned>& _clusterIdHR, const uint8_t* _inColorRGBA8HR, const unsigned _widthHR, const unsigned _heightHR)
{
	if(_outColorsRGBA8.size() != _layers.size())
		return false;
	unsigned widthFactor = _widthHR / _widthLR;
	unsigned heightFactor = _heightHR / _heightLR;

	for(unsigned layerId = 0; layerId<_layers.size(); layerId++)
	{
		//std::set<unsigned>& layer = *it;
		const std::set<unsigned>& layer = _layers[layerId];
		uint8_t* outbuffer = _outColorsRGBA8[layerId];
		for(unsigned j=0; j<_heightLR; ++j)
		{
			for(unsigned i=0; i<_widthLR; ++i)
			{
				unsigned outOffset = j*_widthLR+i;

				/*if(layer.find(cluster) != layer.end())
				{
					outbuffer[offset*4 + 0] = _inColorRGBA8[offset*4 + 0];
					outbuffer[offset*4 + 1] = _inColorRGBA8[offset*4 + 1];
					outbuffer[offset*4 + 2] = _inColorRGBA8[offset*4 + 2];
					outbuffer[offset*4 + 3] = 0xff;
				}*/
				uint8_t color[4];
				unsigned inOffsetHR = (j*_widthHR*heightFactor)+i*widthFactor;
				const uint8_t* localPixels = &_inColorRGBA8HR[inOffsetHR*4];
				const unsigned* localClusterIds = &_clusterIdHR[inOffsetHR];

				getColorForSplitBuffer(color, layer, 
					localPixels, localClusterIds, widthFactor, heightFactor, _widthHR);

				outbuffer[outOffset*4 + 0] = color[0];
				outbuffer[outOffset*4 + 1] = color[1];
				outbuffer[outOffset*4 + 2] = color[2];
				outbuffer[outOffset*4 + 3] = color[3];
				//outbuffer[offset*4 + 3] = 0xff;
			}
		}
	}
	return true;
}



