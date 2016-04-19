#pragma once

// Includes
#include <zmq.hpp>

#include "CApp.h"
#include "CGeoCam.h"
#include "CMuxer.h"


class CGeomux : public CApp
{
public:
	// Attributes
	bool 			m_quitApplication;
	
	CGeoCam 		m_geocam;
	CMuxer			m_muxer;
	
	

	// Methods
	CGeomux( int argCountIn, char* argsIn[] );
	virtual ~CGeomux();

	virtual void Run();
	virtual void HandleSignal( int signalIdIn );
	
	void Update();
};