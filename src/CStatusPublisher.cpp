#include "CStatusPublisher.h"

using namespace std;
using namespace CpperoMQ;

using json = nlohmann::json;

CStatusPublisher::CStatusPublisher( CpperoMQ::Context *contextIn )
	: m_pContext( contextIn )
	, m_publisher( m_pContext->createPublishSocket() )
{
	m_publisher.bind( "ipc:///tmp/geomux_status.ipc" );
}

CStatusPublisher::~CStatusPublisher(){ cout << "Cleaning up CStatusPublisher" << endl; }

void CStatusPublisher::EmitStatus( const std::string &statusIn )
{
	// Create message
	json status = { { "status", statusIn } };
	
	m_publisher.send( OutgoingMessage( "status" ), OutgoingMessage( status.dump().c_str() ) );
}

void CStatusPublisher::EmitError( const std::string &errorIn )
{
	// Create message
	json error = { { "error", errorIn } };
	
	m_publisher.send( OutgoingMessage( "error" ), OutgoingMessage( error.dump().c_str() ) );
}

void CStatusPublisher::EmitChannelRegistration( uint32_t channelNumIn, const std::string &endpointPathIn, bool isOnlineIn )
{
	// Create message
	json message = 	{ 
						{ "chNum", channelNumIn },
						{ "endpoint", endpointPathIn },
						{ "status", isOnlineIn }
					};
					
	cout << "Channel reg: " << message.dump().c_str() << endl;
	
	m_publisher.send( OutgoingMessage( "channel_registration" ), OutgoingMessage( message.dump().c_str() ) );
}

void CStatusPublisher::EmitSettings( const nlohmann::json &settingsIn )
{
	cout << "Settings: " << settingsIn.dump().c_str() << endl;
	
	m_publisher.send( OutgoingMessage( "channel_settings" ), OutgoingMessage( settingsIn.dump().c_str() ) );
}
