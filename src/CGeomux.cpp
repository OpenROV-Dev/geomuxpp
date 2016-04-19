// Includes
#include <signal.h>
#include <unistd.h>

#include "CGeomux.h"
#include "easylogging.hpp"

#include <msgpack.hpp>

CGeomux::CGeomux( int argCountIn, char* argsIn[] )
	: CApp( argCountIn, argsIn )
	, m_quitApplication( false )
	, m_geocam( &m_muxer.m_inputBuffer )
{
}

CGeomux::~CGeomux()
{
}

void CGeomux::Run()
{
	try
	{	
		LOG( INFO ) << "Application Started.";

		// Bind the zmq publisher to the geomux ipc file
		//

		// Set up the muxer
		m_muxer.Initialize();
		
		// Initialize the main channel of camera 0
		m_geocam.InitDevice();
		
		m_geocam.GetPictureTiming();
		
		// Start capturing data from the camera
		m_geocam.StartVideo();
		
		m_geocam.ForceIFrame();
		
		usleep( 50000 );

		// Our application logic goes here.
		while( m_quitApplication == false )
		{
			Update();		
		}
		
		// Stop capturing data
		m_geocam.StopVideo();

		LOG( INFO ) << "Application Ended.";
	}
	catch( const std::exception &e )
	{
		LOG( ERROR ) << "Exception in Run(): " << e.what();
	}
	
}

void CGeomux::HandleSignal( int signalIdIn )
{
	// Print a new line to offset ctrl text
	std::cout << std::endl;

	if( signalIdIn == SIGINT )
	{
		LOG( INFO ) << "SIGINT Detected: Cleaning up gracefully...";
		
		m_quitApplication = true;
	}
	else
	{
		LOG( INFO ) << "Unknown signal detected: Ignoring...";
	}
}

void CGeomux::Update()
{
	{
		std::unique_lock<std::mutex> lock( m_muxer.m_inputBuffer.m_mutex );
		while( m_muxer.m_inputBuffer.GetSize() == 0 )
		{
			// Wait to be signaled that there is data
			m_muxer.m_inputBuffer.m_dataAvailableCondition.wait( lock );
		}
	}
	
	// Run the muxer's update function
	m_muxer.Update();
}