

#include "XmlConfig.h"
using namespace jdb;


#include <iostream>
#include "InclusiveSpectra.h"
#include "ParamSpectra.h"
#include "RefMultHelper.h"
#include "PidPhaseSpace.h"
#include "PidParamMaker.h"
#include "SimultaneousGaussians.h"

#include <exception>

int main( int argc, char* argv[] ) {

	if ( argc >= 2 ){

		try{
			XmlConfig config( argv[ 1 ] );
			//config.report();

			string fileList = "";
			string jobPrefix = "";

			if ( argc >= 4 ){
				fileList = argv[ 2 ];
				jobPrefix = argv[ 3 ];
			}

			string job = config.getString( "jobType" );

			if ( "RefMultHelper" == job ){
				RefMultHelper rmh( &config, "RefMultHelper." );
				rmh.eventLoop();
			} else if ( "InclusiveSpectra" == job ){
				InclusiveSpectra is( &config, "InclusiveSpectra.", fileList, jobPrefix );
				is.make();
			} else if ( "ParamSpectra" == job ){
				ParamSpectra ps( &config, "ParamSpectra.");
				ps.make();
			} else if ( "PidPhaseSpace" == job ){
				PidPhaseSpace pps( &config, "PidPhaseSpace.", fileList, jobPrefix  );
				pps.make();
			} else if ( "MakePidParams" == job ){
				PidParamMaker ppm( &config, "PidParamMaker." );
				ppm.make();
			} else if ( "SimultaneousGaussians" == job ){
				SimultaneousGaussians sg( &config, "SimultaneousGaussians." );
				sg.make();
			}

		} catch ( exception &e ){
			cout << e.what() << endl;
		}

	}
	return 0;
}
