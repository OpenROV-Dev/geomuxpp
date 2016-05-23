#include "CEventEmitter.h"

using namespace std;
using namespace CpperoMQ;

using json = nlohmann::json;

CEventEmitter::CEventEmitter( CpperoMQ::Context *contextIn, const std::string &endpointIn )
	: m_pContext( contextIn )
	, m_pub( m_pContext->createPublishSocket() )
	, m_endpoint( endpointIn )
{
	m_pub.bind( m_endpoint.c_str() );
}

CEventEmitter::~CEventEmitter()
{ 
}

void CEventEmitter::Emit( const char* eventIn, const nlohmann::json &messageIn )
{
	m_pub.send( OutgoingMessage( eventIn ), OutgoingMessage( messageIn.dump().c_str() ) );
}

void CEventEmitter::Emit( const char* eventIn )
{
	m_pub.send( OutgoingMessage( eventIn ) );
}