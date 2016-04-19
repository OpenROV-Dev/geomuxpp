// Includes
#include <signal.h>

#include "Utility.h"
#include "easylogging.hpp"
#include "CGeomux.h"

// Initialize Easylogger
INITIALIZE_EASYLOGGINGPP

// Globals
namespace global
{
	std::unique_ptr<CApp> app;
}

// Prototypes
void SignalHandler( int signalId );
void InitializeLogger();


int main( int argc, char* argv[] )
{
	// Initialize the logger
	InitializeLogger();

	LOG( INFO ) << "-------------------------";
	LOG( INFO ) << "---------NEW RUN---------";

	try
	{
		// Create the app instance
		global::app = util::make_unique<CGeomux>( argc, argv );

		// Register a handler for SIGINT
		signal( SIGINT, SignalHandler );

		// Run the application
		global::app->Run();
	}
	catch( const std::exception &e )
	{
		LOG( ERROR ) << "Exception in main: " << e.what();
	}

	return 0;
}

void SignalHandler( int signalIdIn )
{
	// Call the application's signal handler
	global::app->HandleSignal( signalIdIn );
}

void InitializeLogger()
{
	// Specifiy the location of the configuration file
	el::Configurations config( "config/logger.conf" );

	// Reconfigure the logger
	el::Loggers::reconfigureAllLoggers( config );
}