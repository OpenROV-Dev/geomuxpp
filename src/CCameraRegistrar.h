#pragma once

// Includes
#include <CpperoMQ/All.hpp>
#include <json.hpp>

class CCameraRegistrar
{
public:
	// Methods
	CCameraRegistrar( CpperoMQ::Context *contextIn );
	virtual ~CCameraRegistrar();
	
	bool RegisterCamera( const std::string &cameraNameIn );
	bool RegisterChannel( const std::string &cameraNameIn, uint32_t channelNumIn );
	
	bool UnregisterCamera( const std::string &cameraNameIn );
	
private:
	// Attributes	
	CpperoMQ::Context 			*m_pContext;
	CpperoMQ::RequestSocket 	m_req;
	
	bool SendRequest( const nlohmann::json &requestIn );
};