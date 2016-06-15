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
	, m_cameraOffset( ( (m_arguments.size() > 1 ) ? m_arguments.at( 1 ) : "0" ) )
	, m_commandSubscriber( m_cameraOffset, &m_context )
	, m_gc6500( m_cameraOffset, &m_context )
	, m_lastExecutionTime( std::chrono::steady_clock::now() )
{	
	
}

CGeomux::~CGeomux(){ cout << "Cleaning up CGeomux" << endl; }

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
	
	if( ( std::chrono::steady_clock::now() - m_lastExecutionTime ) > std::chrono::seconds( 5 ) )
	{
		if( !m_gc6500.IsAlive() )
		{
			cerr << "Camera connectivity lost. Shutting down!" << endl;
			Shutdown();
		}
		
		m_lastExecutionTime = std::chrono::steady_clock::now();
	}
}

void CGeomux::HandleMessages()
{
	try
	{		
		// Get message
		IncomingMessage msg;
		
		while( m_commandSubscriber.m_geomuxCmdSub.receive( msg ) )
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

void CGeomux::Shutdown()
{
	m_quit = true;
}

void CGeomux::Restart()
{
	m_quit = true;
	m_restart = true;
}