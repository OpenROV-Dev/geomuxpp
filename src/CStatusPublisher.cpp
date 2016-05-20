#include "CStatusPublisher.h"
#include <unistd.h>

using namespace std;
using namespace CpperoMQ;

using json = nlohmann::json;

CStatusPublisher::CStatusPublisher( const std::string &cameraOffsetIn, CpperoMQ::Context *contextIn )
	: m_cameraOffset( cameraOffsetIn )
	, m_pContext( contextIn )
	, m_statusPub( m_pContext->createPublishSocket() )
	, m_geomuxRegRequester( m_pContext->createRequestSocket() )
{
	m_statusPub.bind( std::string( "ipc:///tmp/geomux_status" + cameraOffsetIn + ".ipc" ).c_str() );
	
	// Connect to the registration server
	m_geomuxRegRequester.connect( "ipc:///tmp/geomux_registration.ipc" );
	
	json registration = { { "offset", m_cameraOffset } };
	
	// Send a registration request
	m_geomuxRegRequester.send( OutgoingMessage( registration.dump().c_str() ) );
	
	// Wait for a reply
	IncomingMessage reply;
	
	cout << "Waiting on registration reply" << endl;
	
	m_geomuxRegRequester.receive( reply );
	
	json response = json::parse( string( reply.charData(), reply.size() ) );
	
	if( response[ "response"] != 1 )
	{
		throw std::runtime_error( "Camera registration denied!" );
	}
	else
	{
		cout << "Got registration reply" << endl;
	}
}

CStatusPublisher::~CStatusPublisher(){ cout << "Cleaning up CStatusPublisher" << endl; }

void CStatusPublisher::EmitStatus( const std::string &statusIn )
{
	// Create message
	json status = { { "status", statusIn } };
	
	m_statusPub.send( OutgoingMessage( "status" ), OutgoingMessage( status.dump().c_str() ) );
}

void CStatusPublisher::EmitError( const std::string &errorIn )
{
	// Create message
	json error = { { "error", errorIn } };
	
	m_statusPub.send( OutgoingMessage( "error" ), OutgoingMessage( error.dump().c_str() ) );
}

void CStatusPublisher::EmitChannelRegistration( uint32_t channelNumIn, const std::string &endpointPathIn, bool isOnlineIn )
{
	// Create message
	json message = 	{ 
						{ "chNum", channelNumIn },
						{ "endpoint", endpointPathIn },
						{ "status", isOnlineIn }
					};
	
	m_statusPub.send( OutgoingMessage( "channel_registration" ), OutgoingMessage( message.dump().c_str() ) );
}

void CStatusPublisher::EmitChannelSettings( const nlohmann::json &settingsIn )
{
	m_statusPub.send( OutgoingMessage( "channel_settings" ), OutgoingMessage( settingsIn.dump().c_str() ) );
}

void CStatusPublisher::EmitChannelHealthStats( const nlohmann::json &healthStatsIn )
{
	m_statusPub.send( OutgoingMessage( "channel_health" ), OutgoingMessage( healthStatsIn.dump().c_str() ) );
}

void CStatusPublisher::EmitChannelAPI( const nlohmann::json &apiIn )
{
	m_statusPub.send( OutgoingMessage( "channel_api" ), OutgoingMessage( apiIn.dump().c_str() ) );
}
