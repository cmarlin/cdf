#pragma once

#include <utility>
#include <stdint.h>
//#include <G3D/G3D.h>
//#include <G3D/PlatformTypes.h>
//
//using namespace G3D;

class SEDT
{
private:
	//Vector2int16 *distMap;
	std::pair<int16_t, int16_t>* distMap;
	int w, h;

public:
	SEDT();

	void release();

	void init(int w, int h, uint8_t *binImg, bool invert = false);

	void updatePix(int idx, int idxOffset, int x, int y, int xOffset, int yOffset );

	void compute(float *outDist);

	~SEDT();
};