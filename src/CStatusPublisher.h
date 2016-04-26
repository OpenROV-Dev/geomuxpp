#pragma once

// Includes
#include <CpperoMQ/All.hpp>
#include <json.hpp>

class CStatusPublisher
{
public:
	// Methods
	CStatusPublisher( CpperoMQ::Context *contextIn );
	virtual ~CStatusPublisher();
	
	void EmitStatus( const std::string &statusIn );
	void EmitError( const std::string &errorIn );
	void EmitSettings( const nlohmann::json &settingsIn );
	void EmitChannelRegistration( uint32_t channelNumIn, const std::string &endpointPathIn, bool isOnlineIn );

private:
	// Attributes
	CpperoMQ::Context 			*m_pContext;
	CpperoMQ::PublishSocket 	m_publisher;
};