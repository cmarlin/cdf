#include "stdafx.h"

#include "CDFLibrary.h"
#include <assert.h>

#include <iterator>

#include "VTCluster.hpp"
#include "ExtractCluster.hpp"
#include "FillArea.hpp"
#include "SplitColorToLayer.hpp"
#include "ExtractLayer.hpp"



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


void EncodeCDF(EncodeResult& _result, const EncodeConfiguration& _config,
				 const OutputData& _outputData, const InputData& _inputData, 
				 const CDF_Encode_CB _callback, const uintptr_t _user)
{
	////
	/// check input parameters
	//
	if(_outputData.m_width>_inputData.m_width || _outputData.m_height>_inputData.m_height)
	{
		_result.m_status = EncodeResult::EncodeStatus_Failed;
		_result.m_msg = "output resolution must be smaller than input resolution; good value: x4 (in 1024x1024 => out 256x256)";
		return;
	}
	unsigned widthFactor = _outputData.m_width/_inputData.m_width;
	unsigned heightFactor = _outputData.m_height/_inputData.m_height;
	if(_inputData.m_width * widthFactor != _outputData.m_width
		|| _inputData.m_height * widthFactor != _outputData.m_height)
	{
		_result.m_status = EncodeResult::EncodeStatus_Failed;
		_result.m_msg = "output resolution must be integer factor of input resolution; good value: x4 (in 1024x1024 => out 256x256)";
		return;
	}

	if(_config.m_layer>CDF_MAX_LAYERS)
	{
		_result.m_status = EncodeResult::EncodeStatus_Failed;
		_result.m_msg = "only \"CDF_MAX_LAYERS\" layers max supported";
		return;
	}
	for(unsigned i=0; i<_config.m_layer; ++i)
	{
		if(_outputData.m_bufferRGBA8[i] == NULL)
		{
			_result.m_status = EncodeResult::EncodeStatus_Failed;
			_result.m_msg = "invalid outputbuffer";
			return;
		}
	}

	unsigned widthHR = _inputData.m_width;
	unsigned heightHR = _inputData.m_height;
	uint8_t* inDataHR = _inputData.m_bufferRGBA8;

	//uint8_t* stepLumHR = new uint8_t[widthHR * heightHR];
	std::vector<uint8_t> stepLumHR(widthHR * heightHR);
	printf("compute lum (c8)...\n");
	computePseudoLuminosity(stepLumHR.data(), inDataHR, widthHR, heightHR);
	printf("compute lum (c8) done\n");
	if(_callback!=NULL)
	{
		printf("save lum (c8)...\n");
		EncodeEvent event;
		event.m_event = EncodeEvent::EncodeEvent_C8;
		_callback(event, _user);
		//dumpBufferUB("out-c8.tga", stepLumHR.data(), widthHR, heightHR);
		printf("save lum (c8) done\n");
	}
	
	printf("compute ids...\n");
	std::vector<ClusterId> clusterIdHR(widthHR * heightHR);
	unsigned clusterCount = tagCluster(clusterIdHR.data(), stepLumHR.data(), widthHR, heightHR);
	printf("compute ids done\n");
	printf("found %d distinct ids\n", clusterCount);
	if(_callback!=NULL)
	{
		printf("save ids...\n");
		EncodeEvent event;
		event.m_event = EncodeEvent::EncodeEvent_ClusterId;
		_callback(event, _user);
		//dumpBufferUI("out-ids.tga", clusterIdHR.data(), widthHR, heightHR);
		printf("save ids done\n");
	}
	
	//const unsigned shrinkFactor = 4;
	//unsigned widthLR = widthHR / shrinkFactor;
	//unsigned heightLR = heightHR / shrinkFactor;

	unsigned widthLR = _outputData.m_width;
	unsigned heightLR = _outputData.m_height;

	//std::vector<unsigned> clusterIdLR(widthLR * heightLR);
	//if(!shrink2DBufferUI(clusterIdLR.data(), widthLR, heightLR, clusterIdHR.data(), widthHR, heightHR))
	//{
	//	fprintf(stderr, "failed to shrink input image, not a multiple of compression factor? %d => %d", widthHR, shrinkFactor);
	//	return 0;
	//}

	/*{
		printf("save out-ref ...\n");
		std::vector<uint8_t> inDataLR(widthLR * heightLR * 4);
		if(!shrink2DBuffer4UB(inDataLR.data(), widthLR, heightLR, inDataHR, widthHR, heightHR))
		{
			fprintf(stderr, "failed to shrink input image, not a multiple of compression factor? %d => %d", widthHR, shrinkFactor);
			return 0;
		}
		dumpBuffer4UB("out-ref.tga", inDataLR.data(), widthLR, heightLR);
		printf("save out-ref done\n");
	}*/

	printf("extract clusters...\n");
	std::map<ClusterId, VTCluster> clusterGraph;
	extractCluster(clusterGraph, widthLR, heightLR, clusterIdHR, widthHR, heightHR);
	printf("extract clusters done\n");

	//unsigned layerCount = 3;
	unsigned layerCount = _config.m_layer;
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
		//buffers[i] = new uint8_t[widthLR * heightLR * 4];
		//memset(buffers[i], 0, widthLR * heightLR * 4);
		buffers[i] = _outputData.m_bufferRGBA8[i];
	}

	splitBuffer(buffers, layers, 
		widthLR, heightLR,
		clusterIdHR, inDataHR, widthHR, heightHR);
	printf("split colors to buffers done\n");
	
	_result.m_status = EncodeResult::EncodeStatus_Success;
	_result.m_msg = NULL;
}