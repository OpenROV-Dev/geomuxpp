#pragma once
 
// Includes
#include <CpperoMQ/All.hpp>
#include "CVideoBuffer.h"

#include <thread>
#include <mutex> 
#include <atomic>
 
extern "C" 
{ 
	// FFmpeg
	#include <libavutil/avassert.h>
	#include <libavutil/channel_layout.h>
	#include <libavutil/opt.h>
	#include <libavutil/log.h>
	#include <libavutil/time.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/timestamp.h>
	#include "libavutil/pixfmt.h"
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
	#include <libavformat/avio.h>
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

enum class EVideoFormat
{
	H264,
	MJPEG,
	UNKNOWN
};

class CMuxer
{
public:
	CVideoBuffer 		m_inputBuffer;
	
	std::atomic<bool> 	m_killThread;
	
	std::thread 		m_thread;

	// Methods
	CMuxer( CpperoMQ::Context *contextIn, const std::string &endpointIn, EVideoFormat formatIn );
	virtual ~CMuxer();
	
	// Methods
	void Initialize();
	void Update();
	void ThreadLoop();
		
	EVideoFormat 				m_format;

	CpperoMQ::Context 			*m_pContext;
	CpperoMQ::PublishSocket 	m_dataPub;
	
	size_t				m_avioContextBufferSize		= 4000000; // ~4mb
	
	int64_t				m_timestamp					= 0;
	int64_t				m_streamTimebase			= 0;

	
	bool				m_holdBuffer 				= false;
	bool				m_isComposingInitFrame 		= false;
	
	// Input structures
	AVFormatContext 	*m_pInputFormatContext 		= NULL;
	AVIOContext 		*m_pInputAvioContext 		= NULL;
	AVCodecContext 		*m_pInputCodecContext		= NULL;

	uint8_t*			m_pInputAvioContextBuffer	= NULL;
	
	// Output structures
	AVFormatContext 	*m_pOutputFormatContext 	= NULL;
	AVOutputFormat 		*m_pOutputFormat 			= NULL;
	AVIOContext 		*m_pOutputAvioContext 		= NULL;
	AVCodecContext 		*m_pOutputCodecContext		= NULL;
	
	uint8_t*			m_pOutputAvioContextBuffer	= NULL;		

	bool 				m_canMux 					= false;
	bool 				m_formatAcquired 			= false;
	
	AVDictionary 		*m_pMuxerOptions			= NULL;

	// Custom read function for ffmpeg
	static int ReadPacket( void *muxerIn, uint8_t *avioBufferOut, int avioBufferSizeAvailableIn );
	static int WritePacket( void *sharedDataIn, uint8_t *avioBufferIn, int bytesAvailableIn );
	
};