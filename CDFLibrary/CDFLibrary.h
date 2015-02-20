#ifndef CDF_LIBRARY_H
#define CDF_LIBRARY_H

#include <stdint.h>


// version
#define CDF_LIBRARY_VERSION_MAJOR 0
#define CDF_LIBRARY_VERSION_MINOR 0

//
#define CDF_MAX_LAYERS 8

class EncodeEvent
{
public:
	enum EventEnum
	{
		EncodeEvent_Result = 0, // final output ? 
		EncodeEvent_C8,
		EncodeEvent_ClusterId,
	};

	EventEnum	m_event;
	uintptr_t	m_data;
	unsigned	m_width;
	unsigned	m_height;
};
typedef void (*CDF_Encode_CB)(EncodeEvent,uintptr_t);

class InputData
{
public:
	unsigned m_width;
	unsigned m_height;
	uint8_t* m_bufferRGBA8;

	InputData()
		:m_width(0)
		,m_height(0)
		,m_bufferRGBA8(NULL)
	{
	}

};

class OutputData
{
public:
	unsigned m_width;
	unsigned m_height;
	uint8_t* m_bufferRGBA8[CDF_MAX_LAYERS];

	OutputData()
		:m_width(0)
		,m_height(0)
	{
		for(int i=0; i<CDF_MAX_LAYERS; ++i)
		{
			m_bufferRGBA8[i] = NULL;
		}
	}
};

class EncodeConfiguration
{
public:
	unsigned	m_layer;	/// [0;CDF_MAX_LAYERS]

	EncodeConfiguration()
		:m_layer(0)
	{
	}
};

class EncodeResult
{
public:
	enum EncodeStatus
	{
		EncodeStatus_Success=0,
		EncodeStatus_Failed,
	};

	EncodeStatus	m_status;
	char*			m_msg;		// null if invalid

	EncodeResult()
		:m_status(EncodeStatus_Success)
		,m_msg(NULL)
	{
	}
};

void EncodeCDF(EncodeResult& _result, const EncodeConfiguration& _config,
				 const OutputData& _outputData, const InputData& _inputData, 
				 const CDF_Encode_CB _callback=NULL, const uintptr_t _cbUserData=0);

#endif //CDF_LIBRARY_H