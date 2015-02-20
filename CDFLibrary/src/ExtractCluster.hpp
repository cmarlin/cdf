#ifndef EXTRACT_VTCLUSTER_HPP
#define EXTRACT_VTCLUSTER_HPP

#include <map>
#include <vector>
#include "VTCluster.hpp"

void extractCluster(std::map<unsigned int, VTCluster>& _clusterGraph, 
		const int _outWidth, const int _outHeight,
		const std::vector<unsigned>& _clusterIds, const unsigned _inWidth, const unsigned _inHeight);

#endif //EXTRACT_VTCLUSTER_HPP