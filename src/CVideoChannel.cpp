// Includes
#include <utility>
#include "CVideoChannel.h"

CVideoChannel::CVideoChannel( video_channel_t channelIn )
	: m_channel( channelIn )
{
	std::stringstream temp;
	temp << (int)m_channel;
	m_channelString = temp.str();
}

CVideoChannel::~CVideoChannel(){}

///////////////////////////////////////
// Private Channel API
///////////////////////////////////////
void CVideoChannel::Initialize()
{
	// Map command strings to API
	RegisterAPIFunctions();
	
	// Register video callback
	// if( mxuvc_video_register_cb( m_channel, CMuxer::VideoCallback, m_pCameraSocket ) )
	// {
	// 	throw std::runtime_error( "Failed to register video callback!" );
	// }
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
		throw std::runtime_error( "Failed to start video on channel: " + m_channelString );
	}
}

void CVideoChannel::StopVideo( const nlohmann::json &commandIn )
{
	if( mxuvc_video_stop( m_channel ) )
	{
		throw std::runtime_error( "Failed to stop video on channel: " + m_channelString );
	}
}

void CVideoChannel::SetMultipleSettings( const nlohmann::json &commandIn )
{
	// TODO
	// Loop through all of the setting objects in the command, calling their API callbacks with that json field being the input to the API callback
}

void CVideoChannel::SetFramerate( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_framerate( m_channel, commandIn[ "value" ].get<uint32_t>() ) )
		{
			throw;
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Failed to set framerate on channel: " + m_channelString );
	}
}

void CVideoChannel::SetBitrate( const nlohmann::json &commandIn )
{
	try
	{
		if( mxuvc_video_set_bitrate( m_channel, commandIn[ "value" ].get<uint32_t>() ) )
		{
			throw;
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Failed to set bitrate on channel: " + m_channelString );
	}
}

// H264
void CVideoChannel::ForceIFrame( const nlohmann::json &commandIn )
{
	if( mxuvc_video_force_iframe( m_channel ) )
	{
		throw std::runtime_error( "Failed to force I-Frame on channel: " + m_channelString );
	}
}

void CVideoChannel::SetGOPLength( const nlohmann::json &commandIn )
{
	if( mxuvc_video_set_goplen( m_channel, commandIn[ "value" ].get<uint32_t>() ) )
	{
		throw std::runtime_error( "Failed to set GOP length on channel: " + m_channelString );
	}
}

void CVideoChannel::SetGOPHierarchy( const nlohmann::json &commandIn )
{
	if( mxuvc_video_set_gop_hierarchy_level( m_channel, commandIn[ "value" ].get<uint32_t>() ) )
	{
		throw std::runtime_error( "Failed to set GOP hierarchy level on channel: " + m_channelString );
	}
}

void CVideoChannel::SetAVCProfile( const nlohmann::json &commandIn )
{
	try
	{
		std::string temp = commandIn[ "value" ];
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
			throw;
		}
		
		if( mxuvc_video_set_profile( m_channel, profile ) )
		{
			throw;
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Failed to set AVC Profile on channel: " + m_channelString );
	}
}

void CVideoChannel::SetAVCLevel( const nlohmann::json &commandIn )
{
	if( mxuvc_video_set_avc_level( m_channel, commandIn[ "value" ].get<uint32_t>() ) )
	{
		throw std::runtime_error( "Failed to set AVC Level on channel: " + m_channelString );
	}
}

void CVideoChannel::SetMaxNALSize( const nlohmann::json &commandIn )
{
	if( mxuvc_video_set_maxnal( m_channel, commandIn[ "value" ].get<uint32_t>() ) )
	{
		throw std::runtime_error( "Failed to set Max NAL size on channel: " + m_channelString );
	}
}

void CVideoChannel::EnableVUI( const nlohmann::json &commandIn )
{
	if( mxuvc_video_set_vui( m_channel, commandIn[ "value" ].get<bool>() ) )
	{
		throw std::runtime_error( "Failed to set VUI enabled/disabled on channel: " + m_channelString );
	}
}
void CVideoChannel::EnablePictTiming( const nlohmann::json &commandIn )
{
	if( mxuvc_video_set_pict_timing( m_channel, commandIn[ "value" ].get<bool>() ) )
	{
		throw std::runtime_error( "Failed to set Pict Timing enabled/disabled on channel: " + m_channelString );
	}
}
void CVideoChannel::SetMaxIFrameSize( const nlohmann::json &commandIn )
{
	if( mxuvc_video_set_max_framesize( m_channel, commandIn[ "value" ].get<uint32_t>() ) )
	{
		throw std::runtime_error( "Failed to set Max I-frame size on channel: " + m_channelString );
	}
}

// MJPEG
void CVideoChannel::SetCompressionQuality( const nlohmann::json &commandIn )
{
	if( mxuvc_video_set_compression_quality( m_channel, commandIn[ "value" ].get<uint32_t>() ) )
	{
		throw std::runtime_error( "Failed to set compression quality on channel: " + m_channelString );
	}
}

// Sensor
void CVideoChannel::EnableFlipVertical( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_flip_vertical( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Flip Vertical on channel: " + m_channelString );
	// }
}

void CVideoChannel::EnableFlipHorizontal( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_flip_horizontal( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Flip Horizontal on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetContrast( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_contrast( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set contrast on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetZoom( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_zoom( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Zoom on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetPan( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_pan( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Pan on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetTilt( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_tilt( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Tilt on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetPantilt( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_pantilt( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Pan/Tilt on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetBrightness( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_brightness( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Brightness on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetHue( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_hue( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Hue on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetGamma( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_gamma( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Gamma on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetSaturation( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_saturation( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Saturation on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetGain( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_gain( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Gain on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetSharpness( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_sharpness( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Sharpness on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetMaxAnalogGain( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_max_analog_gain( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Max Analog Gain on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetHistogramEQ( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_histogram_eq( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Histogram EQ on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetSharpenFilter( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_sharpen_filter( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Sharpen Filter on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetMinAutoExposureFramerate( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_min_exp_framerate( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Min AutoExposure Framerate on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetTemporalFilterStrength( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_tf_strength( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Temporal Filter Strength on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetGainMultiplier( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_gain_multiplier( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Gain Multiplier on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetExposureMode( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_exp( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Exposure Mode on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetNoiseFilterMode( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_nf( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Noise Filter Mode on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetWhiteBalanceMode( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_wb( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set White Balance Mode on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetWideDynamicRangeMode( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_wdr( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Wide Dynamic Range Mode on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetZoneExposure( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_zone_exp( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Zone Exposure on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetZoneWhiteBalance( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_zone_wb( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Zone White Balance on channel: " + m_channelString );
	// }
}

void CVideoChannel::SetPowerLineFrequency( const nlohmann::json &commandIn )
{
	// if( mxuvc_video_set_pwr_line_freq( m_channel ) )
	// {
	// 	throw std::runtime_error( "Failed to set Power Line Frequency on channel: " + m_channelString );
	// }
}