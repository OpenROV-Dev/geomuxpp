// Includes
#include <CpperoMQ/All.hpp>

#include "CGC6500.h"
#include "CVideoChannel.h"
#include "CStatusPublisher.h"
#include "Utility.h"

extern "C" 
{ 
	// FFmpeg
	#include <libavformat/avformat.h>
}

using namespace std;

CGC6500::CGC6500( const std::string &deviceOffsetIn, CpperoMQ::Context *contextIn, CStatusPublisher *publisherIn )
	: m_deviceOffset( deviceOffsetIn )
	, m_pContext( contextIn )
	, m_pStatusPublisher( publisherIn )
{
	// Initialize libavcodec, and register all codecs and formats. We only want to do this once.
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
	cout << "Cleaning up CGC6500" << endl;
	
	// Deinit mxuvc
	mxuvc_video_deinit();
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
		m_pChannels.push_back( util::make_unique<CVideoChannel>( (video_channel_t)i , m_pContext, m_pStatusPublisher ) );
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
		std::cerr << "Error handling message: " << e.what() << std::endl;
	}
}
