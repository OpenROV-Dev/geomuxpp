#include "CCommandSubscriber.h"
#include <unistd.h>

using namespace std;
using namespace CpperoMQ;

using json = nlohmann::json;

CCommandSubscriber::CCommandSubscriber( const std::string &cameraOffsetIn, CpperoMQ::Context *contextIn )
	: m_cameraOffset( cameraOffsetIn )
	, m_pContext( contextIn )
	, m_geomuxCmdSub( m_pContext->createSubscribeSocket() )
{
	// Bind subscriber
	m_geomuxCmdSub.bind( std::string( "ipc:///tmp/geomux_command" + m_cameraOffset + ".ipc" ).c_str() );
	
	// Subscribe to anything (needs to be valid json to survive parsing)
	m_geomuxCmdSub.subscribe();
}

CCommandSubscriber::~CCommandSubscriber(){ cout << "Cleaning up CCommandSubscriber" << endl; }