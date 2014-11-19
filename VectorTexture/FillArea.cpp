#include "stdafx.h"

#include "FillArea.h"
#include <assert.h>


/**
 * Filled horizontal segment of scanline y for xl<=x<=xr.
 * Parent segment was on line y-dy.  dy=1 or -1
 */
 struct Segment
{
	short y, xl, xr, dy;
};

struct FillArea
{
	unsigned int* outCluster;
	unsigned int outTag;

	const uint8_t* inImgC8;
	uint8_t inSelectC8;

    int x0, y0;			/* xmin and ymin */
    int x1, y1;			/* xmax and ymax (inclusive) */

	int width;
	int height;

	uint8_t pixelRead(int x, int y) const {
		assert(x>=0 && x<width);
		assert(y>=0 && y<height);
		int offset = y*width+x;
		return inImgC8[offset];
	}
	unsigned tagRead(int x, int y) const {
		assert(x>=0 && x<width);
		assert(y>=0 && y<height);
		int offset = y*width + x;
		return outCluster[offset];
	}
	void tagWrite(int x, int y, unsigned color) {
		assert(x>=0 && x<width);
		assert(y>=0 && y<height);
		int offset = y*width+x;
		outCluster[offset] = color;
	}
};


void tagArea(const Segment s, FillArea& fillParam)
{
	assert(s.xl>=0 && s.xr<=fillParam.width);
	int y = s.y + s.dy;
	if(y<fillParam.y0 || y>fillParam.y1)
		return;

	int x, l;
	for (x=s.xl; 
		x>=fillParam.x0 && (fillParam.pixelRead(x,y)==fillParam.inSelectC8 && fillParam.tagRead(x,y)!=fillParam.outTag);
		x--)
	{
		fillParam.tagWrite(x, y, fillParam.outTag);
	}

	if (x>=s.xl) goto skip;
	l = x+1;
	if (l<s.xl){
		Segment sgmt;
		sgmt.y = y;
		sgmt.xl = l;
		sgmt.xr = s.xl-1;
		sgmt.dy = -s.dy;
		/* leak on left? */
		tagArea(sgmt, fillParam);
	}
	x = s.xl+1;
	do {
	    for (;
			x<=fillParam.x1 && (fillParam.pixelRead(x,y)==fillParam.inSelectC8 && fillParam.tagRead(x, y)!=fillParam.outTag);
			x++)
		{
			fillParam.tagWrite(x, y, fillParam.outTag);
		}
		Segment sgmt;
		sgmt.y = y;
		sgmt.xl = l;
		sgmt.xr = x-1;
		sgmt.dy = s.dy;
		tagArea(sgmt, fillParam);

		if (x>s.xr+1)
		{
			Segment sgmt;
			sgmt.y = y;
			sgmt.xl = s.xr+1;
			sgmt.xr = x-1;
			sgmt.dy = -s.dy;
			tagArea(sgmt, fillParam);
			/* leak on right? */
		}
skip:	    
		for (x++; 
			x<=s.xr && (fillParam.pixelRead(x, y)!=fillParam.inSelectC8 || fillParam.tagRead(x, y)==fillParam.outTag); 
			x++)
			;
	    l = x;
	} while (x<=s.xr);
}

unsigned tagCluster(unsigned int* _cluster, const uint8_t* _inC8, const int _width, const int _height)
{
	unsigned int clusterName = 0;
	memset(_cluster, ~0, _width*_height*sizeof(*_cluster));
	for(int j=0; j<_height; ++j)
	{
		for(int i=0; i<_width; ++i)
		{
			int offset = (j*_width+i);
			if(_cluster[offset]==~0)
			{
				// new cluster!
				FillArea param;
				param.width = _width;
				param.height = _height;
				param.x0 = param.y0 = 0;
				param.x1 = _width-1;
				param.y1 = _height-1;
				param.inImgC8 = _inC8;
				param.inSelectC8 = _inC8[offset];
				param.outTag = clusterName;
				param.outCluster = _cluster;
				Segment sgmt;
				sgmt.xl = sgmt.xr = i;
				sgmt.y = j;
				sgmt.dy = 1;
				tagArea(sgmt, param);
				sgmt.y = j+1;
				sgmt.y = -1;
				tagArea(sgmt, param);
				clusterName++;
				//return clusterName;
			}
		}
	}
	return clusterName;
}

