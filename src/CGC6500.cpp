// Includes
#include "CGC6500.h"

CGC6500::CGC6500( CpperoMQ::PublishSocket *geomuxStatusPubIn )
	: m_deviceOffset( "0" )
	, m_pGeomuxStatusPub( geomuxStatusPubIn )
{
	
}

CGC6500::~CGC6500(){}

void CGC6500::SetDeviceOffset( const std::string &deviceOffsetIn )
{
	m_deviceOffset = deviceOffsetIn;
}

void CGC6500::Initialize()
{
	
}

void CGC6500::Cleanup()
{
	
}
	
void CGC6500::RegisterAPIMap()
{
	
}

void CGC6500::CreateChannels()
{
	
}

void CGC6500::HandleMessage( const nlohmann::json &commandIn )
{
	
}
void CGC6500::EmitStatus( const std::string &statusIn )
{
	
}
void CGC6500::EmitError( const std::string &errorIn )
{
	
}
