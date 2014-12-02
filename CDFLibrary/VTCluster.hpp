#ifndef VTCLUSTER_HPP
#define VTCLUSTER_HPP

#include <map>
#include <vector>


typedef unsigned ClusterId;

struct VTLink
{
	unsigned m_count;

	VTLink()
	{
		m_count = 0;
	}
};

struct VTCluster
{
	unsigned m_count;
	std::map<ClusterId, VTLink> m_links;

	VTCluster()
	{
		m_count = 0;
	}

	unsigned int SumOfCount() const
	{
		unsigned int sum = 0;
		for(std::map<ClusterId, VTLink>::const_iterator it = m_links.begin();
			it!=m_links.end();
			++it)
		{
			sum += it->second.m_count;
		}
		return sum;
	}
};


inline std::map<ClusterId, VTCluster>::iterator addOrGetCluster(std::map<ClusterId, VTCluster>& _clusterGraph, ClusterId _clusterId)
{
	// insert cluster if not found
	std::map<ClusterId, VTCluster>::iterator itCluster = _clusterGraph.find(_clusterId);
	if(itCluster == _clusterGraph.end())
	{
		VTCluster cluster;
		itCluster = _clusterGraph.insert(std::pair<ClusterId, VTCluster>(_clusterId, cluster)).first;
	}
	return itCluster;
}

inline void addClusterLink(std::map<ClusterId, VTCluster>::iterator _itCluster, ClusterId _to)
{
	// insert link to other cluster if not found
	VTCluster& cluster = _itCluster->second;
	std::map<ClusterId, VTLink>::iterator itLink = cluster.m_links.find(_to);
	if(itLink == cluster.m_links.end())
	{
		VTLink link;
		itLink = cluster.m_links.insert(std::pair<ClusterId, VTLink>(_to, link)).first;
	}

	VTLink& link = itLink->second;
	link.m_count++;
}


inline void addClusterLink(std::map<ClusterId, VTCluster>& _clusterGraph,
					const unsigned _width, const unsigned _height, const unsigned _stride,
					const ClusterId* _from, const ClusterId* _to)
{
	std::vector<ClusterId> fromSet;
	std::vector<ClusterId> toSet;
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
	for(std::vector<ClusterId>::const_iterator itFrom=fromSet.begin();
		itFrom!=fromSet.end();
		++itFrom)
	{
		unsigned from = *itFrom;
		for(std::vector<ClusterId>::const_iterator itTo=toSet.begin();
			itTo!=toSet.end();
			++itTo)
		{
			unsigned to = *itTo;
			std::map<ClusterId, VTCluster>::iterator fromCluster = addOrGetCluster(_clusterGraph, from);
			if(from!=to)
			{
				addClusterLink(fromCluster, to);
				addClusterLink(addOrGetCluster(_clusterGraph, to), from);
			}
			fromCluster->second.m_count++;
		}
	}
}

#endif //VTCLUSTER_HPP