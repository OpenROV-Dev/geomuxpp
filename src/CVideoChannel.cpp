// Includes
#include <utility>
#include <unistd.h>
#include <chrono>
#include "CVideoChannel.h"
#include "easylogging.hpp"

using namespace std;
using json = nlohmann::json;

CVideoChannel::CVideoChannel( video_channel_t channelIn )
	: m_channel( channelIn )
	
{
	m_channelString = std::to_string( (int)m_channel );
	
	Initialize();
}

CVideoChannel::~CVideoChannel(){}


void CVideoChannel::HandleMessage( const nlohmann::json &commandIn )
{
	try
	{
		// Call specified channel command with appropriate API function using passed in value
		m_apiMap.at( commandIn.at( "chCmd" ).get<std::string>() )( commandIn.at( "value" ) );
	}
	catch( const std::exception &e )
	{
		LOG( ERROR ) << "VideoChannel Error: " << e.what();
	}
}

///////////////////////////////////////
// Private Channel API
///////////////////////////////////////
void CVideoChannel::Initialize()
{
	// Map command strings to API
	RegisterAPIFunctions();
	
	// Register video callback
	if( mxuvc_video_register_cb( m_channel, CVideoChannel::VideoCallback, this ) )
	{
		throw std::runtime_error( "Failed to register video callback!" );
	}
}

// This gets called by the MXUVC library every time we have a NAL available
void CVideoChannel::VideoCallback( unsigned char *dataBufferOut, unsigned int bufferSizeIn, video_info_t infoIn, void *userDataIn )
{
	CVideoChannel* channel = (CVideoChannel*) userDataIn;
	
	// LOG(INFO) << "Got video data: " << channel->m_channelString << " - " << bufferSizeIn << " bytes";
	auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	cout << "Update at: " << now << endl;
	
	// Releases the buffer back to the MXUVC
	mxuvc_video_cb_buf_done( channel->m_channel, infoIn.buf_index );
}

void CVideoChannel::Cleanup()
{
	
}

void CVideoChannel::RegisterAPIFunctions()
{
	// Register callbacks in our handler map
	m_apiMap.insert( std::make_pair( std::string("StartVideo"), 					[this]( const nlohmann::json &commandIn ){ this->StartVideo( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("StopVideo"), 						[this]( const nlohmann::json &commandIn ){ this->StopVideo( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetMultipleSettings"), 			[this]( const nlohmann::json &commandIn ){ this->SetMultipleSettings( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetFramerate"), 					[this]( const nlohmann::json &commandIn ){ this->SetFramerate( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetBitrate"), 					[this]( const nlohmann::json &commandIn ){ this->SetBitrate( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("ForceIFrame"), 					[this]( const nlohmann::json &commandIn ){ this->ForceIFrame( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetGOPLength"), 					[this]( const nlohmann::json &commandIn ){ this->SetGOPLength( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetGOPHierarchy"), 				[this]( const nlohmann::json &commandIn ){ this->SetGOPHierarchy( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetAVCProfile"), 					[this]( const nlohmann::json &commandIn ){ this->SetAVCProfile( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetAVCLevel"), 					[this]( const nlohmann::json &commandIn ){ this->SetAVCLevel( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetMaxNALSize"), 					[this]( const nlohmann::json &commandIn ){ this->SetMaxNALSize( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("EnableVUI"), 						[this]( const nlohmann::json &commandIn ){ this->EnableVUI( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("EnablePictTiming"), 				[this]( const nlohmann::json &commandIn ){ this->EnablePictTiming( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetMaxIFrameSize"), 				[this]( const nlohmann::json &commandIn ){ this->SetMaxIFrameSize( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetCompressionQuality"), 			[this]( const nlohmann::json &commandIn ){ this->SetCompressionQuality( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("EnableFlipVertical"), 			[this]( const nlohmann::json &commandIn ){ this->EnableFlipVertical( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("EnableFlipHorizontal"), 			[this]( const nlohmann::json &commandIn ){ this->EnableFlipHorizontal( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetContrast"), 					[this]( const nlohmann::json &commandIn ){ this->SetContrast( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetZoom"), 						[this]( const nlohmann::json &commandIn ){ this->SetZoom( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetPan"), 						[this]( const nlohmann::json &commandIn ){ this->SetPan( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetTilt"), 						[this]( const nlohmann::json &commandIn ){ this->SetTilt( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetPantilt"), 					[this]( const nlohmann::json &commandIn ){ this->SetPantilt( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetBrightness"), 					[this]( const nlohmann::json &commandIn ){ this->SetBrightness( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetHue"), 						[this]( const nlohmann::json &commandIn ){ this->SetHue( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetGamma"), 						[this]( const nlohmann::json &commandIn ){ this->SetGamma( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetSaturation"), 					[this]( const nlohmann::json &commandIn ){ this->SetSaturation( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetGain"), 						[this]( const nlohmann::json &commandIn ){ this->SetGain( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetSharpness"), 					[this]( const nlohmann::json &commandIn ){ this->SetSharpness( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetMaxAnalogGain"), 				[this]( const nlohmann::json &commandIn ){ this->SetMaxAnalogGain( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetHistogramEQ"), 				[this]( const nlohmann::json &commandIn ){ this->SetHistogramEQ( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetSharpenFilter"), 				[this]( const nlohmann::json &commandIn ){ this->SetSharpenFilter( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetMinAutoExposureFramerate"), 	[this]( const nlohmann::json &commandIn ){ this->SetMinAutoExposureFramerate( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetTemporalFilterStrength"), 		[this]( const nlohmann::json &commandIn ){ this->SetTemporalFilterStrength( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetGainMultiplier"), 				[this]( const nlohmann::json &commandIn ){ this->SetGainMultiplier( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetExposureMode"), 				[this]( const nlohmann::json &commandIn ){ this->SetExposureMode( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetNoiseFilterMode"), 			[this]( const nlohmann::json &commandIn ){ this->SetNoiseFilterMode( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetWhiteBalanceMode"), 			[this]( const nlohmann::json &commandIn ){ this->SetWhiteBalanceMode( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetWideDynamicRangeMode"), 		[this]( const nlohmann::json &commandIn ){ this->SetWideDynamicRangeMode( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetZoneExposure"), 				[this]( const nlohmann::json &commandIn ){ this->SetZoneExposure( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetZoneWhiteBalance"), 			[this]( const nlohmann::json &commandIn ){ this->SetZoneWhiteBalance( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetPowerLineFrequency"), 			[this]( const nlohmann::json &commandIn ){ this->SetPowerLineFrequency( commandIn ); } ) );
}

void CVideoChannel::GetVideoSettings()
{
	
}

bool CVideoChannel::IsAlive()
{ 
	return false;
}


///////////////////////////////////////
// Public channel API
///////////////////////////////////////

// General
void CVideoChannel::StartVideo( const nlohmann::json &commandIn )
{
	if( mxuvc_video_start( m_channel ) )
	{
		throw std::runtime_error( "Command failed: StartVideo[" + m_channelString + "]: MXUVC failure" );
	}
}

void CVideoChannel::StopVideo( const nlohmann::json &commandIn )
{
	if( mxuvc_video_stop( m_channel ) )
	{
		throw std::runtime_error( "Command failed: StopVideo[" + m_channelString + "]: MXUVC failure" );
	}
}

void CVideoChannel::SetMultipleSettings( const nlohmann::json &commandIn )
{
	// TODO
	// Loop through all of the setting objects in the command, calling their API callbacks with that json field being the input to the API callback
	
	// cmd : chCmd
	// ch: x
	// chCmd: SetMultipleSettings
	// value: { object with all settings } == commandIn
	// 	key (setting name): { value ( setting parameters ) }
	
	for (json::const_iterator it = commandIn.begin(); it != commandIn.end(); ++it) 
	{
		try
		{
			// Call specified channel command with appropriate API function using passed in value
			m_apiMap.at( it.key() )( it.value() );
		}
		catch( const std::exception &e )
		{
			LOG( ERROR ) << "Failed to set parameter in group: " << it.key();
		}
	}
	
	LOG( INFO ) << "DONE";
}

void CVideoChannel::SetFramerate( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_framerate( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetFramerate[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetBitrate( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_bitrate( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetBitrate[" + m_channelString + "]: " + e.what() );
	}
}

// H264
void CVideoChannel::ForceIFrame( const nlohmann::json &commandIn )
{
	if( mxuvc_video_force_iframe( m_channel ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
}

void CVideoChannel::SetGOPLength( const nlohmann::json &commandIn )
{
	try
	{
		
		if( mxuvc_video_set_goplen( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetGOPLength[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetGOPHierarchy( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_gop_hierarchy_level( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetGOPHierarchy[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetAVCProfile( const nlohmann::json &commandIn )
{
	try
	{
		std::string temp = commandIn.get<std::string>();
		video_profile_t profile;
		
		if( temp == "baseline" )
		{
			profile = PROFILE_BASELINE;
		}
		else if( temp == "main" )
		{
			profile = PROFILE_MAIN;
		}
		else if( temp == "high" )
		{
			profile = PROFILE_HIGH;
		}
		else
		{
			throw std::runtime_error( "Invalid AVC Profile value" );
		}
		
		if( mxuvc_video_set_profile( m_channel, profile ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetAVCProfile[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetAVCLevel( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_avc_level( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetAVCLevel[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetMaxNALSize( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_maxnal( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetMaxNALSize[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::EnableVUI( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_vui( m_channel, commandIn.get<bool>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: EnableVUI[" + m_channelString + "]: " + e.what() );
	}
}
void CVideoChannel::EnablePictTiming( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_pict_timing( m_channel, commandIn.get<bool>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: EnablePictTiming[" + m_channelString + "]: " + e.what() );
	}
}
void CVideoChannel::SetMaxIFrameSize( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_max_framesize( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetMaxIFrameSize[" + m_channelString + "]: " + e.what() );
	}
}

// MJPEG
void CVideoChannel::SetCompressionQuality( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_compression_quality( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetCompressionQuality[" + m_channelString + "]: " + e.what() );
	}
}

// Sensor
void CVideoChannel::EnableFlipVertical( const nlohmann::json &commandIn )
{
	try
	{
		video_flip_t flip = ( commandIn.get<bool>() ? FLIP_ON : FLIP_OFF );
			
		if( mxuvc_video_set_flip_vertical( m_channel, flip ) )
		{
			throw std::runtime_error( "Failed to set Flip Vertical on channel: " + m_channelString );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: EnableFlipVertical[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::EnableFlipHorizontal( const nlohmann::json &commandIn )
{
	try
	{
		video_flip_t flip = ( commandIn.get<bool>() ? FLIP_ON : FLIP_OFF );
			
		if( mxuvc_video_set_flip_horizontal( m_channel, flip ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: EnableFlipHorizontal[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetContrast( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_contrast( m_channel, commandIn.get<uint16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetContrast[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetZoom( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_zoom( m_channel, commandIn.get<uint16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetZoom[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetPan( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_pan( m_channel, commandIn.get<int32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetPan[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetTilt( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_tilt( m_channel, commandIn.get<int32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetTilt[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetPantilt( const nlohmann::json &commandIn )
{
	try
	{
		int32_t pan = commandIn[ "pan" ].get<int32_t>();
		int32_t tilt = commandIn[ "tilt" ].get<int32_t>();
		
		if( mxuvc_video_set_pantilt( m_channel, pan, tilt ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetPantilt[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetBrightness( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_brightness( m_channel, commandIn.get<int16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetBrightness[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetHue( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_hue( m_channel, commandIn.get<int16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetHue[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetGamma( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_gamma( m_channel, commandIn.get<uint16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetGamma[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetSaturation( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_saturation( m_channel, commandIn.get<uint16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetSaturation[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetGain( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_gain( m_channel, commandIn.get<uint16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetGain[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetSharpness( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_sharpness( m_channel, commandIn.get<uint16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetSharpness[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetMaxAnalogGain( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_max_analog_gain( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetMaxAnalogGain[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetHistogramEQ( const nlohmann::json &commandIn )
{
	try
	{
		histo_eq_t histEq = ( commandIn.get<bool>() ? HISTO_EQ_ON : HISTO_EQ_OFF );
		
		if( mxuvc_video_set_histogram_eq( m_channel, histEq ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetHistogramEQ[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetSharpenFilter( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_sharpen_filter( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetSharpenFilter[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetMinAutoExposureFramerate( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_min_exp_framerate( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetMinAutoExposureFramerate[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetTemporalFilterStrength( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_tf_strength( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetTemporalFilterStrength[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetGainMultiplier( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_gain_multiplier( m_channel, commandIn.get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetGainMultiplier[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetExposureMode( const nlohmann::json &commandIn )
{
	try
	{
		exp_set_t exposureMode;
		uint16_t exposureTime = 0;
		std::string mode = commandIn[ "mode" ].get<std::string>();
		
		if( mode == "auto" )
		{
			exposureMode = EXP_AUTO;
		}
		else if( mode == "manual" )
		{
			exposureMode = EXP_MANUAL;
			exposureTime = commandIn[ "exposureTime" ].get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid exposure mode" );
		}
		
		if( mxuvc_video_set_exp( m_channel, exposureMode, exposureTime ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetExposureMode[" + m_channelString + "]: " + e.what() );
	}
		
}

void CVideoChannel::SetNoiseFilterMode( const nlohmann::json &commandIn )
{
	try
	{
		noise_filter_mode_t filterMode;
		uint16_t filterStrength = 0;
		std::string mode = commandIn[ "mode" ].get<std::string>();
		
		if( mode == "auto" )
		{
			filterMode = NF_MODE_AUTO;
		}
		else if( mode == "manual" )
		{
			filterMode = NF_MODE_MANUAL;
			filterStrength = commandIn[ "filterStrength" ].get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid filter mode" );
		}
		
		if( mxuvc_video_set_nf( m_channel, filterMode, filterStrength ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetNoiseFilterMode[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetWhiteBalanceMode( const nlohmann::json &commandIn )
{
	try
	{
		white_balance_mode_t wbMode;
		uint16_t temperature = 0;
		std::string mode = commandIn[ "mode" ].get<std::string>();
		
		if( mode == "auto" )
		{
			wbMode = WB_MODE_AUTO;
		}
		else if( mode == "manual" )
		{
			wbMode = WB_MODE_MANUAL;
			temperature = commandIn[ "temperature" ].get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid wb mode" );
		}
		
		if( mxuvc_video_set_wb( m_channel, wbMode, temperature ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetWhiteBalanceMode[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetWideDynamicRangeMode( const nlohmann::json &commandIn )
{
	try
	{
		wdr_mode_t wdrMode;
		uint8_t intensity = 0;
		std::string mode = commandIn[ "mode" ].get<std::string>();
		
		if( mode == "disable" )
		{
			wdrMode = WDR_DISABLE;
		}
		else if( mode == "auto" )
		{
			wdrMode = WDR_AUTO;
		}
		else if( mode == "manual" )
		{
			wdrMode = WDR_MANUAL;
			intensity = commandIn[ "intensity" ].get<uint8_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid wb mode" );
		}
		
		if( mxuvc_video_set_wdr( m_channel, wdrMode, intensity ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetWideDynamicRangeMode[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetZoneExposure( const nlohmann::json &commandIn )
{
	try
	{
		zone_exp_set_t enableZoneExp;
		uint16_t expValue = 0;
		std::string mode = commandIn[ "mode" ].get<std::string>();
		
		if( mode == "disable" )
		{
			enableZoneExp = ZONE_EXP_DISABLE;
		}
		else if( mode == "enable" )
		{
			enableZoneExp = ZONE_EXP_ENABLE;
			expValue = commandIn[ "expValue" ].get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid zone exp mode" );
		}
		
		if( mxuvc_video_set_zone_exp( m_channel, enableZoneExp, expValue ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetZoneExposure[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetZoneWhiteBalance( const nlohmann::json &commandIn )
{
	try
	{
		zone_wb_set_t enableZoneWb;
		uint16_t wbValue = 0;
		std::string mode = commandIn[ "mode" ].get<std::string>();
		
		if( mode == "disable" )
		{
			enableZoneWb = ZONE_WB_DISABLE;
		}
		else if( mode == "enable" )
		{
			enableZoneWb = ZONE_WB_ENABLE;
			wbValue = commandIn[ "wbValue" ].get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid zone wb mode" );
		}
		
		if( mxuvc_video_set_zone_wb( m_channel, enableZoneWb, wbValue ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetZoneWhiteBalance[" + m_channelString + "]: " + e.what() );
	}
}

void CVideoChannel::SetPowerLineFrequency( const nlohmann::json &commandIn )
{
	try
	{
		pwr_line_freq_mode_t pwrMode;
		std::string mode = commandIn[ "mode" ].get<std::string>();
		
		if( mode == "disable" )
		{
			pwrMode = PWR_LINE_FREQ_MODE_DISABLE;
		}
		else if( mode == "50HZ" )
		{
			pwrMode = PWR_LINE_FREQ_MODE_50HZ;
		}
		else if( mode == "60HZ" )
		{
			pwrMode = PWR_LINE_FREQ_MODE_60HZ;
		}
		else
		{
			throw std::runtime_error( "Invalid pwr freq mode" );
		}
		
		
		if( mxuvc_video_set_pwr_line_freq( m_channel, pwrMode ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetPowerLineFrequency[" + m_channelString + "]: " + e.what() );
	}
}