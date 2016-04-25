// Includes
#include "CGC6500.h"
#include "easylogging.hpp"
#include "Utility.h"

CGC6500::CGC6500( CpperoMQ::PublishSocket *geomuxStatusPubIn )
	: m_deviceOffset( "0" )
	, m_pGeomuxStatusPub( geomuxStatusPubIn )
{
	
}

CGC6500::~CGC6500(){}

void CGC6500::SetDeviceOffset( const std::string &deviceOffsetIn )
{
	m_deviceOffset = deviceOffsetIn;
}

void CGC6500::Initialize()
{
	LOG( INFO ) << "Initializing GC6500...";
	
	// Set a custom device offset
	std::string options( "dev_offset=" + m_deviceOffset );
	
	// Initialize mxuvc
	if( mxuvc_video_init( "v4l2", options.c_str() ) )
	{
		throw std::runtime_error( "Failed to initialize mxuvc using device offset: " + m_deviceOffset );
	}
	
	// Create channels
	CreateChannels();
	
	LOG( INFO ) << "GC6500 initialized!";
}

void CGC6500::Cleanup()
{
	LOG( INFO ) << "Cleaning up GC6500...";
	
	// Deinit mxuvc
	if( mxuvc_video_deinit() )
	{
		throw std::runtime_error( "Deinit failed" );
	}
	
	// Clear channels
	m_pChannels.clear();
	
	LOG( INFO ) << "GC6500 cleaned up!";
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
	for( uint32_t i = 0; i < channelCount; ++i )
	{
		m_pChannels.push_back( util::make_unique<CVideoChannel>( (video_channel_t)i ) );
	}
	
	LOG( INFO ) << "Channels created: " << m_pChannels.size();
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
		LOG( ERROR ) << e.what();
	}
}
void CGC6500::EmitStatus( const std::string &statusIn )
{
	
}
void CGC6500::EmitError( const std::string &errorIn )
{
	
}
