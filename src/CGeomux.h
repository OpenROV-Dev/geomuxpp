#pragma once

// Includes
#include <CpperoMQ/All.hpp>

#include "CApp.h"
#include "CGC6500.h"

class CGeomux : public CApp
{
public:
	// Methods
	CGeomux( int argCountIn, char* argsIn[] );
	virtual ~CGeomux();

	virtual void Run();
	virtual void HandleSignal( int signalIdIn );

private:
	// Attributes
	CpperoMQ::Context 			m_context;
	CpperoMQ::PublishSocket 	m_geomuxStatusPub;
	CpperoMQ::SubscribeSocket 	m_geomuxCmdSub;
	
	CGC6500 m_gc6500;
	
	// Methods
	void Update();
	void HandleMessages();
	
	void EmitStatus( const std::string &statusIn );
	void EmitError( const std::string &errorIn );
	
	void Shutdown();
	void Restart();
};