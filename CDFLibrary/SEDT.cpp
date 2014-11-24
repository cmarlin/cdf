#include "stdafx.h"

#include <stdlib.h>
#include <math.h>

#include "SEDT.h"


SEDT::SEDT()
{
	distMap = NULL;
}

void SEDT::release()
{
	delete[] distMap; 
	distMap = NULL;
}

void SEDT::init( int w, int h, uint8_t *binImg, bool invert /*= false*/ )
{
	const std::pair<int16_t,int16_t> z = std::pair<int16_t,int16_t>(0,0);
	const std::pair<int16_t,int16_t> max = std::pair<int16_t,int16_t>(w+1,h+1);

	this->w = w;
	this->h = h;

	release();

	distMap = new std::pair<int16_t,int16_t>[w*h];

	if (invert)
	{
		for (int i=0; i<w*h; ++i)
		{
			distMap[i] = (binImg[i] < 127) ? max : z;
		}
	}
	else
	{
		for (int i=0; i<w*h; ++i)
		{
			distMap[i] = (binImg[i] < 127) ? z : max;
		}
	}
}

void SEDT::updatePix( int idx, int idxOffset, int x, int y, int xOffset, int yOffset )
{
	if (x+xOffset < 0 || x+xOffset >= w || y+yOffset < 0 || y+yOffset >= h)
	{
		return;
	}

	std::pair<int16_t,int16_t> n = distMap[idx + idxOffset];
	std::pair<int16_t,int16_t> &p = distMap[idx];

	n.first += xOffset;
	n.second += yOffset;

	if (p.first*p.first + p.second*p.second > n.first*n.first + n.second*n.second)
		p = n;
}

void SEDT::compute( float *outDist )
{
	int idx;

	// Pass 0
	for (int y=0;y<h;y++)
	{
		idx = y*w;

		for (int x=0;x<w;x++)
		{
			updatePix( idx, -1, x, y, -1,  0 );
			updatePix( idx, -w, x, y,  0, -1 );
			updatePix( idx, -w-1, x, y, -1, -1 );
			updatePix( idx, -w+1, x, y,  1, -1 );

			++idx;
		}

		--idx;

		for (int x=w-1; x>=0; x--)
		{
			updatePix( idx, 1, x, y, 1, 0 );

			--idx;

		}
	}

	// Pass 1
	idx = w*h-1;
	for (int y=h-1; y>=0; y--)
	{
		idx = (y+1)*w - 1;

		for (int x=w-1; x>=0; x--)
		{
			updatePix( idx, 1, x, y,  1,  0 );
			updatePix( idx, w, x, y,  0,  1 );
			updatePix( idx, w-1, x, y, -1,  1 );
			updatePix( idx, w+1, x, y,  1,  1 );

			--idx;
		}

		++idx;

		for (int x=0;x<w;x++)
		{
			updatePix( idx, -1, x, y, -1, 0 );

			++idx;
		}

		idx -= w;
	}

	for (int i=0; i<w*h; ++i)
	{
		outDist[i] = sqrtf((float)distMap[i].first*distMap[i].first + distMap[i].second*distMap[i].second);
	}
}

SEDT::~SEDT()
{
	release();
}
