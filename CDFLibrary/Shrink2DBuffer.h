#ifndef SHRINK_2D_BUFFER_H
#define SHRINK_2D_BUFFER_H

#include <stdint.h>


bool shrink2DBuffer4UB(
	uint8_t* _outColorRGBA8, 
	const unsigned _outWidth, const unsigned _outHeight,
	const uint8_t* _inColorRGBA8, 
	const unsigned _inWidth, const unsigned _inHeight);

bool shrink2DBufferUI(
	unsigned* _outValues, 
	const unsigned _outWidth, const unsigned _outHeight,
	const unsigned* _inValues, 
	const unsigned _inWidth, const unsigned _inHeight);

#endif //SHRINK_2D_BUFFER_H
