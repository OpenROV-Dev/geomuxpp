// Includes
#include <CpperoMQ/All.hpp>
#include <cctype>
#include <algorithm>

#include "CGC6500.h"
#include "CVideoChannel.h"
#include "Utility.h"

extern "C" 
{ 
	// FFmpeg
	#include <libavformat/avformat.h>
}

using namespace std;
using namespace CpperoMQ;
using json = nlohmann::json;


CGC6500::CGC6500( const std::string &cameraNameIn, CpperoMQ::Context *contextIn )
	: m_pContext( contextIn )
	, m_cameraName( cameraNameIn )
	, m_cameraRegistrar( contextIn )
{
	if( std::all_of( m_cameraName.begin(), m_cameraName.end(), ::isdigit ) == false )
	{
		throw std::runtime_error( "Camera offset must be numeric. Received: " + m_cameraName );
	}
	
	// Make sure a GC6500 SDK version was defined
	if( GC6500_VERSION == "" )
	{
		throw std::runtime_error( "No GC6500 SDK version defined! Cannot safely interact with the camera without knowing the API version!" );
	}
	
	// Initialize libavcodec, and register all codecs and formats. We only want to do this once.
	av_register_all();
	
	// Set a custom device offset
	std::string options( "dev_offset=" + m_cameraName + ",v4l_buffers=16" );
	
	// Initialize mxuvc
	if( mxuvc_video_init( "v4l2", options.c_str() ) )
	{
		throw std::runtime_error( "Failed to initialize mxuvc using device offset: " + m_cameraName );
	}
	
	m_initialized = true;
	
	cout << "Camera initialized" << endl;
	
	// Registration
	if( m_cameraRegistrar.RegisterCamera( m_cameraName ) == false )
	{
		throw std::runtime_error( "Failed to register camera: " + m_cameraName );
	}
	
	// Create channels
	CreateChannels();
}

CGC6500::~CGC6500()
{
	if( m_initialized )
	{
		// Deinit mxuvc
		mxuvc_video_deinit();
	}
	
	try
	{
		m_cameraRegistrar.UnregisterCamera( m_cameraName );
	}
	catch( const std::exception &e )
	{
		cerr << "Error cleaning up gc6500: " << e.what() << endl;
	}
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
		try
		{
			// Attempt to create channel
			auto channel( util::make_unique<CVideoChannel>( m_cameraName, (video_channel_t)i , m_pContext ) );
			
			m_pChannels.push_back( std::move( channel ) );
			
			cout << "Registering channel " << i << endl;
			
			// Register channel with cockpit
			if( m_cameraRegistrar.RegisterChannel( m_cameraName, i ) == false )
			{
				m_pChannels.pop_back();
				throw std::runtime_error( "Registration denied" );
			}
			
			cout << "Registered channel " << i << endl;
		}
		catch( const std::exception &e )
		{
			cerr << "Failed to create channel " << i << ": " << e.what() << endl;
		}
	}
	
	// Throw if no channels were created
	if( m_pChannels.size() == 0 )
	{
		throw std::runtime_error( "Failed to initialize any channels on camera: " + m_cameraName );
	}
	
	for( auto &channel : m_pChannels )
	{
		cout << "Initializing channel " << endl;
		channel->Initialize();
		cout << "Initialized channel " << endl;
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
