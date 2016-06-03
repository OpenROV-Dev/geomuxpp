#include "CCameraRegistrar.h"

using namespace std;
using namespace CpperoMQ;

using json = nlohmann::json;

CCameraRegistrar::CCameraRegistrar( CpperoMQ::Context *contextIn )
	: m_pContext( contextIn )
	, m_req( m_pContext->createRequestSocket() )
{
	m_req.connect( "ipc:///tmp/geomux_registration.ipc" );
}

CCameraRegistrar::~CCameraRegistrar()
{ 
}

bool CCameraRegistrar::SendRequest( const nlohmann::json &requestIn )
{
	// Send a registration request
	m_req.send( OutgoingMessage( requestIn.dump().c_str() ) );

	IncomingMessage reply;
	
	// Wait for reply (blocks)
	m_req.receive( reply );
	
	// Parse reply
	json response = json::parse( string( reply.charData(), reply.size() ) );
	
	if( response[ "response" ] != 1 )
	{
		return false;
	}
	
	return true;
}

bool CCameraRegistrar::RegisterCamera( const std::string &cameraNameIn )
{
	json reg = 	{ 
					{ "type", "camera_registration" },
					{ "camera", cameraNameIn } 
				};
	
	return SendRequest( reg );
}

bool CCameraRegistrar::RegisterChannel( const std::string &cameraNameIn, uint32_t channelNumIn )
{
	json reg = 	{ 
					{ "type", "channel_registration" },
					{ "camera", cameraNameIn },
					{ "channel", channelNumIn }
				};
	
	return SendRequest( reg );
}

bool CCameraRegistrar::UnregisterCamera( const std::string &cameraNameIn )
{
	json unreg = 	{ 
						{ "type", "camera_unregistration" },
						{ "camera", cameraNameIn }
					};
	
	return SendRequest( unreg );
}