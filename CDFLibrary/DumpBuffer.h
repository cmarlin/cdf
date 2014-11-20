#ifndef DUMP_BUFFER_H
#define DUMP_BUFFER_H

void dumpBufferUB(const char* filename, const uint8_t* _data, const int _width, const int _height);
void dumpBuffer4UB(const char* filename, const uint8_t* _data, const int _width, const int _height);
void dumpBufferUI(const char* filename, const unsigned int * _data, const int _width, const int _height);

#endif
