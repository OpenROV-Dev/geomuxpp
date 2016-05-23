#pragma once

// Includes
#include <CpperoMQ/All.hpp>
#include <json.hpp>

class CEventEmitter
{
public:
	// Methods
	CEventEmitter( CpperoMQ::Context *contextIn, const std::string &endpointIn );
	virtual ~CEventEmitter();
	
	void Emit( const char* eventIn, const nlohmann::json &messageIn );
	void Emit( const char* eventIn );
	
private:
	// Attributes	
	CpperoMQ::Context 			*m_pContext;
	CpperoMQ::PublishSocket 	m_pub;	
	
	std::string					m_endpoint;
};