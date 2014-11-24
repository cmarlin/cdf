#include "stdafx.h"

#include <assert.h>

#include "SplitColorToLayer.hpp"
#include "SEDT.h"


void GenerateSEDT(float* _out, uint16_t _radius,
				  uint8_t* _src, int _width, int _height)
{
	//const st	ImgSize = Vector2int16(w, h);
	SEDT				sedt;
	float				*img1, *img2;

	img1 = new float[_width * _height * 2];
	img2 = img1 + _width * _height;

	sedt.init(_width, _height, _src, false);
	sedt.compute(img1);

	sedt.init(_width, _height, _src, true);
	sedt.compute(img2);

	for (int i=0; i<_width*_height; ++i)
	{
		_out[i] = (img1[i]-img2[i])/static_cast<float>(_radius);
		_out[i] = (_out[i] * 0.5f) + 0.5f;
		_out[i] = std::min(1.0f, std::max(0.0f, _out[i]));
	}

	delete []img1;
}


static void splitBuffer(uint8_t* _outColorsRGBA8, const unsigned _widthLR, const unsigned _heightLR,
				 const std::set<unsigned>& _layer, 
				 const std::vector<unsigned>& _clusterIdHR, const uint8_t* _inColorRGBA8HR, const unsigned _widthHR, const unsigned _heightHR)
{
	unsigned _stride = _widthHR;
	unsigned widthFactor = _widthHR / _widthLR;
	unsigned heightFactor = _heightHR / _heightLR;

	uint8_t* src = NULL;
	float *distances = NULL;

	src = new uint8_t[_widthHR*_heightHR];
	unsigned pixelCount = 0;
	for(unsigned j=0; j<_heightHR; ++j)
	{
		for(unsigned i=0; i<_widthHR; i++)
		{
			unsigned offset = ((j * _stride) + i);
			unsigned cluster = _clusterIdHR[offset];
			if(_layer.find(cluster) != _layer.end())
			{
				src[offset] = 0xff;
				pixelCount++;
			}
			else
				src[offset] = 0x00;
		}
	}

	if(pixelCount == 0)
		goto end;

	distances = new float[_widthHR*_heightHR];
	GenerateSEDT(distances, 4,
		src, _widthHR, _heightHR);

	for(unsigned j=0; j<_heightLR; ++j)
	{
		for(unsigned i=0; i<_widthLR; ++i)
		{
			unsigned outOffset = j*_widthLR+i;
			unsigned inOffsetHR = (j*_stride*heightFactor)+i*widthFactor;
			_outColorsRGBA8[outOffset*4 + 0] = 0xff; //_inColorRGBA8HR[pixelOffset*4+0];
			_outColorsRGBA8[outOffset*4 + 1] = 0xff; //_inColorRGBA8HR[pixelOffset*4+1];
			_outColorsRGBA8[outOffset*4 + 2] = 0xff; //_inColorRGBA8HR[pixelOffset*4+2];

			unsigned distance = (unsigned)(distances[inOffsetHR] * 0xff);
			//distance = ((distance) * 0xff / (distanceMax)); //normalize
			//distance = distance>0xff ? 0xff : distance; // clamp
			distance = 0xff - distance; // invert
			_outColorsRGBA8[outOffset*4 + 3] = distance;
		}
	}
end:
	if(src!=NULL)
		delete[] src;
	if(distances!=NULL)
		delete[] distances;
}


bool splitBuffer2(std::vector<uint8_t*>& _outColorsRGBA8, 
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


