#include "stdafx.h"

#include "../CDFLibrary.h"

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

			// keep 6 bits => 64 levels
			int cluster = lum & 0xfc;

			// keep 4 bits => 16 luminosity levels
			//int cluster = ((lum>>4)<<4)|(lum>>4);

			// keep 2 bits => 4 luminosity levels for cluster
			//int cluster = (lum>>6); 
			//cluster = (cluster<<6) | (cluster<<4) | (cluster<<2) | cluster;

			_outC8[outOffset] = cluster;
		}
	}
}


void clearClusterTranslucent(ClusterId* _clusterIds, const uint8_t* _inRGBA8888, const int _width, const int _height)
{
	for(int j=0; j<_height; ++j)
	{
		for(int i=0; i<_width; ++i)
		{
			int clusterOffset = (j*_width+i);
			int colorOffset = clusterOffset*4;
			// pseudo luminosity computing (R+2G+B)/4
			int alpha = _inRGBA8888[colorOffset+3];
			if(alpha == 0)
			{
				_clusterIds[clusterOffset] = ClusterIdInvalid;
			}
		}
	}
}


void EncodeCDF(EncodeResult& _result, const EncodeConfiguration& _config,
				 const OutputData& _outputData, const InputData& _inputData, 
				 const CDF_Encode_CB _callback, const uintptr_t _cbUserData)
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
	unsigned widthFactor = _inputData.m_width/_outputData.m_width;
	unsigned heightFactor = _inputData.m_height/_outputData.m_height;
	if(_outputData.m_width * widthFactor != _inputData.m_width
		|| _outputData.m_height * heightFactor != _inputData.m_height)
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
			_result.m_msg = "invalid (null) outputbuffer";
			return;
		}
	}

	unsigned widthHR = _inputData.m_width;
	unsigned heightHR = _inputData.m_height;
	uint8_t* inDataHR = _inputData.m_bufferRGBA8;

	std::vector<uint8_t> stepLumHR(widthHR * heightHR);
	printf("compute lum (c8)...\n");
	computePseudoLuminosity(stepLumHR.data(), inDataHR, widthHR, heightHR);
	printf("compute lum (c8) done\n");
	if(_callback!=NULL)
	{
		printf("save lum (c8)...\n");
		EncodeEvent event;
		event.m_event = EncodeEvent::EncodeEvent_C8;
		event.m_data = (uintptr_t*)stepLumHR.data();
		event.m_width = widthHR;
		event.m_height = heightHR;
		_callback(event, _cbUserData);
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
		event.m_data = (uintptr_t*)clusterIdHR.data();
		event.m_width = widthHR;
		event.m_height = heightHR;
		_callback(event, _cbUserData);
		printf("save ids done\n");
	}
	
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

	printf("clear translucent ids...\n");
	clearClusterTranslucent(clusterIdHR.data(), inDataHR, widthHR, heightHR);
	printf("clear translucent ids done\n");

	printf("extract clusters...\n");
	std::map<ClusterId, VTCluster> clusterGraph;
	extractCluster(clusterGraph, widthLR, heightLR, clusterIdHR, widthHR, heightHR);
	printf("extract clusters done\n");

	unsigned layerCount = _config.m_layer;
	std::vector<std::set<ClusterId>> layers;
	printf("extract layers...\n");
	unsigned remainErrors = extractLayers(layers, layerCount, clusterGraph);
	unsigned allErrors = sumOfConflicts(clusterGraph);
	printf("remains %d errors out of %d (%.02f%%)\n", remainErrors, allErrors, (float)remainErrors*100/allErrors);
	printf("extract layers done\n");

	printf("split colors to buffers...\n");
	std::vector<uint8_t*> buffers;
	buffers.resize(layerCount);
	for(unsigned i=0; i<layerCount; ++i)
	{
		buffers[i] = _outputData.m_bufferRGBA8[i];
	}

	splitBuffer(buffers, layers, 
		widthLR, heightLR,
		clusterIdHR, inDataHR, widthHR, heightHR);
	printf("split colors to buffers done\n");
	
	_result.m_status = EncodeResult::EncodeStatus_Success;
	_result.m_msg = NULL;
}
