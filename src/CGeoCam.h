#pragma once
 
// Includes
#include <mxuvc.h>
#include <libmxcam.h>

// Defines
#define GEOCAM_DEFAULT_DEVICE_OFFSET 0		// TODO: Eventually remove this construct and have the javascript or some other agent request a specific interface

#define VIDEO_BACKEND "\"v4l2\""

// Forward decs
class CVideoBuffer;

class CGeoCam
{
public:
	// Attributes
	CVideoBuffer* m_pInputBuffer;
	
	int m_deviceOffset = 0;
	
	// Methods
	CGeoCam( CVideoBuffer *videoBufferIn );
	
	virtual ~CGeoCam();
	
	// API
	void InitDevice( int deviceOffsetIn = GEOCAM_DEFAULT_DEVICE_OFFSET, video_channel_t channelIn = CH_MAIN );
	bool StartVideo();
	bool StopVideo();
	bool IsAlive();
	bool ForceIFrame();
	
	void SetFramerate( uint32_t framerateIn );
	void GetFramerate();
	
	void SetGOP( uint32_t gopIn );
	void GetGOP();
	
	void SetBitrate( uint32_t bitrateIn );
	void GetBitrate();
	
	void SetProfile( video_profile_t profileIn );
	void GetProfile();
	
	// If this parameter is set to 0 the camera sends variable size NAL units. If it is set to finite value the camera splits the frame into multiple equal size NAL units with the maximum size equal to the parameter value. 
	void SetMaxNALUnitSize( uint32_t bitrateIn );
	void GetMaxNALUnitSize();
	
	void SetAVCLevel( uint32_t avcLevelIn );
	void GetAVCLevel();
	
	// VUI Contains desired framerate data
	void EnableVUI();
	void DisableVUI();
	
	void GetPictureTiming();
	void SetPictureTiming( uint32_t pictTimingIn );
	
	void Cleanup();
	
	static void VideoCallback( unsigned char *dataBufferOut, unsigned int bufferSizeIn, video_info_t infoIn, void *userDataIn );
};