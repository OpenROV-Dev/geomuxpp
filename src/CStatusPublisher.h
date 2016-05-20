#pragma once

// Includes
#include <CpperoMQ/All.hpp>
#include <json.hpp>

class CStatusPublisher
{
public:
	// Methods
	CStatusPublisher( const std::string &cameraOffsetIn, CpperoMQ::Context *contextIn );
	virtual ~CStatusPublisher();
	
	void EmitStatus( const std::string &statusIn );
	void EmitError( const std::string &errorIn );
	
	void EmitSettings( const nlohmann::json &settingsIn );
	void EmitChannelRegistration( uint32_t channelNumIn, const std::string &endpointPathIn, bool isOnlineIn );
	void EmitChannelHealthStats( const nlohmann::json &healthStatsIn );

private:
	// Attributes
	std::string					m_cameraOffset;
	
	CpperoMQ::Context 			*m_pContext;
	CpperoMQ::PublishSocket 	m_statusPub;
	CpperoMQ::RequestSocket 	m_geomuxRegRequester;
	
};