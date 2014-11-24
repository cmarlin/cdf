#include "stdafx.h"

#include "DumpBuffer.hpp"
#include "tga.h"

void dumpBufferUB(const char* filename, const uint8_t* _data, const int _width, const int _height)
{
	uint8_t* outData = new uint8_t[_width * _height * sizeof(uint32_t)];
	for(int j=0; j<_height; ++j)
	{
		for(int i=0; i<_width; ++i)
		{
			int offset = (j*_width)+i;
			outData[offset*sizeof(uint32_t) + 0] = _data[offset]; //B
			outData[offset*sizeof(uint32_t) + 1] = _data[offset]; //G
			outData[offset*sizeof(uint32_t) + 2] = _data[offset]; //R
			outData[offset*sizeof(uint32_t) + 3] = 0xff; // A
			//outData[offset*sizeof(uint32_t) + 3] = _data[offset]; // A
		}
	}
	FILE* fout = fopen(filename, "wb");
	if(fout!=NULL)
	{
		TGA_HEADER outHeader = {0};
		outHeader.width = _width;
		outHeader.height = _height;
		outHeader.imagetype = IMAGE_TYPE_BGR;
		outHeader.bits = 32;
		outHeader.descriptor = DESCRIPTOR_H_FLIP;

		writeTGA(&outHeader, fout);
		int bytewrote = fwrite(outData, 1, _width*_height*4, fout);
		fclose(fout);
	}

	delete[] outData;
}

void dumpBuffer4UB(const char* filename, const uint8_t* _data, const int _width, const int _height)
{
	uint8_t* outData = new uint8_t[_width * _height * sizeof(uint32_t)];
	for(int j=0; j<_height; ++j)
	{
		for(int i=0; i<_width; ++i)
		{
			int offset = (j*_width)+i;
			outData[offset*sizeof(uint32_t) + 0] = _data[offset*4 + 2]; //B
			outData[offset*sizeof(uint32_t) + 1] = _data[offset*4 + 1]; //G
			outData[offset*sizeof(uint32_t) + 2] = _data[offset*4 + 0]; //R
			//outData[offset*sizeof(uint32_t) + 3] = 0xff; // A
			outData[offset*sizeof(uint32_t) + 3] = _data[offset*4 + 3]; // A
		}
	}
	FILE* fout = fopen(filename, "wb");
	if(fout!=NULL)
	{
		TGA_HEADER outHeader = {0};
		outHeader.width = _width;
		outHeader.height = _height;
		outHeader.imagetype = IMAGE_TYPE_BGR;
		outHeader.bits = 32;
		outHeader.descriptor = DESCRIPTOR_H_FLIP;

		writeTGA(&outHeader, fout);
		int bytewrote = fwrite(outData, 1, _width*_height*4, fout);
		fclose(fout);
	}

	delete[] outData;
}

void dumpBufferUI(const char* filename, const unsigned int * _data, const int _width, const int _height)
{
	uint8_t* outData = new uint8_t[_width * _height * sizeof(uint32_t)];
	for(int j=0; j<_height; ++j)
	{
		for(int i=0; i<_width; ++i)
		{
			int offset = (j*_width)+i;
			outData[offset*sizeof(uint32_t) + 0] = (uint8_t)_data[offset]; //B
			outData[offset*sizeof(uint32_t) + 1] = (uint8_t)_data[offset]; //G
			outData[offset*sizeof(uint32_t) + 2] = (uint8_t)_data[offset]; //R
			outData[offset*sizeof(uint32_t) + 3] = 0xff; // A
		}
	}
	FILE* fout = fopen(filename, "wb");
	if(fout!=NULL)
	{
		TGA_HEADER outHeader = {0};
		outHeader.width = _width;
		outHeader.height = _height;
		outHeader.imagetype = IMAGE_TYPE_BGR;
		outHeader.bits = 32;
		outHeader.descriptor = DESCRIPTOR_H_FLIP;

		writeTGA(&outHeader, fout);
		int bytewrote = fwrite(outData, 1, _width*_height*4, fout);
		fclose(fout);
	}

	delete[] outData;
}

