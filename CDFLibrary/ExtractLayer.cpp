#include "stdafx.h"
#include <assert.h>

#include "ExtractLayer.hpp"

struct greater_than_key
{
    inline bool operator() (const std::pair<ClusterId, unsigned>& clusterConflict1, const std::pair<ClusterId, unsigned>& clusterConflict2)
    {
		return (clusterConflict1.second > clusterConflict2.second);
    }
};


static void countConflictsPerLayer(
	std::vector<unsigned>& _conflictsPerLayer, 
	const ClusterId _cluster, 
	const std::vector<std::set<ClusterId>>& _layers, 
	const std::map<ClusterId, VTCluster>& _clusterGraph)
{
	_conflictsPerLayer.assign(_layers.size(), 0);

	std::map<ClusterId, VTCluster>::const_iterator it = _clusterGraph.find(_cluster);
	const VTCluster& cluster = it->second;
	assert(it != _clusterGraph.end());

	for(std::map<ClusterId, VTLink>::const_iterator itLink = cluster.m_links.begin(); 
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

unsigned extractLayers(std::vector<std::set<ClusterId>>& _layers,
					const int _layerCount,
					const std::map<ClusterId, VTCluster>& _clusterGraph)
{
	unsigned remainingConflicts = 0;
	// find the unassigned cluster with the most possible conflicts
	std::vector<std::pair<ClusterId, unsigned>> clusterAndSumOfConflicts;
	clusterAndSumOfConflicts.reserve(_clusterGraph.size());
	for(std::map<ClusterId, VTCluster>::const_iterator it = _clusterGraph.begin(); it!=_clusterGraph.end(); it++)
	{
		std::pair<ClusterId, unsigned> clusterWithFullCost;
		clusterWithFullCost.first = it->first;
		//clusterWithFullCost.second = it->second.SumOfCount();
		unsigned conflictCount = it->second.SumOfCount();
		unsigned eltCount = it->second.m_count;
		unsigned cost = conflictCount + (conflictCount / eltCount) * SHRT_MAX;
		clusterWithFullCost.second = cost;
		clusterAndSumOfConflicts.push_back(clusterWithFullCost);
	}

	std::sort(clusterAndSumOfConflicts.begin(), clusterAndSumOfConflicts.end(), greater_than_key());
	// assign clusters to layers with minimum conflicts

	_layers.resize(_layerCount);
	std::vector<unsigned> layersConflict;
	//layersConflict.resize(_layers.size());
	for(std::vector<std::pair<ClusterId, unsigned>>::iterator itCluster = clusterAndSumOfConflicts.begin();
		itCluster != clusterAndSumOfConflicts.end();
		itCluster ++)
	{
		ClusterId clusterId = itCluster->first;
		
		//layersConflict.clear();
		countConflictsPerLayer(layersConflict, clusterId, _layers, _clusterGraph);
		unsigned minConflict = UINT_MAX;
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

