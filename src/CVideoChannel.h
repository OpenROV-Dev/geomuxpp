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
typedef std::unordered_map<std::string, std::function<void( const nlohmann::json &paramsIn )>> TApiFunctionMap;
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
	nlohmann::json 					m_api;
	
	TApiFunctionMap 				m_publicApiMap;
	TApiFunctionMap 				m_settingsApiMap;
	TGetAPIMap 						m_privateApiMap;
	
	CMuxer							m_muxer;
	
	static void VideoCallback( unsigned char *dataBufferOut, unsigned int bufferSizeIn, video_info_t infoIn, void *userDataIn );
	
	void LoadAPI();
	void RegisterAPIFunctions();
	
	bool IsCommandSupported( const std::string &commandNameIn );
	bool IsSettingSupported( const std::string &settingNameIn );
	
	void ValidateCommand( const nlohmann::json &commandIn );
	void ValidateSetting( const std::string &settingNameIn, const nlohmann::json &settingIn );
	
	void ValidateParameterType( const nlohmann::json &paramApiIn, const nlohmann::json &paramIn );
	void ValidateParameterValue( const nlohmann::json &paramApiIn, const nlohmann::json &paramIn );
	
	///////////////////////////////////////
	// Public channel API
	///////////////////////////////////////
	
	//--------------------
	// Action API
	
	// Publish commands
	void PublishSettings( const nlohmann::json &paramsIn );
	void PublishHealthStats( const nlohmann::json &paramsIn );
	void PublishAPI( const nlohmann::json &paramsIn );
	
	// General
	void StartVideo( const nlohmann::json &paramsIn );
	void StopVideo( const nlohmann::json &paramsIn );
	
	// H264
	void ForceIFrame( const nlohmann::json &paramsIn );
	
	//--------------------
	// Settings API
	
	// This command accepts a json object with multiple settings and sets them all at once
	void ApplySettings( const nlohmann::json &paramsIn );		
	
	// General
	void SetFramerate( const nlohmann::json &paramsIn );
	void SetBitrate( const nlohmann::json &paramsIn );
	
	// H264
	void SetGOPLength( const nlohmann::json &paramsIn );
	void SetGOPHierarchy( const nlohmann::json &paramsIn );
	void SetAVCProfile( const nlohmann::json &paramsIn );
	void SetAVCLevel( const nlohmann::json &paramsIn );
	void SetMaxNALSize( const nlohmann::json &paramsIn );
	void SetVUI( const nlohmann::json &paramsIn );
	void SetPictTiming( const nlohmann::json &paramsIn );
	void SetMaxIFrameSize( const nlohmann::json &paramsIn );
	
	// MJPEG
	void SetCompressionQuality( const nlohmann::json &paramsIn );
	
	// Sensor
	void SetFlipVertical( const nlohmann::json &paramsIn );
	void SetFlipHorizontal( const nlohmann::json &paramsIn );
	void SetContrast( const nlohmann::json &paramsIn );
	void SetZoom( const nlohmann::json &paramsIn );
	void SetPan( const nlohmann::json &paramsIn );
	void SetTilt( const nlohmann::json &paramsIn );
	void SetPantilt( const nlohmann::json &paramsIn );
	void SetBrightness( const nlohmann::json &paramsIn );
	void SetHue( const nlohmann::json &paramsIn );
	void SetGamma( const nlohmann::json &paramsIn );
	void SetSaturation( const nlohmann::json &paramsIn );
	void SetGain( const nlohmann::json &paramsIn );
	void SetSharpness( const nlohmann::json &paramsIn );
	void SetMaxAnalogGain( const nlohmann::json &paramsIn );
	void SetHistogramEQ( const nlohmann::json &paramsIn );
	void SetSharpenFilter( const nlohmann::json &paramsIn );
	void SetMinAutoExposureFramerate( const nlohmann::json &paramsIn );
	void SetTemporalFilterStrength( const nlohmann::json &paramsIn );
	void SetGainMultiplier( const nlohmann::json &paramsIn );
	void SetExposureMode( const nlohmann::json &paramsIn );
	void SetNoiseFilterMode( const nlohmann::json &paramsIn );
	void SetWhiteBalanceMode( const nlohmann::json &paramsIn );
	void SetWideDynamicRangeMode( const nlohmann::json &paramsIn );
	void SetZoneExposure( const nlohmann::json &paramsIn );
	void SetZoneWhiteBalance( const nlohmann::json &paramsIn );
	void SetPowerLineFrequency( const nlohmann::json &paramsIn );
	
	
	///////////////////////////////////////
	// Private channel API
	///////////////////////////////////////
	
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