// Includes
#include <utility>
#include <unistd.h>
#include <chrono>
#include "CVideoChannel.h"

using namespace std;
using json = nlohmann::json;

CVideoChannel::CVideoChannel( CpperoMQ::Context *contextIn, video_channel_t channelIn )
	: m_channel( channelIn )
	, m_muxer( contextIn, channelIn, EVideoFormat::UNKNOWN )
{
	m_channelString = std::to_string( (int)m_channel );
	
	// Map command strings to API
	RegisterAPIFunctions();
	
	// Register video callback
	if( mxuvc_video_register_cb( m_channel, CVideoChannel::VideoCallback, this ) )
	{
		throw std::runtime_error( "Failed to register video callback!" );
	}
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
		cerr << "VideoChannel Error: " << e.what() << endl;
	}
}

///////////////////////////////////////
// Private Channel API
///////////////////////////////////////

// This gets called by the MXUVC library every time we have a NAL available
void CVideoChannel::VideoCallback( unsigned char *dataBufferOut, unsigned int bufferSizeIn, video_info_t infoIn, void *userDataIn )
{
	auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	cerr << "Input: " << bufferSizeIn << " bytes at: " << now << endl;
	
	CVideoChannel* channel = (CVideoChannel*) userDataIn;
	
	// Write the video data to the channel's muxer input buffer
	channel->m_muxer.m_inputBuffer.Write( dataBufferOut, bufferSizeIn );
	
	// Releases the buffer back to the MXUVC
	mxuvc_video_cb_buf_done( channel->m_channel, infoIn.buf_index );
}


void CVideoChannel::RegisterAPIFunctions()
{
	// Register callbacks in our handler map
	m_apiMap.insert( std::make_pair( std::string("video_start"), 			[this]( const nlohmann::json &commandIn ){ this->StartVideo( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("video_stop"), 			[this]( const nlohmann::json &commandIn ){ this->StopVideo( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("force_iframe"), 			[this]( const nlohmann::json &commandIn ){ this->ForceIFrame( commandIn ); } ) );
	
	m_apiMap.insert( std::make_pair( std::string("any_setting"), 			[this]( const nlohmann::json &commandIn ){ this->SetMultipleSettings( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("framerate"), 				[this]( const nlohmann::json &commandIn ){ this->SetFramerate( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("bitrate"), 				[this]( const nlohmann::json &commandIn ){ this->SetBitrate( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("goplen"), 				[this]( const nlohmann::json &commandIn ){ this->SetGOPLength( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("gop_hierarchy_level"),	[this]( const nlohmann::json &commandIn ){ this->SetGOPHierarchy( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("avc_profile"), 			[this]( const nlohmann::json &commandIn ){ this->SetAVCProfile( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("avc_level"), 				[this]( const nlohmann::json &commandIn ){ this->SetAVCLevel( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("maxnal"), 				[this]( const nlohmann::json &commandIn ){ this->SetMaxNALSize( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("vui"), 					[this]( const nlohmann::json &commandIn ){ this->SetVUI( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("pict_timing"), 			[this]( const nlohmann::json &commandIn ){ this->SetPictTiming( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("max_framesize"), 			[this]( const nlohmann::json &commandIn ){ this->SetMaxIFrameSize( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("compression_quality"),	[this]( const nlohmann::json &commandIn ){ this->SetCompressionQuality( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("flip_vertical"), 			[this]( const nlohmann::json &commandIn ){ this->SetFlipVertical( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("flip_horizontal"), 		[this]( const nlohmann::json &commandIn ){ this->SetFlipHorizontal( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("contrast"), 				[this]( const nlohmann::json &commandIn ){ this->SetContrast( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("zoom"), 					[this]( const nlohmann::json &commandIn ){ this->SetZoom( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("pan"), 					[this]( const nlohmann::json &commandIn ){ this->SetPan( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("tilt"), 					[this]( const nlohmann::json &commandIn ){ this->SetTilt( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("pantilt"), 				[this]( const nlohmann::json &commandIn ){ this->SetPantilt( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("brightness"), 			[this]( const nlohmann::json &commandIn ){ this->SetBrightness( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("hue"), 					[this]( const nlohmann::json &commandIn ){ this->SetHue( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("gamma"), 					[this]( const nlohmann::json &commandIn ){ this->SetGamma( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("saturation"), 			[this]( const nlohmann::json &commandIn ){ this->SetSaturation( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("gain"), 					[this]( const nlohmann::json &commandIn ){ this->SetGain( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("sharpness"), 				[this]( const nlohmann::json &commandIn ){ this->SetSharpness( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("max_analog_gain"), 		[this]( const nlohmann::json &commandIn ){ this->SetMaxAnalogGain( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("histogram_eq"), 			[this]( const nlohmann::json &commandIn ){ this->SetHistogramEQ( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("sharpen_filter"), 		[this]( const nlohmann::json &commandIn ){ this->SetSharpenFilter( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("min_exp_framerate"), 		[this]( const nlohmann::json &commandIn ){ this->SetMinAutoExposureFramerate( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("tf_strength"), 			[this]( const nlohmann::json &commandIn ){ this->SetTemporalFilterStrength( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("gain_multiplier"), 		[this]( const nlohmann::json &commandIn ){ this->SetGainMultiplier( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("exp"), 					[this]( const nlohmann::json &commandIn ){ this->SetExposureMode( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("nf"), 					[this]( const nlohmann::json &commandIn ){ this->SetNoiseFilterMode( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("wb"), 					[this]( const nlohmann::json &commandIn ){ this->SetWhiteBalanceMode( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("wdr"), 					[this]( const nlohmann::json &commandIn ){ this->SetWideDynamicRangeMode( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("zone_exp"), 				[this]( const nlohmann::json &commandIn ){ this->SetZoneExposure( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("zone_wb"), 				[this]( const nlohmann::json &commandIn ){ this->SetZoneWhiteBalance( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("pwr_line_freq"), 			[this]( const nlohmann::json &commandIn ){ this->SetPowerLineFrequency( commandIn ); } ) );
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
	mxuvc_video_set_vui( m_channel, 1 );
	mxuvc_video_set_pict_timing( m_channel, 1);
	
	if( mxuvc_video_start( m_channel ) )
	{
		throw std::runtime_error( "Command failed: StartVideo[" + m_channelString + "]: MXUVC failure" );
	}
}

void CVideoChannel::StopVideo( const nlohmann::json &commandIn )
{
	// Clear the video buffer
	m_muxer.m_inputBuffer.Clear();
	
	if( mxuvc_video_stop( m_channel ) )
	{
		throw std::runtime_error( "Command failed: StopVideo[" + m_channelString + "]: MXUVC failure" );
	}
}

void CVideoChannel::SetMultipleSettings( const nlohmann::json &commandIn )
{
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
			cerr << "Failed to set parameter in group: " << it.key();
		}
	}
}

// "value": 1-30
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

// "value": 100000-2000000
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

// "value": 0-maxint
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

// "value": 0-4
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

// "value": "baseline" | "main" | "high"
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

// "value": 10-52
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

// "value": 0-2000
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

// "value": true | false
void CVideoChannel::SetVUI( const nlohmann::json &commandIn )
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

// "value": true | false
void CVideoChannel::SetPictTiming( const nlohmann::json &commandIn )
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

// "value": 0-64000
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
// "value": 0-10000
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

// --------
// Sensor
// --------

// "value": true | false
void CVideoChannel::SetFlipVertical( const nlohmann::json &commandIn )
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

// "value": true | false
void CVideoChannel::SetFlipHorizontal( const nlohmann::json &commandIn )
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

// "value": 0-200
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

// "value": 0-100
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

// "value": -648000-648000 
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

// "value": -648000-648000 
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

// "value":
// {
// 	"pan": -648000-648000 
// 	"tilt": -648000-648000
// }
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

// "value": -255-255
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

// "value": -18000-18000
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

// "value": 100-300
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

// "value": 0-200
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

// "value": 1-100
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

// "value": 1-100
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

// "value": 0-15
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

// "value": true | false
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

// "value": 0-2
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

// "value": 0-30
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

// "value": 0-7
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

// "value": 0-256
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

// "value":
// {
// 	"sel": "auto" | "manual"
// 	"value": 0-255
// }
void CVideoChannel::SetExposureMode( const nlohmann::json &commandIn )
{
	try
	{
		exp_set_t sel;
		uint16_t value = 0;
		std::string mode = commandIn[ "sel" ].get<std::string>();
		
		if( mode == "auto" )
		{
			sel = EXP_AUTO;
		}
		else if( mode == "manual" )
		{
			sel = EXP_MANUAL;
			value = commandIn[ "value" ].get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid exposure mode" );
		}
		
		if( mxuvc_video_set_exp( m_channel, sel, value ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetExposureMode[" + m_channelString + "]: " + e.what() );
	}
		
}

// "value":
// {
// 	"sel": "auto" | "manual"
// 	"value": 0-100
// }
void CVideoChannel::SetNoiseFilterMode( const nlohmann::json &commandIn )
{
	try
	{
		noise_filter_mode_t sel;
		uint16_t value = 0;
		std::string mode = commandIn[ "sel" ].get<std::string>();
		
		if( mode == "auto" )
		{
			sel = NF_MODE_AUTO;
		}
		else if( mode == "manual" )
		{
			sel = NF_MODE_MANUAL;
			value = commandIn[ "value" ].get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid filter mode" );
		}
		
		if( mxuvc_video_set_nf( m_channel, sel, value ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetNoiseFilterMode[" + m_channelString + "]: " + e.what() );
	}
}

// "value":
// {
// 	"sel": "auto" | "manual"
// 	"value": 2800-6500
// }
void CVideoChannel::SetWhiteBalanceMode( const nlohmann::json &commandIn )
{
	try
	{
		white_balance_mode_t sel;
		uint16_t value = 0;
		std::string mode = commandIn[ "sel" ].get<std::string>();
		
		if( mode == "auto" )
		{
			sel = WB_MODE_AUTO;
		}
		else if( mode == "manual" )
		{
			sel = WB_MODE_MANUAL;
			value = commandIn[ "value" ].get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid wb mode" );
		}
		
		if( mxuvc_video_set_wb( m_channel, sel, value ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetWhiteBalanceMode[" + m_channelString + "]: " + e.what() );
	}
}


// "value":
// {
// 	"sel": "disabled" | "auto" | "manual"
// 	"value": 0-255
// }
void CVideoChannel::SetWideDynamicRangeMode( const nlohmann::json &commandIn )
{
	try
	{
		wdr_mode_t sel;
		uint8_t value = 0;
		std::string mode = commandIn[ "sel" ].get<std::string>();
		
		if( mode == "disable" )
		{
			sel = WDR_DISABLE;
		}
		else if( mode == "auto" )
		{
			sel = WDR_AUTO;
		}
		else if( mode == "manual" )
		{
			sel = WDR_MANUAL;
			value = commandIn[ "value" ].get<uint8_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid wb mode" );
		}
		
		if( mxuvc_video_set_wdr( m_channel, sel, value ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetWideDynamicRangeMode[" + m_channelString + "]: " + e.what() );
	}
}


// "value":
// {
// 	"sel": "enabled" | "disabled"
// 	"value": 0-62
// }
void CVideoChannel::SetZoneExposure( const nlohmann::json &commandIn )
{
	try
	{
		zone_exp_set_t sel;
		uint16_t value = 0;
		std::string mode = commandIn[ "sel" ].get<std::string>();
		
		if( mode == "disable" )
		{
			sel = ZONE_EXP_DISABLE;
		}
		else if( mode == "enable" )
		{
			sel = ZONE_EXP_ENABLE;
			value = commandIn[ "value" ].get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid zone exp mode" );
		}
		
		if( mxuvc_video_set_zone_exp( m_channel, sel, value ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetZoneExposure[" + m_channelString + "]: " + e.what() );
	}
}

// "value":
// {
// 	"sel": "enabled" | "disabled"
// 	"value": 0-63
// }
void CVideoChannel::SetZoneWhiteBalance( const nlohmann::json &commandIn )
{
	try
	{
		zone_wb_set_t sel;
		uint16_t value = 0;
		std::string mode = commandIn[ "sel" ].get<std::string>();
		
		if( mode == "disable" )
		{
			sel = ZONE_WB_DISABLE;
		}
		else if( mode == "enable" )
		{
			sel = ZONE_WB_ENABLE;
			value = commandIn[ "value" ].get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid zone wb mode" );
		}
		
		if( mxuvc_video_set_zone_wb( m_channel, sel, value ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetZoneWhiteBalance[" + m_channelString + "]: " + e.what() );
	}
}

// "value": "disabled" | "50HZ" | "60HZ"
void CVideoChannel::SetPowerLineFrequency( const nlohmann::json &commandIn )
{
	try
	{
		pwr_line_freq_mode_t pwrMode;
		std::string mode = commandIn.get<std::string>();
		
		if( mode == "disabled" )
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

///////////////
// Get API
///////////////

// General
void CVideoChannel::GetAllSettings()
{
	GetSensorSettings();
	GetVideoSettings();
	
	// Publish updates
}

void CVideoChannel::GetSensorSettings()
{
	
}

void CVideoChannel::GetVideoSettings()
{
	
}


void CVideoChannel::GetChannelInfo()
{
	video_channel_info_t info;
	
	if( mxuvc_video_get_channel_info( m_channel, &info ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		// Format
		switch( info.format )
		{
			case VID_FORMAT_H264_RAW:
				m_settings[ "format" ] = "h264";
				break;
			case VID_FORMAT_MJPEG_RAW:
				m_settings[ "format" ] = "mjpeg";
				break;
			default:
				m_settings[ "format" ] = "unknown";
				break;
		}
		
		m_settings[ "width" ] 				= info.width;
		m_settings[ "height" ] 				= info.height;
		m_settings[ "framerate" ] 			= info.framerate;
		m_settings[ "goplen" ] 				= info.goplen;
		m_settings[ "bitrate" ] 			= info.bitrate;
		m_settings[ "compression_quality" ] 	= info.compression_quality;
		
		// Profile
		switch( info.profile )
		{
			case PROFILE_BASELINE:
				m_settings[ "profile" ] = "baseline";
				break;
			case PROFILE_MAIN:
				m_settings[ "profile" ] = "main";
				break;
			case PROFILE_HIGH:
				m_settings[ "profile" ] = "high";
				break;
			default:
				m_settings[ "profile" ] = "unknown";
				break;
		}
	}
}

void CVideoChannel::GetFramerate()
{
	uint32_t value;
	
	if( mxuvc_video_get_framerate( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "framerate" ] = value;
	}
}

void CVideoChannel::GetBitrate()
{
	uint32_t value;
	
	if( mxuvc_video_get_bitrate( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "bitrate" ] = value;
	}
}


// H264
void CVideoChannel::GetGOPLength()
{
	uint32_t value;
	
	if( mxuvc_video_get_goplen( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "goplen" ] = value;
	}
}

void CVideoChannel::GetGOPHierarchy()
{
	uint32_t value;
	
	if( mxuvc_video_get_gop_hierarchy_level( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "gop_hierarchy_level" ] = value;
	}
}

void CVideoChannel::GetAVCProfile()
{
	video_profile_t profile;
	if( mxuvc_video_get_profile( m_channel, &profile ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{	
		switch( profile )
		{
			case PROFILE_BASELINE:
				m_settings[ "profile" ] = "baseline";
				break;
			case PROFILE_MAIN:
				m_settings[ "profile" ] = "main";
				break;
			case PROFILE_HIGH:
				m_settings[ "profile" ] = "high";
				break;
			default:
				m_settings[ "profile" ] = "unknown";
				break;
		}
	}
}

void CVideoChannel::GetAVCLevel()
{
	uint32_t value;
	
	if( mxuvc_video_get_avc_level( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "avc_level" ] = value;
	}
}

void CVideoChannel::GetMaxNALSize()
{
	uint32_t value;
	
	if( mxuvc_video_get_maxnal( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "maxnal" ] = value;
	}
}

void CVideoChannel::GetVUI()
{
	uint32_t value;
	
	if( mxuvc_video_get_vui( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "vui" ] = (bool)value;
	}
}

void CVideoChannel::GetPictTiming()
{
	uint32_t value;
	
	if( mxuvc_video_get_pict_timing( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "pict_timing" ] = (bool)value;
	}
}

void CVideoChannel::GetMaxIFrameSize()
{
	uint32_t value;
	
	if( mxuvc_video_get_max_framesize( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "max_framesize" ] = value;
	}
}


// MJPEG
void CVideoChannel::GetCompressionQuality()
{
	uint32_t value;
	
	if( mxuvc_video_get_compression_quality( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "compression_quality" ] = value;
	}
}

// Sensor
void CVideoChannel::GetFlipVertical()
{
	video_flip_t flip;
	if( mxuvc_video_get_flip_vertical( m_channel, &flip ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{	
		m_settings[ "flip_vertical" ] = ( flip == FLIP_ON ? true : false );
	}
}

void CVideoChannel::GetFlipHorizontal()
{
	video_flip_t flip;
	if( mxuvc_video_get_flip_horizontal( m_channel, &flip ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{	
		m_settings[ "flip_horizontal" ] = ( flip == FLIP_ON ? true : false );
	}
}

void CVideoChannel::GetContrast()
{
	uint16_t value;
	
	if( mxuvc_video_get_contrast( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "contrast" ] = value;
	}
}

void CVideoChannel::GetZoom()
{
	uint16_t value;
	
	if( mxuvc_video_get_zoom( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "zoom" ] = value;
	}
}

void CVideoChannel::GetPan()
{
	int32_t value;
	
	if( mxuvc_video_get_pan( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "pan" ] = value;
	}
}

void CVideoChannel::GetTilt()
{
	int32_t value;
	
	if( mxuvc_video_get_tilt( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "tilt" ] = value;
	}
}

void CVideoChannel::GetPantilt()
{
	int32_t pan;
	int32_t tilt;
	
	if( mxuvc_video_get_pantilt( m_channel, &pan, &tilt ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "pan" ] = pan;
		m_settings[ "tilt" ] = tilt;
	}
}

void CVideoChannel::GetBrightness()
{
	int16_t value;
	
	if( mxuvc_video_get_brightness( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "brightness" ] = value;
	}
}

void CVideoChannel::GetHue()
{
	int16_t value;
	
	if( mxuvc_video_get_hue( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "hue" ] = value;
	}
}

void CVideoChannel::GetGamma()
{
	uint16_t value;
	
	if( mxuvc_video_get_gamma( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "gamma" ] = value;
	}
}

void CVideoChannel::GetSaturation()
{
	uint16_t value;
	
	if( mxuvc_video_get_saturation( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "saturation" ] = value;
	}
}

void CVideoChannel::GetGain()
{
	uint16_t value;
	
	if( mxuvc_video_get_gain( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "gain" ] = value;
	}
}

void CVideoChannel::GetSharpness()
{
	uint16_t value;
	
	if( mxuvc_video_get_sharpness( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "sharpness" ] = value;
	}
}

void CVideoChannel::GetMaxAnalogGain()
{
	uint32_t value;
	
	if( mxuvc_video_get_max_analog_gain( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "max_analog_gain" ] = value;
	}
}

void CVideoChannel::GetHistogramEQ()
{
	histo_eq_t histo;
	if( mxuvc_video_get_histogram_eq( m_channel, &histo ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{	
		m_settings[ "histogram_eq" ] = ( histo == HISTO_EQ_ON ? true : false );
	}
}

void CVideoChannel::GetSharpenFilter()
{
	uint32_t value;
	
	if( mxuvc_video_get_sharpen_filter( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "sharpen_filter" ] = value;
	}
}

void CVideoChannel::GetMinAutoExposureFramerate()
{
	uint32_t value;
	
	if( mxuvc_video_get_min_exp_framerate( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "min_exp_framerate" ] = value;
	}
}

void CVideoChannel::GetTemporalFilterStrength()
{
	uint32_t value;
	
	if( mxuvc_video_get_tf_strength( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "tf_strength" ] = value;
	}
}

void CVideoChannel::GetGainMultiplier()
{
	uint32_t value;
	
	if( mxuvc_video_get_gain_multiplier( m_channel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{
		m_settings[ "gain_multiplier" ] = value;
	}
}

void CVideoChannel::GetExposureMode()
{
	exp_set_t sel;
	uint16_t value;
	if( mxuvc_video_get_exp( m_channel, &sel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{	
		switch( sel )
		{
			case EXP_AUTO:
				m_settings[ "exp" ] = { { "sel", "auto" }, { "value", value } };
				break;
			case EXP_MANUAL:
				m_settings[ "exp" ] = { { "sel", "manual" }, { "value", value } };
				break;
			default:
				m_settings[ "exp" ] = "unknown";
				break;
		}
	}
}

void CVideoChannel::GetNoiseFilterMode()
{
	noise_filter_mode_t sel;
	uint16_t value;
	if( mxuvc_video_get_nf( m_channel, &sel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{	
		switch( sel )
		{
			case NF_MODE_AUTO:
				m_settings[ "nf" ] = { { "sel", "auto" }, { "value", value } };
				break;
			case NF_MODE_MANUAL:
				m_settings[ "nf" ] = { { "sel", "manual" }, { "value", value } };
				break;
			default:
				m_settings[ "nf" ] = "unknown";
				break;
		}
	}
}

void CVideoChannel::GetWhiteBalanceMode()
{
	white_balance_mode_t sel;
	uint16_t value;
	if( mxuvc_video_get_wb( m_channel, &sel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{	
		switch( sel )
		{
			case WB_MODE_AUTO:
				m_settings[ "wb" ] = { { "sel", "auto" }, { "value", value } };
				break;
			case WB_MODE_MANUAL:
				m_settings[ "wb" ] = { { "sel", "manual" }, { "value", value } };
				break;
			default:
				m_settings[ "wb" ] = "unknown";
				break;
		}
	}
}

void CVideoChannel::GetWideDynamicRangeMode()
{
	wdr_mode_t sel;
	uint8_t value;
	if( mxuvc_video_get_wdr( m_channel, &sel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{	
		switch( sel )
		{
			case WDR_AUTO:
				m_settings[ "wdr" ] = { { "sel", "auto" }, { "value", value } };
				break;
			case WDR_MANUAL:
				m_settings[ "wdr" ] = { { "sel", "manual" }, { "value", value } };
				break;
			case WDR_DISABLE:
				m_settings[ "wdr" ] = { { "sel", "disabled" }, { "value", value } };
				break;
			default:
				m_settings[ "wdr" ] = "unknown";
				break;
		}
	}
}

void CVideoChannel::GetZoneExposure()
{
	zone_exp_set_t sel;
	uint16_t value;
	if( mxuvc_video_get_zone_exp( m_channel, &sel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{	
		switch( sel )
		{
			case ZONE_EXP_DISABLE:
				m_settings[ "zone_exp" ] = { { "sel", "disabled" }, { "value", value } };
				break;
			case ZONE_EXP_ENABLE:
				m_settings[ "zone_exp" ] = { { "sel", "enabled" }, { "value", value } };
				break;
			default:
				m_settings[ "zone_exp" ] = "unknown";
				break;
		}
	}
}

void CVideoChannel::GetZoneWhiteBalance()
{
	zone_wb_set_t sel;
	uint16_t value;
	if( mxuvc_video_get_zone_wb( m_channel, &sel, &value ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{	
		switch( sel )
		{
			case ZONE_WB_DISABLE:
				m_settings[ "zone_wb" ] = { { "sel", "disabled" }, { "value", value } };
				break;
			case ZONE_WB_ENABLE:
				m_settings[ "zone_wb" ] = { { "sel", "enabled" }, { "value", value } };
				break;
			default:
				m_settings[ "zone_wb" ] = "unknown";
				break;
		}
	}
}

void CVideoChannel::GetPowerLineFrequency()
{
	pwr_line_freq_mode_t mode;
	if( mxuvc_video_get_pwr_line_freq( m_channel, &mode ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
	else
	{	
		switch( mode )
		{
			case PWR_LINE_FREQ_MODE_DISABLE:
				m_settings[ "pwr_line_freq" ] = "disabled";
				break;
			case PWR_LINE_FREQ_MODE_50HZ:
				m_settings[ "pwr_line_freq" ] = "50HZ";
				break;
			case PWR_LINE_FREQ_MODE_60HZ:
				m_settings[ "pwr_line_freq" ] = "60HZ";
				break;
			default:
				m_settings[ "pwr_line_freq" ] = "unknown";
				break;
		}
	}
}
