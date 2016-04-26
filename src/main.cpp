// Includes
#include "Utility.h"
#include "CGeomux.h"

int main( int argc, char* argv[] )
{	
	bool restart = false;
	
	do
	{
		std::cout << "-------------------------" << std::endl;
		std::cout << "---------NEW RUN---------" << std::endl;

		try
		{
			// Create the application
			std::unique_ptr<CApp> app = util::make_unique<CGeomux>( argc, argv );
		
			// Run the application
			app->Run();
			
			restart = app->m_restart;
		}
		catch( const std::exception &e )
		{
			std::cerr << "Exception in main: " << e.what() << std::endl;
		}
		
		if( restart )
		{
			std::cout << "Restarting..." << std::endl;
		}
	}
	while( restart );

	return 0;
}