#ifndef EXTRACT_LAYER_HPP
#define EXTRACT_LAYER_HPP

#include "stdafx.h"
#include "VTCluster.hpp"

unsigned extractLayers(std::vector<std::set<ClusterId>>& _layers,
					const int _layerCount,
					const std::map<ClusterId, VTCluster>& _clusterGraph);


#endif //EXTRACT_LAYER_HPP