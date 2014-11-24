#include "stdafx.h"

#include <stdlib.h>

#include "Test.hpp"
#include "DumpBuffer.hpp"


void testLinear()
{
	uint8_t* buffers[1];
	const int widthLR = 4;
	const int heightLR = 4;
	buffers[0] = new uint8_t[widthLR * heightLR * 4];

	for(int j=0; j<heightLR; ++j)
		for(int i=0; i<widthLR; ++i)
		{
			int offset = j*widthLR + i;
			buffers[0][offset*4+0] = i+j & 0x1 ? 0xff : 0x00;
			buffers[0][offset*4+1] = i+j & 0x1 ? 0xff : 0x00;
			buffers[0][offset*4+2] = i+j & 0x1 ? 0xff : 0x00;

			buffers[0][offset*4+3] = i+j>=widthLR ?
				(i+j>widthLR ? 0xff : 0xbf)
				: (i+j<widthLR-1)?0x00 : 0x3f;
		}

	int i=0;
	char outfileName[_MAX_PATH];
	sprintf(outfileName, "out-%d.tga", i);
	dumpBuffer4UB(outfileName, buffers[i], widthLR, heightLR);
	delete[] buffers[0];
	buffers[0] = NULL;

}
