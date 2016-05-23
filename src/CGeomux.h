#pragma once

// Includes
#include <CpperoMQ/All.hpp>

#include "CApp.h"
#include "CCommandSubscriber.h"
#include "CGC6500.h"

class CGeomux : public CApp
{
public:
	// Methods
	CGeomux( int argCountIn, char* argsIn[] );
	virtual ~CGeomux();

	virtual void Run();

private:
	// Attributes
	CpperoMQ::Context 			m_context;
	
	std::string					m_cameraOffset;
	CCommandSubscriber			m_commandSubscriber;
	CGC6500 					m_gc6500;
	
	// Methods
	void Update();
	void HandleMessages();
	
	void Shutdown();
	void Restart();
};