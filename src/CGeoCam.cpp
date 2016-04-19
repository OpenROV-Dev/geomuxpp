// Includes
#include "CGeoCam.h"
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <cstring>

#include <chrono>

#include "easylogging.hpp"
#include "CVideoBuffer.h"

// Namespaces
using namespace std;

CGeoCam::CGeoCam( CVideoBuffer *videoBufferIn )
	: m_pInputBuffer( videoBufferIn )
{
	
}

CGeoCam::~CGeoCam()
{
	Cleanup();
}

void CGeoCam::InitDevice( int deviceOffsetIn, video_channel_t channelIn )
{
	// Set a custom device offset
	stringstream options( "dev_offset=" );
	options << deviceOffsetIn;
	
	// Initialize mxuvc
	if( mxuvc_video_init( "v4l2", options.str().c_str() ) )
	{
		LOG( ERROR ) << "Init fail";
		throw std::runtime_error( "Failed to initialize mxuvc!" );
	}
	
	// Register our video callback
	if( mxuvc_video_register_cb( channelIn, CGeoCam::VideoCallback, m_pInputBuffer ) )
	{
		LOG( ERROR ) << "Register fails";
		Cleanup();
		throw std::runtime_error( "Failed to register video callback!" );
	}
}

bool CGeoCam::StartVideo()
{
	return ( mxuvc_video_start( CH_MAIN ) == 0 );
}

bool CGeoCam::StopVideo()
{
	return ( mxuvc_video_stop( CH_MAIN ) == 0 );
}

bool CGeoCam::ForceIFrame()
{
	return ( mxuvc_video_force_iframe( CH_MAIN ) == 0 );
}

bool CGeoCam::IsAlive()
{
	// Check to see if camera is available. False if its been unplugged or USB host is not responding
	// Note: Documentation is wrong. 1 == Alive, 0 == Dead
	return ( mxuvc_video_alive() == 1 );
}

void CGeoCam::EnableVUI()
{
	mxuvc_video_set_vui( CH_MAIN, 1 );
}

void CGeoCam::GetPictureTiming()
{
	uint32_t temp;
	mxuvc_video_get_pict_timing( CH_MAIN, &temp );
	
	LOG( INFO ) << "Pict timing: " << temp;
}

void CGeoCam::SetPictureTiming( uint32_t pictTimingIn )
{
	mxuvc_video_set_pict_timing( CH_MAIN, pictTimingIn );
}

void CGeoCam::Cleanup()
{
	// Deinitialize mxuvc resources and exit. Automatically calls mxuvc_video_stop()
	mxuvc_video_deinit();
}

// This gets called by the MXUVC library every time we have a NAL available
void CGeoCam::VideoCallback( unsigned char *dataBufferOut, unsigned int bufferSizeIn, video_info_t infoIn, void *userDataIn )
{
	CVideoBuffer *inputBuffer = (CVideoBuffer*)userDataIn;

	LOG(INFO) << "Got video data: " << bufferSizeIn << " bytes";
	
	if( inputBuffer )
	{
		inputBuffer->Write( dataBufferOut, bufferSizeIn );
	}
	
	// Releases the buffer back to the MXUVC
	mxuvc_video_cb_buf_done( CH_MAIN, infoIn.buf_index );
}
