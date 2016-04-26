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
	CGC6500( CpperoMQ::Context *contextIn, CpperoMQ::PublishSocket *geomuxStatusPubIn );
	virtual ~CGC6500();
	
	void SetDeviceOffset( const std::string &deviceOffsetIn );
	
	void HandleMessage( const nlohmann::json &commandIn );

private:	
	// Attributes
	std::string 									m_deviceOffset;
	std::vector<std::unique_ptr<CVideoChannel>> 	m_pChannels;
	
	CpperoMQ::PublishSocket 		*m_pGeomuxStatusPub;
	
	CpperoMQ::Context *m_pContext;
	
	// Methods
	void RegisterAPIMap();
	void CreateChannels();
	
	void EmitStatus( const std::string &statusIn );
	void EmitError( const std::string &errorIn );
};