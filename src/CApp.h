#pragma once

// Includes
#include <vector>

#include "Utility.h"

class CApp
{
public:
	// Attributes
	std::vector<std::string> m_arguments;

	// Methods
	CApp( int argCountIn, char* argsIn[] );
	virtual ~CApp();

	// Pure virtuals - You must implement these in your application class
	virtual void Run() = 0;
	virtual void HandleSignal( int signalIdIn ) = 0;
};