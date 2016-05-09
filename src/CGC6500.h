#pragma once

// Includes
#include <geocamera/mxuvc.h>
#include <json.hpp>

// Forward decs
class CStatusPublisher;
class CVideoChannel;

// Defines
#define VIDEO_BACKEND "\"v4l2\""

class CGC6500
{
public:
	// Methods
	CGC6500( const std::string &deviceOffsetIn, CpperoMQ::Context *contextIn, CStatusPublisher *publisherIn );
	virtual ~CGC6500();
	
	void HandleMessage( const nlohmann::json &commandIn );

private:	
	// Attributes
	std::string 									m_deviceOffset;
	std::vector<std::unique_ptr<CVideoChannel>> 	m_pChannels;
	
	bool											m_gc6500Initialized = false;
	
	CpperoMQ::Context 								*m_pContext;
	CStatusPublisher 								*m_pStatusPublisher;
	
	// Methods
	void CreateChannels();
};