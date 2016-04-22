#pragma once

// Includes
#include <mxuvc.h>
#include <libmxcam.h>

#include <zmq.hpp>
#include <CpperoMQ/All.hpp>
#include <json.hpp>

#include <unordered_map>
#include <functional>

// Defines
#define VIDEO_BACKEND "\"v4l2\""

// Typedefs
typedef std::unordered_map<std::string, std::function<void( const nlohmann::json &commandIn )>> TApiFunctionMap;

class CVideoChannel
{
public:
	// Methods
	CVideoChannel( video_channel_t channelIn );
	virtual ~CVideoChannel();

	void HandleMessage( const nlohmann::json &commandIn );

private:
	video_channel_t 				m_channel;
	
	TApiFunctionMap 				m_apiMap;
	
	///////////////////////////////////////
	// Private Channel API
	///////////////////////////////////////
	void Initialize();
	void Cleanup();
	void RegisterAPIFunctions();
	void GetVideoSettings();
	bool IsAlive();
	
	///////////////////////////////////////
	// Public channel API
	///////////////////////////////////////
	
	// General
	void StartVideo( const nlohmann::json &commandIn );
	void StopVideo( const nlohmann::json &commandIn );
	
	void SetMultipleSettings( const nlohmann::json &commandIn );		// This command accepts a json object with multiple settings and sets them all at once

	void SetFramerate( const nlohmann::json &commandIn );
	void SetBitrate( const nlohmann::json &commandIn );
	
	// H264
	void ForceIFrame( const nlohmann::json &commandIn );
	void SetGOPLength( const nlohmann::json &commandIn );
	void SetGOPHierarchy( const nlohmann::json &commandIn );
	void SetAVCProfile( const nlohmann::json &commandIn );
	void SetAVCLevel( const nlohmann::json &commandIn );
	void SetMaxNALSize( const nlohmann::json &commandIn );
	void EnableVUI( const nlohmann::json &commandIn );
	void EnablePictTiming( const nlohmann::json &commandIn );
	void SetMaxIFrameSize( const nlohmann::json &commandIn );
	
	// MJPEG
	void SetCompressionQuality( const nlohmann::json &commandIn );
	
	// Sensor
	void EnableFlipVertical( const nlohmann::json &commandIn );
	void EnableFlipHorizontal( const nlohmann::json &commandIn );
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

};