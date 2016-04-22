#pragma once

// Includes
#include <mxuvc.h>
#include <libmxcam.h>
#include <zmq.hpp>
#include <CpperoMQ/All.hpp>
#include <json.hpp>

#include "CVideoChannel.h"

// Defines
#define VIDEO_BACKEND "\"v4l2\""

class CGC6500
{
public:
	// Methods
	CGC6500( CpperoMQ::PublishSocket *geomuxStatusPubIn );
	virtual ~CGC6500();
	
	void SetDeviceOffset( const std::string &deviceOffsetIn );
	
	void Initialize();
	void Cleanup();
	
	void HandleMessage( const nlohmann::json &commandIn );

private:	
	// Attributes
	std::string 					m_deviceOffset;
	std::vector<CVideoChannel> 		m_channels;
	
	CpperoMQ::PublishSocket 		*m_pGeomuxStatusPub;
	
	// Methods
	void RegisterAPIMap();
	void CreateChannels();
	
	void EmitStatus( const std::string &statusIn );
	void EmitError( const std::string &errorIn );
};