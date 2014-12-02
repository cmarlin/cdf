#include "stdafx.h"

#include "ExtractCluster.hpp"

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
void extractCluster(std::map<ClusterId, VTCluster>& _clusterGraph, 
		const int _outWidth, const int _outHeight,
		const std::vector<ClusterId>& _clusterIds, const unsigned _inWidth, const unsigned _inHeight)
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
