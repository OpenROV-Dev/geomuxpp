// Includes
#include <utility>
#include <unistd.h>
#include <chrono>
#include <fstream>
#include <algorithm>

#include "CVideoChannel.h"
#include "GC6500_API.h"

using namespace std;
using json = nlohmann::json;

CVideoChannel::CVideoChannel( const std::string &cameraOffsetIn, video_channel_t channelIn, CpperoMQ::Context *contextIn )
	: m_channel( channelIn )
	, m_cameraString( cameraOffsetIn )
	, m_channelString( std::to_string( (int)m_channel ) )
	, m_eventEndpoint( std::string( "ipc:///tmp/geomux_event" + m_cameraString + "_" + m_channelString + ".ipc" ) )
	, m_videoEndpoint( std::string( "ipc:///tmp/geomux_video" + m_cameraString + "_" + m_channelString + ".ipc" ) )
	, m_eventEmitter( contextIn, m_eventEndpoint )
	, m_muxer( contextIn, m_videoEndpoint, EVideoFormat::UNKNOWN )
{
	cout << "Registering API" << endl;
	// Map command strings to API
	RegisterAPIFunctions();
	
	cout << "Getting all settings" << endl;
	
	// Fetch all setting info from the camera. Some is critical (format), most is not.
	GetAllSettings();
	
	cout << "Loading API" << endl;
	
	// Load API
	LoadAPI();
	
	cout << "Registering camera cb" << endl;
	
	// TODO:
	// Dynamically create the type of muxer based on the detected video format (h264 vs mjpeg)
	
	// Register video callback. Must be done before a number of actions can be performed on the channel
	if( mxuvc_video_register_cb( m_channel, CVideoChannel::VideoCallback, this ) )
	{
		throw std::runtime_error( "Failed to register video callback!" );
	}
}

CVideoChannel::~CVideoChannel()
{
}

void CVideoChannel::Initialize()
{
	try
	{
		// Publish API for channel
		ReportAPI( json() );
	
		// Publish settings for channel
		ReportSettings( json() );
	}
	catch( std::exception &e )
	{
		cout << "Error initializing channel: " << e.what() << endl;
	}
}


void CVideoChannel::HandleMessage( const nlohmann::json &commandIn )
{
	try
	{
		// commandIn Format:
		// {
		// 		"cmd": 		"chCmd"
		// 		"ch": 		<channelNum>
		// 		"chCmd": 	<commandName>
		// 		"params": 	<commandParams>
		// }

		// Validate Command
		ValidateCommand( commandIn );

		const std::string command( commandIn.at( "chCmd" ).get<std::string>() );
		const json params( commandIn.at( "params" ) );
		
		// Call specified channel command with specified parameters
		m_publicApiMap.at( command )( params );
	}
	catch( const std::exception &e )
	{
		cerr << "VideoChannel Error: " << std::string( e.what() ) << endl;
	}
}

///////////////////////////////////////
// Private Channel API
///////////////////////////////////////

// This gets called by the MXUVC library every time we have a NAL available
void CVideoChannel::VideoCallback( unsigned char *dataBufferOut, unsigned int bufferSizeIn, video_info_t infoIn, void *userDataIn )
{
	// auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	// cout << "Input: " << bufferSizeIn << " bytes at: " << now << endl;
	
	CVideoChannel* channel = (CVideoChannel*) userDataIn;
	
	// Write the video data to the channel's muxer input buffer
	channel->m_muxer.m_inputBuffer.Write( dataBufferOut, bufferSizeIn );
	
	// Releases the buffer back to the MXUVC
	mxuvc_video_cb_buf_done( channel->m_channel, infoIn.buf_index );
}

void CVideoChannel::LoadAPI()
{
	try
	{
		// Check the API version
		if( GC6500_VERSION != gc6500::api.at( "version" ).get<std::string>() )
		{
			throw std::runtime_error( "GC6500 version used in this build does not match API schema!" );
		}
		
		m_api[ "version" ] = gc6500::api.at( "version" ).get<std::string>();
		
		for( json::iterator it = gc6500::api.at( "publicAPI" ).begin(); it != gc6500::api.at( "publicAPI" ).end(); ++it ) 
		{
			cout << "Checking support for: " << it.key() << " - ";
			
			// Check to see if it is supported for this channel's format
			if( IsCommandSupported( it.key() ) == true )
			{
				// Copy the API to the channel's API
				m_api[ "publicAPI" ][ it.key() ] = it.value();
				
				cout << "Success!" << endl;
			}
			else
			{
				cout << "Fail!" << endl;
			}
		}
		
		// Iterate settings API section and grab relevant API descriptors		
		for( json::iterator it = gc6500::api.at( "settingsAPI" ).begin(); it != gc6500::api.at( "settingsAPI" ).end(); ++it ) 
		{
			cout << "Checking support for: " << it.key() << " - ";
			
			// Check to see if it is supported for this channel's format
			if( IsSettingSupported( it.key() ) == true )
			{
				// Copy the API to the channel's API
				m_api[ "settingsAPI" ][ it.key() ] = it.value();
				
				cout << "Success!" << endl;
			}
			else
			{
				cout << "Fail!" << endl;
			}
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Failed to load API. Error: " + std::string( e.what() ) );
	}
}

void CVideoChannel::RegisterAPIFunctions()
{
	// Register callbacks in our handler map
	
	// Public API
	m_publicApiMap.insert( std::make_pair( std::string("video_start"), 				[this]( const nlohmann::json &paramsIn ){ this->StartVideo( paramsIn ); } ) );
	m_publicApiMap.insert( std::make_pair( std::string("video_stop"), 				[this]( const nlohmann::json &paramsIn ){ this->StopVideo( paramsIn ); } ) );
	m_publicApiMap.insert( std::make_pair( std::string("force_iframe"), 			[this]( const nlohmann::json &paramsIn ){ this->ForceIFrame( paramsIn ); } ) );
	m_publicApiMap.insert( std::make_pair( std::string("report_settings"), 			[this]( const nlohmann::json &paramsIn ){ this->ReportSettings( paramsIn ); } ) );
	m_publicApiMap.insert( std::make_pair( std::string("report_health"), 			[this]( const nlohmann::json &paramsIn ){ this->ReportHealth( paramsIn ); } ) );
	m_publicApiMap.insert( std::make_pair( std::string("report_api"), 				[this]( const nlohmann::json &paramsIn ){ this->ReportAPI( paramsIn ); } ) );
	m_publicApiMap.insert( std::make_pair( std::string("apply_settings"),			[this]( const nlohmann::json &paramsIn ){ this->ApplySettings( paramsIn ); } ) );
	
	// Settings API
	m_settingsApiMap.insert( std::make_pair( std::string("framerate"), 				[this]( const nlohmann::json &paramsIn ){ this->SetFramerate( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("bitrate"), 				[this]( const nlohmann::json &paramsIn ){ this->SetBitrate( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("goplen"), 				[this]( const nlohmann::json &paramsIn ){ this->SetGOPLength( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("gop_hierarchy_level"),	[this]( const nlohmann::json &paramsIn ){ this->SetGOPHierarchy( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("avc_profile"), 			[this]( const nlohmann::json &paramsIn ){ this->SetAVCProfile( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("avc_level"), 				[this]( const nlohmann::json &paramsIn ){ this->SetAVCLevel( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("maxnal"), 				[this]( const nlohmann::json &paramsIn ){ this->SetMaxNALSize( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("vui"), 					[this]( const nlohmann::json &paramsIn ){ this->SetVUI( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("pict_timing"), 			[this]( const nlohmann::json &paramsIn ){ this->SetPictTiming( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("max_framesize"), 			[this]( const nlohmann::json &paramsIn ){ this->SetMaxIFrameSize( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("compression_quality"),	[this]( const nlohmann::json &paramsIn ){ this->SetCompressionQuality( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("flip_vertical"), 			[this]( const nlohmann::json &paramsIn ){ this->SetFlipVertical( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("flip_horizontal"), 		[this]( const nlohmann::json &paramsIn ){ this->SetFlipHorizontal( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("contrast"), 				[this]( const nlohmann::json &paramsIn ){ this->SetContrast( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("zoom"), 					[this]( const nlohmann::json &paramsIn ){ this->SetZoom( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("pan"), 					[this]( const nlohmann::json &paramsIn ){ this->SetPan( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("tilt"), 					[this]( const nlohmann::json &paramsIn ){ this->SetTilt( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("pantilt"), 				[this]( const nlohmann::json &paramsIn ){ this->SetPantilt( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("brightness"), 			[this]( const nlohmann::json &paramsIn ){ this->SetBrightness( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("hue"), 					[this]( const nlohmann::json &paramsIn ){ this->SetHue( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("gamma"), 					[this]( const nlohmann::json &paramsIn ){ this->SetGamma( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("saturation"), 			[this]( const nlohmann::json &paramsIn ){ this->SetSaturation( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("gain"), 					[this]( const nlohmann::json &paramsIn ){ this->SetGain( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("sharpness"), 				[this]( const nlohmann::json &paramsIn ){ this->SetSharpness( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("max_analog_gain"), 		[this]( const nlohmann::json &paramsIn ){ this->SetMaxAnalogGain( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("histogram_eq"), 			[this]( const nlohmann::json &paramsIn ){ this->SetHistogramEQ( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("sharpen_filter"), 		[this]( const nlohmann::json &paramsIn ){ this->SetSharpenFilter( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("min_exp_framerate"), 		[this]( const nlohmann::json &paramsIn ){ this->SetMinAutoExposureFramerate( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("tf_strength"), 			[this]( const nlohmann::json &paramsIn ){ this->SetTemporalFilterStrength( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("gain_multiplier"), 		[this]( const nlohmann::json &paramsIn ){ this->SetGainMultiplier( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("exp"), 					[this]( const nlohmann::json &paramsIn ){ this->SetExposureMode( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("nf"), 					[this]( const nlohmann::json &paramsIn ){ this->SetNoiseFilterMode( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("wb"), 					[this]( const nlohmann::json &paramsIn ){ this->SetWhiteBalanceMode( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("wdr"), 					[this]( const nlohmann::json &paramsIn ){ this->SetWideDynamicRangeMode( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("zone_exp"), 				[this]( const nlohmann::json &paramsIn ){ this->SetZoneExposure( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("zone_wb"), 				[this]( const nlohmann::json &paramsIn ){ this->SetZoneWhiteBalance( paramsIn ); } ) );
	m_settingsApiMap.insert( std::make_pair( std::string("pwr_line_freq"), 			[this]( const nlohmann::json &paramsIn ){ this->SetPowerLineFrequency( paramsIn ); } ) );

	// Private API - For convenience of being able to loop through all possible settings and get their current values
	m_privateApiMap.insert( std::make_pair( std::string("channel_info"), 		[this](){ this->GetChannelInfo(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("framerate"), 			[this](){ this->GetFramerate(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("bitrate"), 			[this](){ this->GetBitrate(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("goplen"), 				[this](){ this->GetGOPLength(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("gop_hierarchy_level"),	[this](){ this->GetGOPHierarchy(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("avc_profile"), 		[this](){ this->GetAVCProfile(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("avc_level"), 			[this](){ this->GetAVCLevel(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("maxnal"), 				[this](){ this->GetMaxNALSize(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("vui"), 				[this](){ this->GetVUI(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("pict_timing"), 		[this](){ this->GetPictTiming(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("max_framesize"), 		[this](){ this->GetMaxIFrameSize(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("compression_quality"),	[this](){ this->GetCompressionQuality(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("flip_vertical"), 		[this](){ this->GetFlipVertical(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("flip_horizontal"), 	[this](){ this->GetFlipHorizontal(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("contrast"), 			[this](){ this->GetContrast(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("zoom"), 				[this](){ this->GetZoom(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("pan"), 				[this](){ this->GetPan(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("tilt"), 				[this](){ this->GetTilt(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("pantilt"), 			[this](){ this->GetPantilt(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("brightness"), 			[this](){ this->GetBrightness(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("hue"), 				[this](){ this->GetHue(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("gamma"), 				[this](){ this->GetGamma(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("saturation"), 			[this](){ this->GetSaturation(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("gain"), 				[this](){ this->GetGain(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("sharpness"), 			[this](){ this->GetSharpness(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("max_analog_gain"), 	[this](){ this->GetMaxAnalogGain(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("histogram_eq"), 		[this](){ this->GetHistogramEQ(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("sharpen_filter"), 		[this](){ this->GetSharpenFilter(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("min_exp_framerate"), 	[this](){ this->GetMinAutoExposureFramerate(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("tf_strength"), 		[this](){ this->GetTemporalFilterStrength(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("gain_multiplier"), 	[this](){ this->GetGainMultiplier(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("exp"), 				[this](){ this->GetExposureMode(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("nf"), 					[this](){ this->GetNoiseFilterMode(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("wb"), 					[this](){ this->GetWhiteBalanceMode(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("wdr"), 				[this](){ this->GetWideDynamicRangeMode(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("zone_exp"), 			[this](){ this->GetZoneExposure(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("zone_wb"), 			[this](){ this->GetZoneWhiteBalance(); } ) );
	m_privateApiMap.insert( std::make_pair( std::string("pwr_line_freq"), 		[this](){ this->GetPowerLineFrequency(); } ) );
}

bool CVideoChannel::IsCommandSupported( const std::string &commandNameIn )
{	
	try
	{
		json formats = gc6500::api.at( "publicAPI" ).at( commandNameIn ).at( "formats" );
		
		// Check to see if this command is supported for this channel's format
		for( json::iterator it = formats.begin(); it != formats.end(); ++it ) 
		{
			// Format matched, or the command is supported for all format types
			if( *it == m_settings.at( "format" ).at( "value" ) || *it == "all" )
			{
				return true;
			}
		}
		
		return false;
	}
	catch( std::exception &e )
	{
		std::cerr << "Error checking command support: " << std::string( e.what() );
		return false;
	}
}

bool CVideoChannel::IsSettingSupported( const std::string &settingNameIn )
{
	try
	{
		json formats = gc6500::api.at( "settingsAPI" ).at( settingNameIn ).at( "formats" );
		
		// Check to see if this setting is supported for this channel's format
		for( json::iterator it = formats.begin(); it != formats.end(); ++it ) 
		{
			// Format matched, or the command is supported for all format types
			if( *it == m_settings.at( "format" ).at( "value" ) || *it == "all" )
			{
				return true;
			}			
		}
		
		return false;
	}
	catch( std::exception &e )
	{
		std::cerr << "Error checking setting support: " << std::string( e.what() );
		return false;
	}
}

void CVideoChannel::ValidateCommand( const nlohmann::json &commandIn )
{
	std::string commandName;
	
	try
	{
		// Loop through the API definition for the given command and validate the parameters
		commandName = commandIn.at( "chCmd" ).get<std::string>();

		json apiParams 		= gc6500::api.at( "publicAPI" ).at( commandName ).at( "params" );
		json commandParams 	= commandIn.at( "params" ); 
		
		if( apiParams.empty() == true )
		{
			// Nothing to validate
			return;
		}
		else
		{
			if( commandParams.empty() )
			{
				throw std::runtime_error( "Parameters required, none supplied" );
			}
		}
		
		// Loop through the parameter definitions for this command in the API and validate them
		for( json::iterator it = commandParams.begin(); it != commandParams.end(); ++it ) 
		{
			const std::string paramName( it.key() );
			json apiParam = apiParams.at( paramName );
			
			try
			{
				ValidateParameterType( apiParam, it.value() );
			}
			catch( const std::exception &e )
			{
				throw std::runtime_error( "Parameter [" + paramName + "] type did not match API. Expected type: <" + std::string( e.what() ) + ">" );
			}
			
			try
			{
				ValidateParameterValue( apiParam, it.value() );
			}
			catch( const std::exception &e )
			{
				throw std::runtime_error( "Parameter [" + paramName + "] value did not match API . Error raised: " + std::string( e.what() ) );
			}
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command Invalid: [" + commandName + "]. Reason: " + std::string( e.what() ) );
	}
}

void CVideoChannel::ValidateSetting( const std::string &settingNameIn, const nlohmann::json &settingIn )
{
	try
	{
		if( !IsSettingSupported( settingNameIn ) )
		{
			throw std::runtime_error( "Setting not supported for this video format." );
		}
		
		json apiParams 		= gc6500::api.at( "settingsAPI" ).at( settingNameIn ).at( "params" );
		json settingParams 	= settingIn;
		
		if( apiParams.empty() == true )
		{
			// Nothing to validate
			return;
		}
		else
		{
			if( settingParams.empty() )
			{
				throw std::runtime_error( "Parameters required, none supplied" );
			}
		}
		
		// Loop through the parameter definitions for this command in the API and validate them
		for( json::iterator it = settingParams.begin(); it != settingParams.end(); ++it ) 
		{
			const std::string paramName( it.key() );			
			json apiParam = apiParams.at( paramName );
			
			try
			{
				ValidateParameterType( apiParam, it.value() );
			}
			catch( const std::exception &e )
			{
				throw std::runtime_error( "Parameter [" + paramName + "] type did not match API. Expected type: <" + std::string( e.what() ) + ">" );
			}
			
			try
			{
				ValidateParameterValue( apiParam, it.value() );
			}
			catch( const std::exception &e )
			{
				throw std::runtime_error( "Parameter [" + paramName + "] value did not match API . Error raised: " + std::string( e.what() ) );
			}
		}
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Setting Invalid: [" + settingNameIn + "]. Reason: " + std::string( e.what() ) );
	}
}

void CVideoChannel::ValidateParameterType( const nlohmann::json &paramApiIn, const nlohmann::json &paramIn )
{
	const std::string type( paramApiIn.at( "type" ).get<std::string>() );
	
	if( type == "object" )
	{
		if( paramIn.is_object() == false )
		{
			throw std::runtime_error( type );
		}
	}
	else if( type == "int" || type == "float" )
	{
		if( paramIn.is_number() == false )
		{
			throw std::runtime_error( type );
		}
	}
	else if( type == "boolean" )
	{
		if( paramIn.is_boolean() == false )
		{
			throw std::runtime_error( type );
		}
	}
	else if( type == "string" )
	{
		if( paramIn.is_object() == false )
		{
			throw std::runtime_error( type );
		}
	}
}

void CVideoChannel::ValidateParameterValue( const nlohmann::json &paramApiIn, const nlohmann::json &paramIn )
{
	const std::string type( paramApiIn.at( "type" ).get<std::string>() );
	
	if( type == "object" )
	{
		// No way to validate
	}
	else if( type == "int" || type == "float" )
	{
		// If a min and max are specified, validate the values
		if( paramApiIn.find( "min" ) != paramApiIn.end() && paramApiIn.find( "max" ) != paramApiIn.end() ) 
		{
			if( paramIn < paramApiIn.at( "min" ) || paramIn > paramApiIn.at( "max" ) )
			{
				throw std::runtime_error( "Out of range" );
			}
		}
	}
	else if( type == "boolean" )
	{
		// No need to validate
	}
	else if( type == "string" )
	{
		try
		{
			json options = paramApiIn.at( "options" );
			
			// Check to see if the passed in option matches a valid option
			if ( std::none_of( options.begin(), options.end(), [ paramIn ]( const json &option ){ return option == paramIn; } ) )
			{
				throw std::runtime_error( "Invalid option: " + paramIn.get<std::string>() );
			}
		}
		catch( const std::exception &e )
		{
			// Options field wasn't present. Add further string validation here if necessary
		}
	}
}


///////////////////////////////////////
// Public API
///////////////////////////////////////

// General
void CVideoChannel::ReportSettings( const nlohmann::json &paramsIn )
{	
	// Report current value of all settings for this channel
	m_eventEmitter.Emit( "settings", m_settings );
}

void CVideoChannel::ReportHealth( const nlohmann::json &paramsIn )
{
	json health = 
	{
		{ "fps", (float)m_muxer.m_fps },
		{ "droppedFrames", (int)m_muxer.m_droppedFrames },
		{ "latency_us", (int)m_muxer.m_latency_us }
	};
	
	m_eventEmitter.Emit( "health", health );
}

void CVideoChannel::ReportAPI( const nlohmann::json &paramsIn )
{	
	m_eventEmitter.Emit( "api", m_api );
}

void CVideoChannel::StartVideo( const nlohmann::json &paramsIn )
{
	// TODO: Eventually, save this in default settings
	mxuvc_video_set_vui( m_channel, 1 );
	mxuvc_video_set_pict_timing( m_channel, 1);
	
	if( mxuvc_video_start( m_channel ) )
	{
		throw std::runtime_error( "Command failed: StartVideo[" + m_channelString + "]: MXUVC failure" );
	}
	
	m_eventEmitter.Emit( "status", "video_started" );
}

void CVideoChannel::StopVideo( const nlohmann::json &paramsIn )
{
	// Clear the video buffer so it is fresh when video is started again
	m_muxer.m_inputBuffer.Clear();
	
	if( mxuvc_video_stop( m_channel ) )
	{
		throw std::runtime_error( "Command failed: StopVideo[" + m_channelString + "]: MXUVC failure" );
	}
	
	m_eventEmitter.Emit( "status", "video_stopped" );
}

void CVideoChannel::ForceIFrame( const nlohmann::json &paramsIn )
{
	if( mxuvc_video_force_iframe( m_channel ) )
	{
		throw std::runtime_error( "MXUVC Failure" );
	}
}

void CVideoChannel::ApplySettings( const nlohmann::json &paramsIn )
{	
	// paramsIn format:
	// { 
	//		<setting1>: <setting1_params>,
	//		<setting2>: <setting2_params>,
	//		<settingX>: <settingX_params>
	// }
	
	// settingX_params format:
	// { 
	//		<param1>: <value1>,
	//		<param2>: <value2>
	//		<paramX>: <valueX>
	// }
	
	json update;
	
	for( json::const_iterator it = paramsIn.at( "settings" ).begin(); it != paramsIn.at( "settings" ).end(); ++it ) 
	{
		try
		{
			// Validate setting
			ValidateSetting( it.key(), it.value() );
			
			// Call specified channel command with appropriate API function using passed in value
			m_settingsApiMap.at( it.key() )( it.value() );

			// If we didn't throw, the update was successful. Add to our update
			update[ it.key() ] = m_settings[ it.key() ];
		}
		catch( const std::exception &e )
		{
			cerr << "Failed to apply setting: " << it.key() << ": " << e.what() << endl;
		}
	}
	
	// Issue update for all changed settings
	m_eventEmitter.Emit( "settings", update );
}

///////////////////////////////////////
// Settings API
///////////////////////////////////////

void CVideoChannel::SetFramerate( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_framerate( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetFramerate();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetFramerate[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetBitrate( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_bitrate( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetBitrate();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetBitrate[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetGOPLength( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_goplen( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetGOPLength();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetGOPLength[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetGOPHierarchy( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_gop_hierarchy_level( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetGOPHierarchy();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetGOPHierarchy[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetAVCProfile( const nlohmann::json &paramsIn )
{
	try
	{
		std::string temp = paramsIn.at( "value" ).get<std::string>();
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
		
		GetAVCProfile();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetAVCProfile[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetAVCLevel( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_avc_level( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetAVCLevel();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetAVCLevel[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetMaxNALSize( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_maxnal( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetMaxNALSize();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetMaxNALSize[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetVUI( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_vui( m_channel, paramsIn.at( "enabled" ).get<bool>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetVUI();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: EnableVUI[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetPictTiming( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_pict_timing( m_channel, paramsIn.at( "enabled" ).get<bool>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetPictTiming();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: EnablePictTiming[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetMaxIFrameSize( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_max_framesize( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetMaxIFrameSize();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetMaxIFrameSize[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetCompressionQuality( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_compression_quality( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetCompressionQuality();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetCompressionQuality[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

// ----------------
// Sensor Settings
// ----------------

void CVideoChannel::SetFlipVertical( const nlohmann::json &paramsIn )
{
	try
	{
		video_flip_t flip = ( paramsIn.at( "enabled" ).get<bool>() ? FLIP_ON : FLIP_OFF );
			
		if( mxuvc_video_set_flip_vertical( m_channel, flip ) )
		{
			throw std::runtime_error( "Failed to set Flip Vertical on channel: " + m_channelString );
		}
		
		GetFlipVertical();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: EnableFlipVertical[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetFlipHorizontal( const nlohmann::json &paramsIn )
{
	try
	{
		video_flip_t flip = ( paramsIn.at( "enabled" ).get<bool>() ? FLIP_ON : FLIP_OFF );
			
		if( mxuvc_video_set_flip_horizontal( m_channel, flip ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetFlipHorizontal();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: EnableFlipHorizontal[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetContrast( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_contrast( m_channel, paramsIn.at( "value" ).get<uint16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetContrast();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetContrast[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetZoom( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_zoom( m_channel, paramsIn.at( "value" ).get<uint16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetZoom();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetZoom[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetPan( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_pan( m_channel, paramsIn.at( "value" ).get<int32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetPan();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetPan[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetTilt( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_tilt( m_channel, paramsIn.at( "value" ).get<int32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetTilt();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetTilt[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetPantilt( const nlohmann::json &paramsIn )
{
	try
	{
		int32_t pan = paramsIn.at( "pan" ).get<int32_t>();
		int32_t tilt = paramsIn.at( "tilt" ).get<int32_t>();
		
		if( mxuvc_video_set_pantilt( m_channel, pan, tilt ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetPantilt();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetPantilt[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetBrightness( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_brightness( m_channel, paramsIn.at( "value" ).get<int16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetBrightness();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetBrightness[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetHue( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_hue( m_channel, paramsIn.at( "value" ).get<int16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetHue();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetHue[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetGamma( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_gamma( m_channel, paramsIn.at( "value" ).get<uint16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetGamma();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetGamma[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetSaturation( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_saturation( m_channel, paramsIn.at( "value" ).get<uint16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetSaturation();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetSaturation[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetGain( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_gain( m_channel, paramsIn.at( "value" ).get<uint16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetGain();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetGain[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetSharpness( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_sharpness( m_channel, paramsIn.at( "value" ).get<uint16_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetSharpness();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetSharpness[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetMaxAnalogGain( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_max_analog_gain( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetMaxAnalogGain();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetMaxAnalogGain[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetHistogramEQ( const nlohmann::json &paramsIn )
{
	try
	{
		histo_eq_t histEq = ( paramsIn.at( "enabled" ).get<bool>() ? HISTO_EQ_ON : HISTO_EQ_OFF );
		
		if( mxuvc_video_set_histogram_eq( m_channel, histEq ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetHistogramEQ();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetHistogramEQ[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetSharpenFilter( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_sharpen_filter( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetSharpenFilter();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetSharpenFilter[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetMinAutoExposureFramerate( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_min_exp_framerate( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetMinAutoExposureFramerate();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetMinAutoExposureFramerate[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetTemporalFilterStrength( const nlohmann::json &paramsIn )
{
	try
	{
		bool enabled = paramsIn.at( "enabled" ).get<bool>();
		
		if( enabled == true )
		{
			if( mxuvc_video_set_tf_strength( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
			{
				throw std::runtime_error( "MXUVC Failure" );
			}
		}
		else
		{
			if( mxuvc_video_set_tf_strength( m_channel, 0 ) )
			{
				throw std::runtime_error( "MXUVC Failure" );
			}
		}
		
		GetTemporalFilterStrength();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetTemporalFilterStrength[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetGainMultiplier( const nlohmann::json &paramsIn )
{
	try
	{
		if( mxuvc_video_set_gain_multiplier( m_channel, paramsIn.at( "value" ).get<uint32_t>() ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetGainMultiplier();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetGainMultiplier[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetExposureMode( const nlohmann::json &paramsIn )
{
	try
	{
		exp_set_t sel;
		uint16_t value = 0;
		std::string mode = paramsIn.at( "mode" ).get<std::string>();
		
		if( mode == "auto" )
		{
			sel = EXP_AUTO;
		}
		else if( mode == "manual" )
		{
			sel = EXP_MANUAL;
			value = paramsIn.at( "value" ).get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid exposure mode" );
		}
		
		if( mxuvc_video_set_exp( m_channel, sel, value ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetExposureMode();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetExposureMode[" + m_channelString + "]: " + std::string( e.what() ) );
	}
		
}

void CVideoChannel::SetNoiseFilterMode( const nlohmann::json &paramsIn )
{
	try
	{
		noise_filter_mode_t sel;
		uint16_t value = 0;
		std::string mode = paramsIn.at( "mode" ).get<std::string>();
		
		if( mode == "auto" )
		{
			sel = NF_MODE_AUTO;
		}
		else if( mode == "manual" )
		{
			sel = NF_MODE_MANUAL;
			value = paramsIn.at( "value" ).get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid filter mode" );
		}
		
		if( mxuvc_video_set_nf( m_channel, sel, value ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetNoiseFilterMode();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetNoiseFilterMode[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetWhiteBalanceMode( const nlohmann::json &paramsIn )
{
	try
	{
		white_balance_mode_t sel;
		uint16_t value = 0;
		std::string mode = paramsIn.at( "mode" ).get<std::string>();
		
		if( mode == "auto" )
		{
			sel = WB_MODE_AUTO;
		}
		else if( mode == "manual" )
		{
			sel = WB_MODE_MANUAL;
			value = paramsIn.at( "value" ).get<uint16_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid wb mode" );
		}
		
		if( mxuvc_video_set_wb( m_channel, sel, value ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetWhiteBalanceMode();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetWhiteBalanceMode[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetWideDynamicRangeMode( const nlohmann::json &paramsIn )
{
	try
	{
		wdr_mode_t sel;
		uint8_t value = 0;
		
		bool enabled = paramsIn.at( "enabled" ).get<bool>();
		std::string mode = paramsIn.at( "mode" ).get<std::string>();
		
		if( enabled == false )
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
			value = paramsIn.at( "value" ).get<uint8_t>();
		}
		else
		{
			throw std::runtime_error( "Invalid wb mode" );
		}
		
		if( mxuvc_video_set_wdr( m_channel, sel, value ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetWideDynamicRangeMode();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetWideDynamicRangeMode[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetZoneExposure( const nlohmann::json &paramsIn )
{
	try
	{
		zone_exp_set_t sel;
		uint16_t value = 0;
		bool enabled = paramsIn.at( "enabled" ).get<bool>();
		
		if( enabled )
		{
			sel = ZONE_EXP_DISABLE;
		}
		else
		{
			sel = ZONE_EXP_ENABLE;
			value = paramsIn.at( "value" ).get<uint16_t>();
		}
		
		if( mxuvc_video_set_zone_exp( m_channel, sel, value ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetZoneExposure();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetZoneExposure[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetZoneWhiteBalance( const nlohmann::json &paramsIn )
{
	try
	{
		zone_wb_set_t sel;
		uint16_t value = 0;
		bool enabled = paramsIn.at( "enabled" ).get<bool>();
		
		if( enabled )
		{
			sel = ZONE_WB_DISABLE;
		}
		else
		{
			sel = ZONE_WB_ENABLE;
			value = paramsIn.at( "value" ).get<uint16_t>();
		}

		if( mxuvc_video_set_zone_wb( m_channel, sel, value ) )
		{
			throw std::runtime_error( "MXUVC Failure" );
		}
		
		GetZoneWhiteBalance();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetZoneWhiteBalance[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

void CVideoChannel::SetPowerLineFrequency( const nlohmann::json &paramsIn )
{
	try
	{
		pwr_line_freq_mode_t pwrMode;
		
		bool enabled = paramsIn.at( "enabled" ).get<bool>();
		std::string mode = paramsIn.at( "mode" ).get<std::string>();
		
		if( enabled == false )
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
		
		GetPowerLineFrequency();
	}
	catch( const std::exception &e )
	{
		throw std::runtime_error( "Command failed: SetPowerLineFrequency[" + m_channelString + "]: " + std::string( e.what() ) );
	}
}

///////////////
// Private API
///////////////

void CVideoChannel::GetAllSettings()
{
	// Loop through private API map and call all functions to retrieve all settings available for this chip
	// TODO: Eventually, this should use the loaded API to smartly load only what is relevant based on the channel's format
	for ( auto it = m_privateApiMap.begin(); it != m_privateApiMap.end(); ++it )
	{
		try
		{
			it->second();
		}
		catch( const std::exception &e )
		{
			if( it->first == "channel_info" )
			{
				throw std::runtime_error( "Failed to get critical setting: channel_info" );
			}
			else
			{
				cerr << "Failed to get non-critical parameter in group: " << it->first << endl;
			}
		}
	}
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
				m_settings[ "format" ][ "value" ] = "h264";
				break;
			case VID_FORMAT_MJPEG_RAW:
				m_settings[ "format" ][ "value" ] = "mjpeg";
				break;
			default:
				throw std::runtime_error( "Unsupported video format" );
				break;
		}
		
		m_settings[ "width" ][ "value" ] 	= info.width;
		m_settings[ "height" ][ "value" ] 	= info.height;
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
		m_settings[ "framerate" ][ "value" ] = value;
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
		m_settings[ "bitrate" ][ "value" ] = value;
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
		m_settings[ "goplen" ][ "value" ] = value;
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
		m_settings[ "gop_hierarchy_level" ][ "value" ] = value;
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
				m_settings[ "avc_profile" ][ "value" ] = "baseline";
				break;
			case PROFILE_MAIN:
				m_settings[ "avc_profile" ][ "value" ] = "main";
				break;
			case PROFILE_HIGH:
				m_settings[ "avc_profile" ][ "value" ] = "high";
				break;
			default:
				m_settings[ "avc_profile" ] = "unknown";
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
		m_settings[ "avc_level" ][ "value" ] = value;
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
		m_settings[ "maxnal" ][ "enabled" ] = ( value != 0 );
		m_settings[ "maxnal" ][ "value" ] 	= value;
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
		m_settings[ "vui" ][ "enabled" ] = (bool)value;
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
		m_settings[ "pict_timing" ][ "enabled" ] = (bool)value;
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
		m_settings[ "max_framesize" ][ "value" ] = value;
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
		m_settings[ "compression_quality" ][ "value" ] = value;
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
		m_settings[ "flip_vertical" ][ "enabled" ] = ( flip == FLIP_ON );
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
		m_settings[ "flip_horizontal" ][ "enabled" ] = ( flip == FLIP_ON );
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
		m_settings[ "contrast" ][ "value" ] = value;
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
		m_settings[ "zoom" ][ "value" ] = value;
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
		m_settings[ "pan" ][ "value" ] = value;
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
		m_settings[ "tilt" ][ "value" ] = value;
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
		m_settings[ "pan" ][ "value" ] 	= pan;
		m_settings[ "tilt" ][ "value" ] = tilt;
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
		m_settings[ "brightness" ][ "value" ] = value;
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
		m_settings[ "hue" ][ "value" ] = value;
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
		m_settings[ "gamma" ][ "value" ] = value;
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
		m_settings[ "saturation" ][ "value" ] = value;
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
		m_settings[ "gain" ][ "value" ] = value;
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
		m_settings[ "sharpness" ][ "value" ] = value;
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
		m_settings[ "max_analog_gain" ][ "value" ] = value;
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
		m_settings[ "histogram_eq" ][ "enabled" ] = ( histo == HISTO_EQ_ON );
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
		m_settings[ "sharpen_filter" ][ "value" ] = value;
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
		m_settings[ "min_exp_framerate" ][ "value" ] = value;
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
		m_settings[ "tf_strength" ][ "enabled" ] = ( value != 0 );
		m_settings[ "tf_strength" ][ "value" ] = value;
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
		m_settings[ "gain_multiplier" ][ "value" ] = value;
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
				m_settings[ "exp" ][ "mode" ] = "auto";
				break;
			case EXP_MANUAL:
				m_settings[ "exp" ][ "mode" ] = "manual";
				break;
			default:
				m_settings[ "exp" ][ "mode" ] = "unknown";
				break;
		}
		
		m_settings[ "exp" ][ "value" ] = value;
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
				m_settings[ "nf" ][ "mode" ] = "auto";
				break;
			case NF_MODE_MANUAL:
				m_settings[ "nf" ][ "mode" ] = "manual";
				break;
			default:
				m_settings[ "nf" ][ "mode" ] = "unknown";
				break;
		}
		
		m_settings[ "nf" ][ "value" ] = value;
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
				m_settings[ "wb" ][ "mode" ] = "auto";
				break;
			case WB_MODE_MANUAL:
				m_settings[ "wb" ][ "mode" ] = "manual";
				break;
			default:
				m_settings[ "wb" ][ "mode" ] = "unknown";
				break;
		}
		
		m_settings[ "wb" ][ "value" ] = value;
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
				m_settings[ "wdr" ][ "mode" ] 		= "auto";
				m_settings[ "wdr" ][ "enabled" ] 	= true;
				break;
			case WDR_MANUAL:
				m_settings[ "wdr" ][ "mode" ] 		= "manual";
				m_settings[ "wdr" ][ "enabled" ] 	= true;
				break;
			case WDR_DISABLE:
				m_settings[ "wdr" ][ "mode" ] 		= "disabled";
				m_settings[ "wdr" ][ "enabled" ] 	= false;
				break;
			default:
				m_settings[ "wdr" ][ "mode" ] 		= "unknown";
				m_settings[ "wdr" ][ "enabled" ] 	= false;
				break;
		}
		
		m_settings[ "wdr" ][ "value" ] = value;
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
				m_settings[ "zone_exp" ][ "enabled" ] 	= false;
				break;
			case ZONE_EXP_ENABLE:
				m_settings[ "zone_exp" ][ "enabled" ] 	= true;
				break;
			default:
				m_settings[ "zone_exp" ][ "enabled" ] 	= false;
				break;
		}
		
		m_settings[ "zone_exp" ][ "value" ] = value;
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
				m_settings[ "zone_wb" ][ "enabled" ] 	= false;
				break;
			case ZONE_WB_ENABLE:
				m_settings[ "zone_wb" ][ "enabled" ] 	= true;
				break;
			default:
				m_settings[ "zone_wb" ][ "enabled" ] 	= false;
				break;
		}
		
		m_settings[ "zone_wb" ][ "value" ] = value;
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
				m_settings[ "pwr_line_freq" ][ "mode" ] 	= "disabled";
				m_settings[ "pwr_line_freq" ][ "enabled" ] 	= false;
				break;
			case PWR_LINE_FREQ_MODE_50HZ:
				m_settings[ "pwr_line_freq" ][ "mode" ] 	= "50HZ";
				m_settings[ "pwr_line_freq" ][ "enabled" ] 	= true;
				break;
			case PWR_LINE_FREQ_MODE_60HZ:
				m_settings[ "pwr_line_freq" ][ "mode" ] 	= "60HZ";
				m_settings[ "pwr_line_freq" ][ "enabled" ] 	= true;
				break;
			default:
				m_settings[ "pwr_line_freq" ][ "mode" ] 	= "disabled";
				m_settings[ "pwr_line_freq" ][ "enabled" ] 	= false;
				break;
		}
	}
}
