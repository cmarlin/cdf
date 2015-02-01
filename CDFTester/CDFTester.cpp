// CDFTester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdlib.h>

//#include "freeimage.h"
#include "CDFLibrary.h"
#include "tga.h"
#include "DumpBuffer.hpp"

unsigned SHRINK_FACTOR = 4;
unsigned LAYER_COUNT = 3;

int _tmain(int argc, _TCHAR* argv[])
{
	//FreeImage_Initialise();
	//FreeImage_DeInitialise();

	//uint8_t* stepC8 = NULL;
	uint8_t* inDataHR = NULL;
	unsigned widthHR, heightHR;

	FILE* fin;
	char* filename = argv[1];
	if((fin=fopen(filename, "rb"))!=NULL)
	{
		printf("read input file...\n");

		TGA_HEADER inHeader;
		TGA_ReadHeader(&inHeader, fin);
		inDataHR = new uint8_t[inHeader.width * inHeader.height * sizeof(uint32_t)];
		TGA_ReadDataRGBA8(inDataHR, &inHeader, fin);
		widthHR = inHeader.width;
		heightHR = inHeader.height;
		fclose(fin);
		printf("read input file done\n");
	}
	else
	{
		return 0;
	}

	////
	/// prepare output
	//
	unsigned widthLR = widthHR / SHRINK_FACTOR;
	unsigned heightLR = heightHR / SHRINK_FACTOR;

	OutputData outputData;
	outputData.m_width = widthLR;
	outputData.m_height = heightLR;
	for(unsigned i=0; i<LAYER_COUNT; ++i)
	{
		outputData.m_bufferRGBA8[i] = new uint8_t[widthLR * heightLR * 4];
		memset(outputData.m_bufferRGBA8[i], 0, widthLR * heightLR * 4);
	}

	////
	/// encode
	//
	InputData inputData;
	inputData.m_width = widthHR;
	inputData.m_height = heightHR;
	inputData.m_bufferRGBA8 = inDataHR;
	
	EncodeResult result;

	EncodeConfiguration config;
	config.m_layer = LAYER_COUNT;

	EncodeCDF(result, config, outputData, inputData);

	if(result.m_status!=EncodeResult::EncodeStatus_Success)
	{
		printf("Encoding fails:%s", result.m_msg!=NULL ? result.m_msg : "no message");
		return -1;
	}

	////
	/// save result
	//
	printf("save %d outputs ...\n", LAYER_COUNT);
	for(unsigned i=0; i<LAYER_COUNT; ++i)
	{
		char outfileName[_MAX_PATH];
		sprintf(outfileName, "out-%d.tga", i);
		dumpBuffer4UB(outfileName, outputData.m_bufferRGBA8[i], widthLR, heightLR);
	}
	printf("save outputs done\n");

	for(unsigned i=0; i<LAYER_COUNT; ++i)
	{
		delete[] outputData.m_bufferRGBA8[i];
	}

	if(inDataHR!=NULL){delete[] inDataHR; inDataHR=NULL;}
		
	return 0;
}

