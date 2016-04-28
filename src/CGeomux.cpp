// Includes
#include <csignal>
#include <unistd.h>
#include <json.hpp>

#include "CGeomux.h"

using namespace std;
using namespace CpperoMQ;

using json = nlohmann::json;

CGeomux::CGeomux( int argCountIn, char* argsIn[] )
	: CApp( argCountIn, argsIn )
	, m_geomuxCmdSub( m_context.createSubscribeSocket() )
	, m_statusPublisher( &m_context )
	, m_gc6500( ( (m_arguments.size() > 1 ) ? m_arguments.at( 1 ) : "0" ), &m_context, &m_statusPublisher )
{	
	// Bind subscriber
	m_geomuxCmdSub.bind( "ipc:///tmp/geomux_command.ipc" );
	
	// Subscribe to anything (needs to be valid json to survive parsing)
	m_geomuxCmdSub.subscribe();
}

CGeomux::~CGeomux(){ cout << "Cleaning up CGeomux" << endl; }

void CGeomux::Run()
{
	try
	{	
		cout << "Application Started." << endl;

		m_statusPublisher.EmitStatus( "ready" );
		
		while( !m_quit )
		{
			Update();
		}	
		
		m_statusPublisher.EmitStatus( "exiting" );

		cout << "Application Ended." << endl;

	}
	catch( const std::exception &e )
	{
		cerr << "Exception in Run(): " << e.what() << endl;
	}
}

void CGeomux::Update()
{	
	HandleMessages();
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
		m_statusPublisher.EmitError( e.what() );
		cerr << "Error handling message: " << e.what() << endl;
		return;
	}
}

void CGeomux::Shutdown()
{
	m_statusPublisher.EmitStatus( "shuttingDown" );
	m_quit = true;
}

void CGeomux::Restart()
{
	m_statusPublisher.EmitStatus( "restarting" );
	m_quit = true;
	m_restart = true;
}