#include "CApp.h"

CApp::CApp( int argCountIn, char* argsIn[] )
{
	// Add all arguments to a vector of strings for easy access
	for( int i = 0; i < argCountIn; ++i )
	{
		m_arguments.push_back( argsIn[ i ] );
	}
}

CApp::~CApp()
{
}