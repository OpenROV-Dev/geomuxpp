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
	, m_geomuxStatusPub( m_context.createPublishSocket() )
	, m_geomuxCmdSub( m_context.createSubscribeSocket() )
	, m_gc6500( ( (m_arguments.size() > 1 ) ? m_arguments.at( 1 ) : "0" ), &m_context, &m_geomuxStatusPub )
{	
	// Bind publisher and subscriber
	m_geomuxStatusPub.bind( "ipc:///tmp/geomux_status.ipc" );
	m_geomuxCmdSub.bind( "ipc:///tmp/geomux_command.ipc" );
	
	// Subscribe to anything (needs to be valid json to survive parsing)
	m_geomuxCmdSub.subscribe();
}

CGeomux::~CGeomux()
{
}

void CGeomux::Run()
{
	try
	{	
		cout << "Application Started." << endl;

		while( !m_quit )
		{
			Update();
		}	

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
		cerr << "Error handling message: " << e.what() << endl;
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
	m_quit = true;
}

void CGeomux::Restart()
{
	m_quit = true;
	m_restart = true;
}