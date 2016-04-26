// Includes
#include "CGC6500.h"
#include "Utility.h"

extern "C" 
{ 
	// FFmpeg
	#include <libavformat/avformat.h>

}

CGC6500::CGC6500( CpperoMQ::Context *contextIn, CpperoMQ::PublishSocket *geomuxStatusPubIn )
	: m_deviceOffset( "0" )
	, m_pGeomuxStatusPub( geomuxStatusPubIn )
	, m_pContext( contextIn )
{
	// Initialize libavcodec, and register all codecs and formats.
	av_register_all();
	
	// Set a custom device offset
	std::string options( "dev_offset=" + m_deviceOffset + ",v4l_buffers=16" );
	
	// Initialize mxuvc
	if( mxuvc_video_init( "v4l2", options.c_str() ) )
	{
		throw std::runtime_error( "Failed to initialize mxuvc using device offset: " + m_deviceOffset );
	}
	
	// Create channels
	CreateChannels();
}

CGC6500::~CGC6500()
{
	// Deinit mxuvc
	if( mxuvc_video_deinit() )
	{
		throw std::runtime_error( "Deinit failed" );
	}
}

void CGC6500::SetDeviceOffset( const std::string &deviceOffsetIn )
{
	m_deviceOffset = deviceOffsetIn;
}

void CGC6500::CreateChannels()
{
	// Get the number of channels on the camera
	uint32_t channelCount;
	
	if( mxuvc_video_get_channel_count( &channelCount ) )
	{
		throw std::runtime_error( "Failed to get channel count." );
	}
	
	// Create a new CVideoChannel for each detected channel on the camera
	// TODO: Figure out why other channels arent working
	//for( uint32_t i = 0; i < channelCount; ++i )
	for( uint32_t i = 0; i < 1; ++i )
	{
		m_pChannels.push_back( util::make_unique<CVideoChannel>( m_pContext, (video_channel_t)i ) );
	}
	
	std::cout << "Channels created: " << m_pChannels.size() << std::endl;
}

void CGC6500::HandleMessage( const nlohmann::json &commandIn )
{
	try
	{		
		if( commandIn[ "cmd" ].get<std::string>() == "chCmd" )
		{
			size_t channelNum = commandIn.at( "ch" ).get<size_t>();
			
			// Pass message down to specified channel
			m_pChannels.at( channelNum )->HandleMessage( commandIn );
		}
		else
		{
			throw std::runtime_error( "Unknown command" );
		}
	}
	catch( const std::exception &e )
	{
		std::cerr << e.what() << std::endl;
	}
}
void CGC6500::EmitStatus( const std::string &statusIn )
{
	
}
void CGC6500::EmitError( const std::string &errorIn )
{
	
}
