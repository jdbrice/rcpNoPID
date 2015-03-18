#ifndef REF_MULT_CORRECTION_H
#define REF_MULT_CORRECTION_H


/**
 * JDB
 */
#include "XmlConfig.h"
#include "ConfigRange.h"
#include "Logger.h"
#include "HistoBins.h"
using namespace jdb;
/**
 * ROOT
 */
#include "TMath.h"
#include "TRandom3.h"

class RefMultCorrection
{
protected:

	TRandom3 * rGen;
	Logger * logger;
	XmlConfig * cfg;

	int year;
	double energy;
	ConfigRange * zVertexRange;
	ConfigRange * runRange;
	HistoBins * centralityBins;
	vector<double> zParameters;
	vector<int> badRuns;



public:

	RefMultCorrection(string paramFile ){

		logger = new Logger();
		logger->setClassSpace( "RefMultCorrection" );

		logger->info(__FUNCTION__) << "Loading params from " << paramFile << endl;
		cfg = new XmlConfig( paramFile.c_str() );

		year 		= cfg->getInt( "year" );
		energy 		= cfg->getDouble( "energy" );
		zVertexRange= new ConfigRange( cfg, "zVertex" );
		runRange	= new ConfigRange( cfg, "runRange" );
		zParameters = cfg->getDoubleVector( "zParameters" );
		badRuns 	= cfg->getIntVector( "badRuns" );

		centralityBins = new HistoBins( cfg, "bins" );

		logger->info(__FUNCTION__) << "Year : " << year << endl;
		logger->info(__FUNCTION__) << "Energy : " << energy << endl;
		logger->info(__FUNCTION__) << "Z Vertex : " << zVertexRange->toString() << endl;
		logger->info(__FUNCTION__) << "Run Range : " << runRange->toString() << endl;
		logger->info(__FUNCTION__) << "Z Params : " ;
		for ( int i = 0; i < zParameters.size(); i++ )
			logger->info("", false) << zParameters[ i ] << " ";
		logger->info("", false) << endl;

		logger->debug( __FUNCTION__ ) << "Centrality Bins (80-75% -> 0-5%) : " << endl;
		logger->debug( __FUNCTION__ ) << centralityBins->toString() << endl;

		logger->debug( __FUNCTION__ ) << endl;
		logger->debug( __FUNCTION__ ) << "Centrality bin16 sanity" <<endl;
		logger->debug( __FUNCTION__ ) << "Centrality Bin for M=3 (-1) = " << bin16( 3 ) << endl;
		logger->debug( __FUNCTION__ ) << "Centrality Bin for M=8 (0)= " << bin16( 8 ) << endl;
		logger->debug( __FUNCTION__ ) << "Centrality Bin for M=240 (15) = " << bin16( 240 ) << endl;
		logger->debug( __FUNCTION__ ) << "Centrality Bin for M=600 (-2)= " << bin16( 600 ) << endl;
		logger->debug( __FUNCTION__ ) << "Centrality bin9 sanity" <<endl;
		logger->debug( __FUNCTION__ ) << "Centrality Bin for M=3 (-1)= " << bin9( 3 ) << endl;
		logger->debug( __FUNCTION__ ) << "Centrality Bin for M=8 (0)= " << bin9( 8 ) << endl;
		logger->debug( __FUNCTION__ ) << "Centrality Bin for M=25 (2)= " << bin9( 25 ) << endl;
		logger->debug( __FUNCTION__ ) << "Centrality Bin for M=240 (8) = " << bin9( 240 ) << endl;
		logger->debug( __FUNCTION__ ) << "Centrality Bin for M=600 (-2)= " << bin9( 600 ) << endl;
		logger->debug( __FUNCTION__ ) << endl;

		logger->debug( __FUNCTION__ ) << "Bad Run Sanity Check" << endl;
		logger->debug( __FUNCTION__ ) << "Run 15047004 is Bad (BAD): " << isBad(15047004) << endl;
		logger->debug( __FUNCTION__ ) << "Run 15057050 is Bad (GOOD): " << isBad(15057050) << endl;
		logger->debug( __FUNCTION__ ) << endl;

		rGen = new TRandom3( );

	}
	~RefMultCorrection(){

		delete zVertexRange;
		delete runRange;
		delete cfg;
		delete logger;
	}


	bool isBad( int runIndex ){

		vector<int>::iterator it = std::find( badRuns.begin(), badRuns.end(), runIndex );
		if ( it == badRuns.end() )
			return false;
		return true;

	}
	int bin16( int m, double z = -1000 ){

		if ( z < -999 ){ // corrected refMult - just use
			int bin = centralityBins->findBin( m );
			return bin;
		} else if ( z >= zVertexRange->min && z <= zVertexRange->max ){
			int bin = centralityBins->findBin( refMult( m, z ) );
			return bin;
		}
		return -1;
	}
	int bin9( int m, double z = -1000 ){
		int preBin = bin16( m, z );
		if ( preBin < 0 )
			return preBin;
		if ( preBin < 14 )
			return (preBin / 2); // integer divide and floor
		else if ( 14 == preBin )
			return 7;
		else if ( 15 == preBin )
			return 8;
		return -1;
	}
	int refMult( int rawRefMult, double z ){

		if ( z < zVertexRange->min || z > zVertexRange->max )
			return rawRefMult;

		double rmZ = zPolEval( z );
		if ( rmZ > 0 && zParameters.size() >= 1 ){

			const double center = zParameters[ 0 ];
			double corRefMult = center / rmZ;

			double refMultRnd = rawRefMult + rGen->Rndm();

			return TMath::Nint( refMultRnd * corRefMult );
		}

		logger->warn( __FUNCTION__ ) << "Ref Mult not corrected " << endl;
		return rawRefMult;
	}

	double zPolEval( double z ){
		if ( zParameters.size() <= 0 ){
			logger->warn(__FUNCTION__) << "No Z Parameters " << endl;
			return 0;
		}

		double r = 0;
		//r = zParameters[ 0 ] + zParameters[ 1 ] * z;// + zParameters[ 2 ] * z * z + zParameters[ 3 ] * z * z * z + zParameters[ 4 ] * z * z * z * z + zParameters[ 5 ] * z*z*z*z*z
		for ( int i = 0; i < zParameters.size(); i++ ){
			r += zParameters[ i ] * TMath::Power( z, i );
		}
		return r;
	}
	
};


#endif