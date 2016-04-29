#pragma once

// Includes
#include <CpperoMQ/All.hpp>
#include <json.hpp>

class CCommandSubscriber
{
public:
	// Methods
	CCommandSubscriber( const std::string &cameraOffsetIn, CpperoMQ::Context *contextIn );
	virtual ~CCommandSubscriber();
	
private:
	// Attributes
	std::string					m_cameraOffset;
	CpperoMQ::Context 			*m_pContext;
	
public:
	CpperoMQ::SubscribeSocket 	m_geomuxCmdSub;
	
};