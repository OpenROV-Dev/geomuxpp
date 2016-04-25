// Includes
#include <signal.h>
#include <unistd.h>
#include <json.hpp>

#include "CGeomux.h"
#include "easylogging.hpp"

using namespace std;
using namespace CpperoMQ;

using json = nlohmann::json;

CGeomux::CGeomux( int argCountIn, char* argsIn[] )
	: CApp( argCountIn, argsIn )
	, m_geomuxStatusPub( m_context.createPublishSocket() )
	, m_geomuxCmdSub( m_context.createSubscribeSocket() )
	, m_gc6500( &m_geomuxStatusPub )
{
	if( m_arguments.size() > 1 )
	{
		// Set the device offset for the gc6500 to the first argument
		m_gc6500.SetDeviceOffset( m_arguments[ 1 ] );
	}
}

CGeomux::~CGeomux()
{
}

void CGeomux::Run()
{
	try
	{	
		do
		{
			if( m_restart )
			{
				LOG( INFO ) << "Application Restarted.";
				m_restart = false;
			}
			else
			{
				LOG( INFO ) << "Application Started.";
			}
			
			Initialize();
			
			while( !m_quit && !m_restart )
			{
				LOG( INFO ) << "Update";
				Update();
			}	
			
			Cleanup();

			LOG( INFO ) << "Application Ended.";
		}
		while( m_restart );
	}
	catch( const std::exception &e )
	{
		LOG( ERROR ) << "Exception in Run(): " << e.what();
		Cleanup();
	}
}

void CGeomux::HandleSignal( int signalIdIn )
{
	// Print a new line to offset ctrl text
	std::cout << std::endl;

	if( signalIdIn == SIGINT )
	{
		LOG( INFO ) << "SIGINT Detected: Cleaning up gracefully...";
		
		Shutdown();
	}
	else
	{
		LOG( INFO ) << "Unknown signal detected: Ignoring...";
	}
}

void CGeomux::Initialize()
{
	// Bind publisher and subscriber
	m_geomuxStatusPub.bind( "ipc:///tmp/geomux_status.ipc" );
	m_geomuxCmdSub.bind( "ipc:///tmp/geomux_command.ipc" );
	
	// Subscribe to anything (needs to be valid json to survive parsing)
	// m_geomuxCmdSub.setReceiveTimeout(0);
	m_geomuxCmdSub.subscribe();
	
	m_gc6500.Initialize();
	
	EmitStatus( "initialized" );
}

void CGeomux::Update()
{	
	HandleMessages();
}

void CGeomux::Cleanup()
{
	LOG( INFO ) << "Cleanup";
	m_gc6500.Cleanup();
}

void CGeomux::HandleMessages()
{
	try
	{
		// Get message
		IncomingMessage msg;
		
		while( m_geomuxCmdSub.receive( msg ) )
		{
			// Parse into json object
			json message = json::parse( string( msg.charData(), msg.size() ) );
			
			// Pass json message to processor
			if( message.at( "cmd" ) == "shutdown" )
			{
				Shutdown();
				break;
			}
			else if( message.at( "cmd" ) == "restart" )
			{
				LOG( INFO ) << "Restarting";
				Restart();
				break;
			}
			else
			{
				m_gc6500.HandleMessage( message );
			}
		}
		
	}
	catch( const std::exception &e )
	{
		LOG( ERROR ) << "Error handling message: " << e.what();
		EmitError( e.what() );
		return;
	}
}

void CGeomux::EmitStatus( const std::string &statusIn )
{
	// Create message
	json status = { { "status", statusIn } };
	
	m_geomuxStatusPub.send( OutgoingMessage( status.dump().c_str() ) );
}

void CGeomux::EmitError( const std::string &errorIn )
{
	// Create message
	json error = { { "error", errorIn } };
	
	m_geomuxStatusPub.send( OutgoingMessage( error.dump().c_str() ) );
}

void CGeomux::Shutdown()
{
	EmitStatus( "shuttingDown" );
	m_quit = true;
	m_restart = false;
}

void CGeomux::Restart()
{
	EmitStatus( "restarting" );
	m_restart = true;
}