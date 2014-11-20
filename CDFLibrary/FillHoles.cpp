#include "stdafx.h"

//#include <string>
#include <vector>
#include <assert.h>

// 2 pass: horizontal, then vertical
void fillHoles(uint8_t* _colorRGBA8, const unsigned _width, const unsigned _height)
{
	std::vector<std::pair<unsigned, unsigned>> distH;
	distH.resize(_width*_height);
	std::pair<unsigned, unsigned> clearElt(~0, ~0);
	for(std::vector<std::pair<unsigned, unsigned>>::iterator it = distH.begin(); it!=distH.end(); it++) *it=clearElt;

	for(unsigned j=0; j<_height; ++j)
	{
		for(unsigned i=0; i<_width; ++i)
		{
			// look for start of hole
			unsigned offsetStart = (j*_width+i)*4;
			if(_colorRGBA8[offsetStart+3] == 0)
			{
				// look for end
				unsigned ii=i;
				for(; ii<_width; ++ii)
				{
					unsigned offsetEnd = (j*_width+ii)*4;
					if(_colorRGBA8[offsetEnd+3]!=0)
						break;
				}

				if(i==0 && ii!=_width)
				{
					unsigned offsetTo = (j*_width+ii);
					for(unsigned k=i; k<ii; ++k)
					{
						unsigned offset = (j*_width+k);
						std::pair<unsigned, unsigned> value(offsetTo, ii-k);
						assert(value.second>0);
						assert(_colorRGBA8[offset*4+3] == 0);
						distH[offset] = value;
					}
				}
				if(i>0 && ii==_width)
				{
					unsigned offsetFrom = j*_width+i-1; // on first pixel hole
					for(unsigned k=i; k<ii; ++k)
					{
						unsigned offset = (j*_width+k);
						std::pair<unsigned, unsigned> value(offsetFrom, k-(i-1));
						assert(value.second>0);
						assert(_colorRGBA8[offset*4+3] == 0);
						distH[offset] = value;
					}
				}
				if(i>0 && ii<_width)
				{
					// pick colors
					unsigned offsetFrom = j*_width+i-1; // on first pixel hole
					unsigned offsetTo = j*_width+ii; // on first pixel filled
						 
					// fill
					for(unsigned k=i; k<(i+ii)/2; ++k)
					{
						unsigned offset = (j*_width+k);
						std::pair<unsigned, unsigned> value(offsetFrom, k-(i-1));
						assert(value.second>0);
						assert(_colorRGBA8[offset*4+3] == 0);
						distH[offset] = value;
					}
					for(unsigned k=(i+ii)/2; k<ii; ++k)
					{
						unsigned offset = (j*_width+k);
						std::pair<unsigned, unsigned> value(offsetTo, ii-k);
						assert(value.second>0);
						assert(_colorRGBA8[offset*4+3] == 0);
						distH[offset] = value;
					}
				}

				// continue
				i = ii;
			}
		}
	}

	std::vector<std::pair<unsigned, unsigned>> distV;
	distV.resize(_width*_height);
	for(std::vector<std::pair<unsigned, unsigned>>::iterator it = distV.begin(); it!=distV.end(); it++) *it=clearElt;

	for(unsigned i=0; i<_width; ++i)
	{
		for(unsigned j=0; j<_height; ++j)
		{
			// look for start of hole
			unsigned offsetStart = (j*_width+i)*4;
			if(_colorRGBA8[offsetStart+3] == 0)
			{
				// look for end
				unsigned jj=j;
				for(; jj<_height; ++jj)
				{
					unsigned offsetEnd = (jj*_width+i)*4;
					if(_colorRGBA8[offsetEnd+3]!=0)
						break;
				}

				if(j==0 && jj!=_height)
				{
					unsigned offsetTo = (jj*_width+i);
					for(unsigned k=j; k<jj; ++k)
					{
						unsigned offset = (k*_width+i);
						std::pair<unsigned, unsigned> value(offsetTo, jj-k);
						assert(value.second>0);
						assert(_colorRGBA8[offset*4+3] == 0);
						distV[offset] = value;
					}
				}
				if(j>0 && jj==_height)
				{
					unsigned offsetFrom = (j-1)*_width+i; // on first pixel hole
					for(unsigned k=j; k<jj; ++k)
					{
						unsigned offset = (k*_width+i);
						std::pair<unsigned, unsigned> value(offsetFrom, k-(j-1));
						assert(value.second>0);
						assert(_colorRGBA8[offset*4+3] == 0);
						distV[offset] = value;
					}
				}
				if(j>0 && jj<_height)
				{
					// pick colors
					unsigned offsetFrom = (j-1)*_width+i; // on first pixel hole
					unsigned offsetTo = jj*_width+i; // on first pixel filled
						 
					// fill
					for(unsigned k=j; k<(j+jj)/2; ++k)
					{
						unsigned offset = (k*_width+i);
						std::pair<unsigned, unsigned> value(offsetFrom, k-(j-1));
						assert(value.second>0);
						assert(_colorRGBA8[offset*4+3] == 0);
						distV[offset] = value;
					}
					for(unsigned k=(j+jj)/2; k<jj; ++k)
					{
						unsigned offset = (k*_width+i);
						std::pair<unsigned, unsigned> value(offsetTo, jj-k);
						assert(value.second>0);
						assert(_colorRGBA8[offset*4+3] == 0);
						distV[offset] = value;
					}
				}

				// continue
				j = jj;
			}
		}
	}

	// fill final color
	for(unsigned j=0; j<_height; ++j)
	{
		for(unsigned i=0; i<_width; ++i)
		{
			unsigned offset = j*_width + i;
			std::pair<unsigned, unsigned> h = distH[offset];
			std::pair<unsigned, unsigned> v = distV[offset];
			//std::pair<unsigned, unsigned> nearest;
			if(h.second == ~0
				&& v.second == ~0)
				continue;
			unsigned color[3]= {0};
			//unsigned colorCount = 0;
			if(h.second == v.second)
			{
				color[0] = (_colorRGBA8[h.first*4+0] + _colorRGBA8[v.first*4+0])>>1;
				color[1] = (_colorRGBA8[h.first*4+1] + _colorRGBA8[h.first*4+1])>>1;
				color[2] = (_colorRGBA8[h.first*4+2] + _colorRGBA8[h.first*4+2])>>1;
			}
			else if(h.second == ~0 || v.second<h.second)
			{
				color[0] = _colorRGBA8[v.first*4+0];
				color[1] = _colorRGBA8[v.first*4+1];
				color[2] = _colorRGBA8[v.first*4+2];
			}
			else if(v.second == ~0 || h.second<v.second)
			{
				color[0] = _colorRGBA8[h.first*4+0];
				color[1] = _colorRGBA8[h.first*4+1];
				color[2] = _colorRGBA8[h.first*4+2];
			}

			assert(_colorRGBA8[offset*4+3] == 0);
			_colorRGBA8[offset*4+0] = color[0];
			_colorRGBA8[offset*4+1] = color[1];
			_colorRGBA8[offset*4+2] = color[2];
			//_colorRGBA8[offset*4+3] = 0; //_colorRGBA8[nearest.first*4+3];
		}
	}

	// check no residual color
	/*for(unsigned j=0; j<_height; ++j)
	{
		for(unsigned i=0; i<_width; ++i)
		{
			unsigned offset = j*_width + i;
			if(_colorRGBA8[offset*4+0]==0xff
				&& _colorRGBA8[offset*4+1]==0x00
				&& _colorRGBA8[offset*4+2]==0xff
				&& _colorRGBA8[offset*4+3]==0)
			{
				assert(false);
			}
		}
	}*/
}

