// main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <assert.h>

#include <iterator>
//#include "freeimage.h"
#include "tga.h"
#include "VTCluster.hpp"
#include "ExtractCluster.hpp"
#include "Shrink2DBuffer.hpp"
#include "FillArea.hpp"
#include "DumpBuffer.hpp"
#include "SplitColorToLayer.hpp"
#include "ExtractLayer.hpp"


/**
	1°) Cluster creation (not so easy ;-)) - coherent group of pixels (high or low res ?)
	2°) Assign Cluster to Layer & expand - minimise expand conflicts
	- generate distance field the same time
	- color graph probleme (edge conflict)
	3°) Isocontour...
	- sam
*/

void computePseudoLuminosity(uint8_t* _outC8, const uint8_t* _inRGBA8888, const int _width, const int _height)
{
	for(int j=0; j<_height; ++j)
	{
		for(int i=0; i<_width; ++i)
		{
			int offset = (j*_width+i);
			int inOffset = offset*4;
			// pseudo luminosity computing (R+2G+B)/4
			int lum = (_inRGBA8888[inOffset+0] + (_inRGBA8888[inOffset+1]<<1) + _inRGBA8888[inOffset+2]) >> 2;
			int outOffset = offset;
			//int cluster = ((lum>>4)<<4)|(lum>>4); // keep 4 bits => 16 luminosity levels
			int cluster = (lum>>6); // keep 2 bits => 4 luminosity levels for cluster
			cluster = (cluster<<6) | (cluster<<4) | (cluster<<2) | cluster;
			_outC8[outOffset] = cluster;
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	//testLinear();
	//return 0;

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

	printf("save lum (c8)...\n");
	//uint8_t* stepLumHR = new uint8_t[widthHR * heightHR];
	std::vector<uint8_t> stepLumHR(widthHR * heightHR);
	computePseudoLuminosity(stepLumHR.data(), inDataHR, widthHR, heightHR);
	dumpBufferUB("out-c8.tga", stepLumHR.data(), widthHR, heightHR);
	printf("save lum (c8) done\n");
	
	printf("compute ids...\n");
	std::vector<ClusterId> clusterIdHR(widthHR * heightHR);
	unsigned clusterCount = tagCluster(clusterIdHR.data(), stepLumHR.data(), widthHR, heightHR);
	printf("compute ids done\n");
	printf("found %d distinct ids\n", clusterCount);
	printf("save ids...\n");
	dumpBufferUI("out-ids.tga", clusterIdHR.data(), widthHR, heightHR);
	printf("save ids done\n");
	
	const unsigned shrinkFactor = 4;
	unsigned widthLR = widthHR / shrinkFactor;
	unsigned heightLR = heightHR / shrinkFactor;
	//std::vector<unsigned> clusterIdLR(widthLR * heightLR);
	//if(!shrink2DBufferUI(clusterIdLR.data(), widthLR, heightLR, clusterIdHR.data(), widthHR, heightHR))
	//{
	//	fprintf(stderr, "failed to shrink input image, not a multiple of compression factor? %d => %d", widthHR, shrinkFactor);
	//	return 0;
	//}

	{
		printf("save out-ref ...\n");
		std::vector<uint8_t> inDataLR(widthLR * heightLR * 4);
		if(!shrink2DBuffer4UB(inDataLR.data(), widthLR, heightLR, inDataHR, widthHR, heightHR))
		{
			fprintf(stderr, "failed to shrink input image, not a multiple of compression factor? %d => %d", widthHR, shrinkFactor);
			return 0;
		}
		dumpBuffer4UB("out-ref.tga", inDataLR.data(), widthLR, heightLR);
		printf("save out-ref done\n");
	}

	printf("extract clusters...\n");
	std::map<ClusterId, VTCluster> clusterGraph;
	extractCluster(clusterGraph, widthLR, heightLR, clusterIdHR, widthHR, heightHR);
	printf("extract clusters done\n");

	unsigned layerCount = 3;
	std::vector<std::set<ClusterId>> layers;
	printf("extract layers...\n");
	unsigned remainErrors = extractLayers(layers, layerCount, clusterGraph);
	printf("remains %d errors\n", remainErrors);
	printf("extract layers done\n");

	printf("split colors to buffers...\n");
	std::vector<uint8_t*> buffers;
	buffers.resize(layerCount);
	for(unsigned i=0; i<layerCount; ++i)
	{
		buffers[i] = new uint8_t[widthLR * heightLR * 4];
		memset(buffers[i], 0, widthLR * heightLR * 4);
	}

	splitBuffer(buffers, layers, 
		widthLR, heightLR,
		clusterIdHR, inDataHR, widthHR, heightHR);
	printf("split colors to buffers done\n");
		
	printf("save %d outputs ...\n", layerCount);
	for(unsigned i=0; i<layerCount; ++i)
	{
		char outfileName[_MAX_PATH];
		sprintf(outfileName, "out-%d.tga", i);
		dumpBuffer4UB(outfileName, buffers[i], widthLR, heightLR);

		// fill holes
		//fillHoles(buffers[i], widthLR, heightLR);
		//sprintf(outfileName, "out-%d-filled.tga", i);
		//dumpBuffer4UB(outfileName, buffers[i], widthLR, heightLR);
	}
	printf("save outputs done\n");

	for(unsigned i=0; i<layerCount; ++i)
	{
		delete[] buffers[i];
	}
	buffers.clear();

	//if(stepLumHR!=NULL){delete[] stepLumHR; stepLumHR=NULL;}
	if(inDataHR!=NULL){delete[] inDataHR; inDataHR=NULL;}
		
	return 0;
}

	