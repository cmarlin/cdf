// main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdint.h>
#include <assert.h>

#include <vector>
#include <map>
#include <set>
#include <hash_set>
#include <algorithm>
#include <iterator>
//#include "freeimage.h"
#include "tga.h"
#include "Shrink2DBuffer.h"
#include "FillArea.h"
#include "DumpBuffer.h"
#include "FillHoles.h"
#include "SplitColorToLayer.h"


/**
	1°) Cluster creation (not so easy ;-)) - coherent group of pixels (high or low res ?)
	2°) Assign Cluster to Layer & expand - minimise expand conflicts
	- generate distance field the same time
	- color graph probleme (edge conflict)
	3°) Isocontour...
	- sam
*/

void groupCluster(uint8_t* _outC8, const uint8_t* _inRGBA8888, const int _width, const int _height)
{
	for(int j=0; j<_height; ++j)
	{
		for(int i=0; i<_width; ++i)
		{
			int offset = (j*_width+i);
			int inOffset = offset*4;
			// pseudo luminosity computing
			int lum = (_inRGBA8888[inOffset+0] + (_inRGBA8888[inOffset+1]<<1) + _inRGBA8888[inOffset+2]) >> 2;
			int outOffset = offset;
			//int cluster = ((lum>>6)<<6);
			int cluster = (lum>>6);
			cluster = (cluster<<6) | (cluster<<4) | (cluster<<2) | cluster;
			_outC8[outOffset] = cluster;
		}
	}

	// TODO: remove antialiasing ?
	/*for(int j=1; j<(_height-1); ++j)
	{
		for(int i=1; i<(_width-1); ++i)
		{
			//int offset = (j*_width+i);


			_outC8[outOffset] = cluster;
		}
	}*/
}


struct VTLink
{
	unsigned int m_count;
};

struct VTCluster
{
	std::map<unsigned int, VTLink> m_links;

	unsigned int SumOfCount() const
	{
		unsigned int sum = 0;
		for(std::map<unsigned int, VTLink>::const_iterator it = m_links.begin();
			it!=m_links.end();
			++it)
		{
			sum += it->second.m_count;
		}
		return sum;
	}
};

void addClusterLink(std::map<unsigned int, VTCluster>& _clusterGraph, unsigned _from, unsigned _to)
{
	std::map<unsigned int, VTCluster>::iterator itGraph = _clusterGraph.find(_from);
	if(itGraph == _clusterGraph.end())
	{
		VTCluster cluster;
		itGraph = _clusterGraph.insert(std::pair<unsigned, VTCluster>(_from, cluster)).first;
	}

	VTCluster& cluster = itGraph->second;
	std::map<unsigned, VTLink>::iterator itCluster = cluster.m_links.find(_to);
	if(itCluster == cluster.m_links.end())
	{
		VTLink link;
		link.m_count = 0;
		itCluster = cluster.m_links.insert(std::pair<unsigned, VTLink>(_to, link)).first;
	}

	VTLink& link = itCluster->second;
	link.m_count++;
}

void addClusterLink(std::map<unsigned int, VTCluster>& _clusterGraph,
					const unsigned _width, const unsigned _height, const unsigned _stride,
					const unsigned* _from, const unsigned* _to)
{
	std::vector<unsigned> fromSet;
	std::vector<unsigned> toSet;
	for(unsigned j=0; j<_height; ++j)
	{
		for(unsigned i=0; i<_width; ++i)
		{
			unsigned cluster = _from[j*_stride + i];
			//if(fromSet.find(cluster)==fromSet.end())
			//	fromSet.insert(cluster);
			if(find(fromSet.begin(), fromSet.end(), cluster)==fromSet.end())
				fromSet.push_back(cluster);
		}
	}

	for(unsigned j=0; j<_height; ++j)
	{
		for(unsigned i=0; i<_width; ++i)
		{
			unsigned cluster = _to[j*_stride + i];
			//if(toSet.find(cluster)==toSet.end())
			//	toSet.insert(cluster);
			if(find(toSet.begin(), toSet.end(), cluster)==toSet.end())
				toSet.push_back(cluster);
		}
	}

	//std::vector<int> common_data;
	//set_intersection(from.begin(),from.end(),to.begin(),to.end(), std::back_inserter(common_data));
	//std::set<int> common_data;
	//set_intersection(from.begin(),from.end(),to.begin(),to.end(), std::inserter(common_data, common_data.begin()));
	for(std::vector<unsigned>::const_iterator itFrom=fromSet.begin();
		itFrom!=fromSet.end();
		++itFrom)
	{
		unsigned from = *itFrom;
		for(std::vector<unsigned>::const_iterator itTo=toSet.begin();
			itTo!=toSet.end();
			++itTo)
		{
			unsigned to = *itTo;
			if(from!=to)
			{
				addClusterLink(_clusterGraph, to, from);
				addClusterLink(_clusterGraph, from, to);
			}
		}
	}
}
/*
void extractCluster(std::map<unsigned int, VTCluster>& _clusterGraph, const std::vector<unsigned>& _cluster, const int _width, const int _height)
{
	for(int j=0; j<(_height-1); ++j)
	{
		for(int i=0; i<(_width-1); ++i)
		{
			int offsetUL = j*_width+i;
			int offsetUR = j*_width+i+1;
			int offsetDL = (j+1)*_width+i;
			int offsetDR = (j+1)*_width+i+1;
			//assert(offsetDR<_cluster.size());

			if(_cluster[offsetUL]!=_cluster[offsetUR]){
				addClusterLink(_clusterGraph, _cluster[offsetUL], _cluster[offsetUR]);
				addClusterLink(_clusterGraph, _cluster[offsetUR], _cluster[offsetUL]);
			}
			if(_cluster[offsetUL]!=_cluster[offsetDL]){
				addClusterLink(_clusterGraph, _cluster[offsetUL], _cluster[offsetDL]);
				addClusterLink(_clusterGraph, _cluster[offsetDL], _cluster[offsetUL]);
			}
			if(_cluster[offsetUL]!=_cluster[offsetDR]){
				addClusterLink(_clusterGraph, _cluster[offsetUL], _cluster[offsetDR]);
				addClusterLink(_clusterGraph, _cluster[offsetDR], _cluster[offsetUL]);
			}
			if(_cluster[offsetDL]!=_cluster[offsetUR]){
				addClusterLink(_clusterGraph, _cluster[offsetDL], _cluster[offsetUR]);
				addClusterLink(_clusterGraph, _cluster[offsetUR], _cluster[offsetDL]);
			}
		}
	}
}
*/
void extractCluster(std::map<unsigned int, VTCluster>& _clusterGraph, 
		const int _outWidth, const int _outHeight,
		const std::vector<unsigned>& _clusterIds, const unsigned _inWidth, const unsigned _inHeight)
{
	unsigned widthRatio = _inWidth / _outWidth;
	unsigned heightRatio = _inHeight / _outHeight;
	for(int j=0; j<(_outHeight-1); ++j)
	{
		for(int i=0; i<(_outWidth-1); ++i)
		{
			unsigned offsetUL = (j*_inWidth*widthRatio)+(i*heightRatio);
			unsigned offsetUR = (j*_inWidth*widthRatio)+((i+1)*heightRatio);
			unsigned offsetDL = ((j+1)*_inWidth*widthRatio)+(i*heightRatio);
			unsigned offsetDR = ((j+1)*_inWidth*widthRatio)+((i+1)*heightRatio);
			//assert(offsetDR<_cluster.size());

			addClusterLink(_clusterGraph, widthRatio, heightRatio, _inWidth, &_clusterIds[offsetUL], &_clusterIds[offsetUR]);
			addClusterLink(_clusterGraph, widthRatio, heightRatio, _inWidth, &_clusterIds[offsetUL], &_clusterIds[offsetDL]);
			addClusterLink(_clusterGraph, widthRatio, heightRatio, _inWidth, &_clusterIds[offsetUL], &_clusterIds[offsetDR]);
			addClusterLink(_clusterGraph, widthRatio, heightRatio, _inWidth, &_clusterIds[offsetDL], &_clusterIds[offsetUR]);
		}
	}
}

struct greater_than_key
{
    inline bool operator() (const std::pair<unsigned int, unsigned int>& clusterConflict1, const std::pair<unsigned int, unsigned int>& clusterConflict2)
    {
		return (clusterConflict1.second > clusterConflict2.second);
    }
};

void countConflictsPerLayer(
	std::vector<unsigned>& _conflictsPerLayer, 
	const unsigned int _cluster, 
	const std::vector<std::set<unsigned int>>& _layers, 
	const std::map<unsigned int, VTCluster>& _clusterGraph)
{
	_conflictsPerLayer.assign(_layers.size(), 0);

	std::map<unsigned int, VTCluster>::const_iterator it = _clusterGraph.find(_cluster);
	const VTCluster& cluster = it->second;
	assert(it != _clusterGraph.end());

	for(std::map<unsigned int, VTLink>::const_iterator itLink = cluster.m_links.begin(); 
		itLink!=cluster.m_links.end(); 
		++itLink)
	{
		unsigned otherCluster = itLink->first;

		for(unsigned layerIndex = 0;
			layerIndex < _layers.size();
			layerIndex++)
		{

			if(_layers[layerIndex].find(otherCluster) != _layers[layerIndex].end())
			{
				_conflictsPerLayer[layerIndex] += itLink->second.m_count; 
				break;
			}
		}
	}
}

unsigned extractLayers(std::vector<std::set<unsigned int>>& _layers,
					const int _layerCount,
					const std::map<unsigned int, VTCluster>& _clusterGraph)
{
	unsigned remainingConflicts = 0;
	// find the unassigned cluster with the most possible conflicts
	std::vector<std::pair<unsigned int, unsigned int>> clusterAndSumOfConflicts;
	clusterAndSumOfConflicts.reserve(_clusterGraph.size());
	for(std::map<unsigned int, VTCluster>::const_iterator it = _clusterGraph.begin(); it!=_clusterGraph.end(); it++)
	{
		std::pair<unsigned, unsigned> clusterWithFullCost;
		clusterWithFullCost.first = it->first;
		clusterWithFullCost.second = it->second.SumOfCount();
		clusterAndSumOfConflicts.push_back(clusterWithFullCost);
	}

	std::sort(clusterAndSumOfConflicts.begin(), clusterAndSumOfConflicts.end(), greater_than_key());
	// assign clusters to layers with minimum conflicts

	_layers.resize(_layerCount);
	std::vector<unsigned int> layersConflict;
	//layersConflict.resize(_layers.size());
	for(std::vector<std::pair<unsigned int, unsigned int>>::iterator itCluster = clusterAndSumOfConflicts.begin();
		itCluster != clusterAndSumOfConflicts.end();
		itCluster ++)
	{
		unsigned int clusterId = itCluster->first;
		
		//layersConflict.clear();
		countConflictsPerLayer(layersConflict, clusterId, _layers, _clusterGraph);
		unsigned int minConflict = UINT_MAX;
		int layerWithLeastConflicts = -1;
		for(unsigned i=0; i<layersConflict.size(); ++i)
		{
			if(layersConflict[i]<minConflict)
			{
				minConflict = layersConflict[i];
				layerWithLeastConflicts = i;
			}
		}
		remainingConflicts += minConflict;
		_layers[layerWithLeastConflicts].insert(clusterId);
	}
	return remainingConflicts;
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
		TGA_HEADER inHeader;
		TGA_ReadHeader(&inHeader, fin);
		inDataHR = new uint8_t[inHeader.width * inHeader.height * sizeof(uint32_t)];
		TGA_ReadDataRGBA8(inDataHR, &inHeader, fin);
		widthHR = inHeader.width;
		heightHR = inHeader.height;
		fclose(fin);
	}
	else
	{
		return 0;
	}

	//uint8_t* stepLumHR = new uint8_t[widthHR * heightHR];
	std::vector<uint8_t> stepLumHR(widthHR * heightHR);
	groupCluster(stepLumHR.data(), inDataHR, widthHR, heightHR);
	dumpBufferUB("out-c8.tga", stepLumHR.data(), widthHR, heightHR);
	
	std::vector<unsigned> clusterIdHR(widthHR * heightHR);
	unsigned clusterCount = tagCluster(clusterIdHR.data(), stepLumHR.data(), widthHR, heightHR);
	dumpBufferUI("out-ids.tga", clusterIdHR.data(), widthHR, heightHR);
	
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
		std::vector<uint8_t> inDataLR(widthLR * heightLR * 4);
		if(!shrink2DBuffer4UB(inDataLR.data(), widthLR, heightLR, inDataHR, widthHR, heightHR))
		{
			fprintf(stderr, "failed to shrink input image, not a multiple of compression factor? %d => %d", widthHR, shrinkFactor);
			return 0;
		}
		dumpBuffer4UB("out-ref.tga", inDataLR.data(), widthLR, heightLR);
	}


	std::map<unsigned int, VTCluster> clusterGraph;
	//extractCluster(clusterGraph, clusterIdLR, widthLR, heightLR);
	extractCluster(clusterGraph, widthLR, heightLR, clusterIdHR, widthHR, heightHR);

	unsigned layerCount = 3;
	std::vector<std::set<unsigned>> layers;
	extractLayers(layers, layerCount, clusterGraph);

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

	for(unsigned i=0; i<layerCount; ++i)
	{
		delete[] buffers[i];
	}
	buffers.clear();

	//if(stepLumHR!=NULL){delete[] stepLumHR; stepLumHR=NULL;}
	if(inDataHR!=NULL){delete[] inDataHR; inDataHR=NULL;}
		
	return 0;
}

	