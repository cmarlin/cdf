#ifndef TGA_HEADER_H
#define TGA_HEADER_H

#include <stdio.h>
#include <string.h>


#define IMAGE_TYPE_BGR (0x02)
#define IMAGE_TYPE_RLE (0x08)
#define DESCRIPTOR_H_FLIP (0x32)

#pragma pack(1)   // n=1

struct TGA_HEADER
{
    unsigned char  identsize;          // size of ID field that follows 18 byte header (0 usually)
    unsigned char  colourmaptype;      // type of colour map 0=none, 1=has palette
    unsigned char  imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    short colourmapstart;     // first colour map entry in palette
    short colourmaplength;    // number of colours in palette
    unsigned char  colourmapbits;      // number of bits per palette entry 15,16,24,32

    short xstart;             // image x origin
    short ystart;             // image y origin
    short width;              // image width in pixels
    short height;             // image height in pixels
    unsigned char  bits;               // image bits per pixel 8,16,24,32
    unsigned char  descriptor;         // image descriptor bits (vh flip bits)
    
    // pixel data follows header
    
};

// datas in 24 bits are formated a 'BGR'



inline void TGA_ReadHeader(TGA_HEADER* _header, FILE *_fin)
{
	fread(&(_header->identsize), sizeof(_header->identsize), 1, _fin);
    fread(&_header->colourmaptype, sizeof(_header->colourmaptype), 1, _fin);
    fread(&_header->imagetype, sizeof(_header->imagetype), 1, _fin);

    fread(&_header->colourmapstart, sizeof(_header->colourmapstart), 1, _fin);
    fread(&_header->colourmaplength, sizeof(_header->colourmaplength), 1, _fin);
    fread(&_header->colourmapbits, sizeof(_header->colourmapbits), 1, _fin);

    fread(&_header->xstart, sizeof(_header->xstart), 1, _fin);
    fread(&_header->ystart, sizeof(_header->ystart), 1, _fin);
    fread(&_header->width, sizeof(_header->width), 1, _fin);
    fread(&_header->height, sizeof(_header->height), 1, _fin);
    fread(&_header->bits, sizeof(_header->bits), 1, _fin);
    fread(&_header->descriptor, sizeof(_header->descriptor), 1, _fin);

	unsigned char dummy[255];
	fread(dummy, 1, _header->identsize, _fin);
}

inline void TGA_ReadDataRGBA8(void* _data, const TGA_HEADER* _header, FILE *_fin)
{
	int iBytesPerPixel = (_header->bits/8);
	unsigned char* dataRead = (unsigned char*)_data;
	memset(dataRead, 0, _header->width*_header->height*4);
	if((_header->imagetype&IMAGE_TYPE_RLE) == 0)
	{
		for(int j=0; j<_header->height; j++)
		{
			for(int i=0; i<_header->width; i++)
			{
				int read = fread(dataRead, 1, iBytesPerPixel, _fin);
				dataRead += 4;
			}
		}
	}
	else
	{
		int iPixelCount = _header->width*_header->height;
		int iPixelCurrent = 0;
		while(iPixelCurrent<iPixelCount)
		{	
			unsigned char iRLE = 0;
			int read = fread(&iRLE, sizeof(unsigned char), 1, _fin);
			if(iRLE&0x80)
			{
				// copy same value
				fread(dataRead, 1, iBytesPerPixel, _fin);
				unsigned char* pColor = dataRead;
				dataRead += 4;
				while(iRLE)
				{
					memcpy(dataRead, pColor, iBytesPerPixel);
					dataRead += 4;
					iRLE--;
				}
			}
			else
			{
				while(iRLE)
				{
					fread(dataRead, 1, iBytesPerPixel, _fin);
					dataRead += 4;
					iRLE--;
				}
			}
		}
	}

	// swap from bgra to rgba
	for(int k=0; k<(_header->width*_header->height); k++)
	{
		dataRead = &((unsigned char*)_data)[k*4];
		unsigned char r = dataRead[0];
		unsigned char b = dataRead[2];
		dataRead[2] = r;
		dataRead[0] = b;
	}
}

inline void writeTGA(const TGA_HEADER* _header, FILE *fin)
{
	fwrite(&_header->identsize, sizeof(_header->identsize), 1, fin);
    fwrite(&_header->colourmaptype, sizeof(_header->colourmaptype), 1, fin);
    fwrite(&_header->imagetype, sizeof(_header->imagetype), 1, fin);

    fwrite(&_header->colourmapstart, sizeof(_header->colourmapstart), 1, fin);
    fwrite(&_header->colourmaplength, sizeof(_header->colourmaplength), 1, fin);
    fwrite(&_header->colourmapbits, sizeof(_header->colourmapbits), 1, fin);

    fwrite(&_header->xstart, sizeof(_header->xstart), 1, fin);
    fwrite(&_header->ystart, sizeof(_header->ystart), 1, fin);
    fwrite(&_header->width, sizeof(_header->width), 1, fin);
    fwrite(&_header->height, sizeof(_header->height), 1, fin);
    fwrite(&_header->bits, sizeof(_header->bits), 1, fin);
    fwrite(&_header->descriptor, sizeof(_header->descriptor), 1, fin);
}

#pragma pack()   // default

#endif //TGA_HEADER
