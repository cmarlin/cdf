#ifndef SPLIT_COLOR_TO_LAYER_H
#define SPLIT_COLOR_TO_LAYER_H

#include <stdint.h>
#include <vector>
#include <set>


bool splitBuffer(std::vector<uint8_t*>& _outColorsRGBA8, 
				 const std::vector<std::set<unsigned>>& _layers, 
				 const unsigned _widthLR, const unsigned _heightLR,
				 const std::vector<unsigned>& _clusterIdHR, const uint8_t* _inColorRGBA8HR, const unsigned _widthHR, const unsigned _heightHR);


#endif 