#pragma once

// Includes
#include <mxuvc.h>
#include <json.hpp>

#include "CCameraRegistrar.h"

// Forward decs
class CVideoChannel;

// Defines
#define VIDEO_BACKEND "\"v4l2\""

class CGC6500
{
public:
	// Methods
	CGC6500( const std::string &cameraNameIn, CpperoMQ::Context *contextIn );
	virtual ~CGC6500();
	
	void HandleMessage( const nlohmann::json &commandIn );
	bool IsAlive();

private:	
	// Pointers
	CpperoMQ::Context 								*m_pContext;

	// Attributes
	bool											m_initialized = false;
	std::string 									m_cameraName;
	CCameraRegistrar								m_cameraRegistrar;
	
	std::vector<std::unique_ptr<CVideoChannel>> 	m_pChannels;
	
	// Methods
	void CreateChannels();
};