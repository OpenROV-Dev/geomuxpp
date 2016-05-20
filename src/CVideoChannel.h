#pragma once

// Includes
#include <mxuvc.h>
#include <CpperoMQ/All.hpp>
#include <json.hpp>

#include <unordered_map>
#include <functional>

#include "CMuxer.h"

// Forward decs
class CStatusPublisher;

// Defines
#define VIDEO_BACKEND "\"v4l2\""

// Typedefs
typedef std::unordered_map<std::string, std::function<void( const nlohmann::json &commandIn )>> TApiFunctionMap;
typedef std::unordered_map<std::string, std::function<void()>> TGetAPIMap;


class CVideoChannel
{
public:
	// Methods
	CVideoChannel( const std::string &cameraOffsetIn, video_channel_t channelIn, CpperoMQ::Context *contextIn, CStatusPublisher *publisherIn );
	virtual ~CVideoChannel();

	bool IsAlive();
	void HandleMessage( const nlohmann::json &commandIn );

private:
	
	video_channel_t 				m_channel;
	std::string						m_cameraString;
	std::string						m_channelString;
	std::string 					m_endpoint;
	
	CStatusPublisher 				*m_pStatusPublisher;
	
	nlohmann::json 					m_settings;
	
	TApiFunctionMap 				m_apiMap;
	TGetAPIMap 						m_getAPIMap;
	
	CMuxer							m_muxer;
	
	static void VideoCallback( unsigned char *dataBufferOut, unsigned int bufferSizeIn, video_info_t infoIn, void *userDataIn );
	
	///////////////////////////////////////
	// Private Channel API
	///////////////////////////////////////
	void RegisterAPIFunctions();
	
	///////////////////////////////////////
	// Public channel API
	///////////////////////////////////////
	
	//---------
	// Action API
	//---------
	
	// General
	void StartVideo( const nlohmann::json &commandIn );
	void StopVideo( const nlohmann::json &commandIn );
	
	// H264
	void ForceIFrame( const nlohmann::json &commandIn );
	
	//---------
	// Set API
	//---------
	
	// General
	void SetMultipleSettings( const nlohmann::json &commandIn );		// This command accepts a json object with multiple settings and sets them all at once
	
	void SetFramerate( const nlohmann::json &commandIn );
	void SetBitrate( const nlohmann::json &commandIn );
	void PublishSettings( const nlohmann::json &commandIn );
	void PublishHealthStats( const nlohmann::json &commandIn );
	
	// H264
	void SetGOPLength( const nlohmann::json &commandIn );
	void SetGOPHierarchy( const nlohmann::json &commandIn );
	void SetAVCProfile( const nlohmann::json &commandIn );
	void SetAVCLevel( const nlohmann::json &commandIn );
	void SetMaxNALSize( const nlohmann::json &commandIn );
	void SetVUI( const nlohmann::json &commandIn );
	void SetPictTiming( const nlohmann::json &commandIn );
	void SetMaxIFrameSize( const nlohmann::json &commandIn );
	
	// MJPEG
	void SetCompressionQuality( const nlohmann::json &commandIn );
	
	// Sensor
	void SetFlipVertical( const nlohmann::json &commandIn );
	void SetFlipHorizontal( const nlohmann::json &commandIn );
	void SetContrast( const nlohmann::json &commandIn );
	void SetZoom( const nlohmann::json &commandIn );
	void SetPan( const nlohmann::json &commandIn );
	void SetTilt( const nlohmann::json &commandIn );
	void SetPantilt( const nlohmann::json &commandIn );
	void SetBrightness( const nlohmann::json &commandIn );
	void SetHue( const nlohmann::json &commandIn );
	void SetGamma( const nlohmann::json &commandIn );
	void SetSaturation( const nlohmann::json &commandIn );
	void SetGain( const nlohmann::json &commandIn );
	void SetSharpness( const nlohmann::json &commandIn );
	void SetMaxAnalogGain( const nlohmann::json &commandIn );
	void SetHistogramEQ( const nlohmann::json &commandIn );
	void SetSharpenFilter( const nlohmann::json &commandIn );
	void SetMinAutoExposureFramerate( const nlohmann::json &commandIn );
	void SetTemporalFilterStrength( const nlohmann::json &commandIn );
	void SetGainMultiplier( const nlohmann::json &commandIn );
	void SetExposureMode( const nlohmann::json &commandIn );
	void SetNoiseFilterMode( const nlohmann::json &commandIn );
	void SetWhiteBalanceMode( const nlohmann::json &commandIn );
	void SetWideDynamicRangeMode( const nlohmann::json &commandIn );
	void SetZoneExposure( const nlohmann::json &commandIn );
	void SetZoneWhiteBalance( const nlohmann::json &commandIn );
	void SetPowerLineFrequency( const nlohmann::json &commandIn );
	
	
	
	///////////////
	// Get API
	///////////////
	
	// General
	void GetAllSettings();
	
	void GetChannelInfo();
	void GetFramerate();
	void GetBitrate();
	
	// H264
	void GetGOPLength();
	void GetGOPHierarchy();
	void GetAVCProfile();
	void GetAVCLevel();
	void GetMaxNALSize();
	void GetVUI();
	void GetPictTiming();
	void GetMaxIFrameSize();
	
	// MJPEG
	void GetCompressionQuality();
	
	// Sensor
	void GetFlipVertical();
	void GetFlipHorizontal();
	void GetContrast();
	void GetZoom();
	void GetPan();
	void GetTilt();
	void GetPantilt();
	void GetBrightness();
	void GetHue();
	void GetGamma();
	void GetSaturation();
	void GetGain();
	void GetSharpness();
	void GetMaxAnalogGain();
	void GetHistogramEQ();
	void GetSharpenFilter();
	void GetMinAutoExposureFramerate();
	void GetTemporalFilterStrength();
	void GetGainMultiplier();
	void GetExposureMode();
	void GetNoiseFilterMode();
	void GetWhiteBalanceMode();
	void GetWideDynamicRangeMode();
	void GetZoneExposure();
	void GetZoneWhiteBalance();
	void GetPowerLineFrequency();
};