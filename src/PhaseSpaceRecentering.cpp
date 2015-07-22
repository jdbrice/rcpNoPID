#include "PhaseSpaceRecentering.h"

PhaseSpaceRecentering::PhaseSpaceRecentering( double dedxSigma, double tofSigma, string bTable, int bMethod ){
	this->dedxSigma = dedxSigma;
	this->tofSigma = tofSigma;

	dedxGen = new Bichsel( bTable, bMethod);
	tofGen = new TofGenerator();

	species = { "Pi", "K", "P" };

	// in GeV / c^2
	piMass 		= 0.1395702;
	kaonMass 	= 0.493667;
	protonMass 	= 0.9382721;
}
PhaseSpaceRecentering::~PhaseSpaceRecentering(){
	delete dedxGen;
	delete tofGen;
}

TofGenerator * PhaseSpaceRecentering::tofGenerator() { return tofGen; }
Bichsel * PhaseSpaceRecentering::dedxGenerator() { return dedxGen; }

double PhaseSpaceRecentering::mass( string pType ){
	if ( "P" == pType )
		return protonMass;
	if ( "K" == pType )
		return kaonMass;
	if ( "Pi" == pType )
		return piMass;

	ERROR( " UNKOWN plc : " << pType )
	return -10.0;	
}
vector<string> PhaseSpaceRecentering::otherSpecies( string center ){
	vector<string> res;
	for ( int i = 0; i < species.size(); i++ ){
		if ( species[ i ] != center )
			res.push_back( species[ i ] );
	}
	return res;
}
vector<string> PhaseSpaceRecentering::allSpecies(){
	return species;
}
vector<double> PhaseSpaceRecentering::centeredTofMeans( string center, double p, vector<string> others ){

	double cMean = tofGen->mean( p, mass( center ) );
	
	vector<double> res;
	for ( int i = 0; i < others.size(); i++ ){
		double m = (tofGen->mean( p, mass( others[ i ] ) ) - cMean);
		res.push_back( m );
	}

	return res;
}
vector<double> PhaseSpaceRecentering::centeredTofMeans( string center, double p ){

	double cMean = tofGen->mean( p, mass( center ) );
	
	vector<double> res;
	for ( int i = 0; i < species.size(); i++ ){
		double m = (tofGen->mean( p, mass( species[ i ] ) ) - cMean);
		res.push_back( m );
	}

	return res;
}
map<string, double> PhaseSpaceRecentering::centeredTofMap( string center, double p ){

	double cMean = tofGen->mean( p, mass( center ) );
	
	map<string, double> res;
	for ( int i = 0; i < species.size(); i++ ){
		double m = (tofGen->mean( p, mass( species[ i ] ) ) - cMean);
		res[ species[ i ] ] = m;
	}

	return res;
}


vector<double> PhaseSpaceRecentering::centeredDedxMeans( string center, double p, vector<string> others ){
	
	const double cMean = dedxGen->meanLog( p, mass( center ), -1, 1000 );

	vector<double> res;
	for ( int i = 0; i < others.size(); i++ ){
		double m = dedxGen->meanLog( p, mass( others[ i ] ), -1, 1000 ) - cMean;
		res.push_back( m );
	}

	return res;
}
vector<double> PhaseSpaceRecentering::centeredDedxMeans( string center, double p ){
	
	const double cMean = dedxGen->meanLog( p, mass( center ), -1, 1000 );
	
	vector<double> res;
	for ( int i = 0; i < species.size(); i++ ){
		double m = dedxGen->meanLog( p, mass( species[ i ] ), -1, 1000 ) - cMean;
		res.push_back( m );
	}
	return res;
}

map<string, double> PhaseSpaceRecentering::centeredDedxMap( string center, double p ){
	
	const double cMean = dedxGen->meanLog( p, mass( center ), -1, 1000 );
	
	map<string, double> res;
	for ( int i = 0; i < species.size(); i++ ){
		double m = dedxGen->meanLog( p, mass( species[ i ] ), -1, 1000 ) - cMean;
		res[species[ i ] ] = m ;
	}
	return res;
}

double PhaseSpaceRecentering::rDedx( string centerSpecies, double dedx, double p ){ 

	double mean = dedxGen->meanLog( p, mass( centerSpecies ), -1, 1000 );
	double nDedx = log( dedx );
	double nSig = nDedx - mean ;

	return nSig;

	return -999.0;
}

double PhaseSpaceRecentering::nlDedx( string centerSpecies, double dedx, double p, double avgP ){

	double dedxLog = log( dedx );

	// mean for this species
	//double mu = dedxGen->meanLog( p, mass( centerSpecies ), -1, 1000 );
	const double muAvg = dedxGen->meanLog( avgP, mass( centerSpecies ), -1, 1000 );

	double n1 = 0;
	double d1 = 0;

	for ( int i = 0; i < species.size(); i++ ){

		const double iMu = dedxGen->meanLog( p, mass( species[ i ] ), -1, 1000 );
		const double iMuAvg = dedxGen->meanLog( avgP, mass( species[ i ] ), -1, 1000 );
		
		// may change to a functional dependance 
		double sigma = dedxSigma; 

		double iL = lh( dedxLog, iMu, sigma );
		double w = dedxLog + iMuAvg - iMu;
		

		n1 += (iL * w);
		d1 += iL;

	}

	if ( 0 == n1 || 0 == d1){
		return dedxLog - muAvg;
	}

	double nDedx = (n1/d1) - (muAvg);

	return nDedx;

}




double PhaseSpaceRecentering::rTof( string centerSpecies, double beta, double p  ){

	double deltaInvBeta = ( 1.0 / beta ) - tofGen->mean( p, mass( centerSpecies ) );

	return deltaInvBeta;
}

double PhaseSpaceRecentering::nlTof( string centerSpecies, double beta, double p, double avgP ){

	const double sigma = tofSigma; 
	const double tof = 1.0 / beta;
	
	// mean for this species
	//const double mu =  tofGen->mean( p, mass( centerSpecies ) );
	const double muAvg =  tofGen->mean( avgP, mass( centerSpecies ) );
	
	double n1 = 0;
	double d1 = 0;

	for ( int i = 0; i < species.size(); i++ ){
		
		double iMu =  tofGen->mean( p, mass( species[ i ] ) ) ;
		double iMuAvg =  tofGen->mean( avgP, mass( species[ i ] ) ) ;
		
		
		double iL = lh( tof, iMu, sigma );
		
		
		double w = tof + iMuAvg - iMu;
		
		n1 += (iL * w);
		d1 += iL;

	}

	if ( 0 == n1 || 0 == d1){
		return tof - muAvg;
	}
	
	double nTof = (n1/d1) - ( muAvg );
	
	return nTof;

}

double PhaseSpaceRecentering::nl2Tof( string centerSpecies, double beta, double p, double avgP ){

	const double sigma = tofSigma; 
	const double tof = 1.0 / beta;
	

	if ( 0 >= beta ){
		return -100;
	}
	// mean for this species
	//const double mu =  tofGen->mean( p, mass( centerSpecies ) );
	const double muAvg =  tofGen->mean( avgP, mass( centerSpecies ) );
	
	double n1 = 0;
	double d1 = 0;
	double n2 = 0;
	double d2 = 0;

	for ( int i = 0; i < species.size(); i++ ){
		
		double iMu =  tofGen->mean( p, mass( species[ i ] ) ) ;
		double iMuAvg =  tofGen->mean( avgP, mass( species[ i ] ) ) ;
		
		
		double iL = lh( tof, iMu, sigma );
		double iL2 = lh( muAvg, iMu, sigma );
		
		double w = tof + iMuAvg - iMu;
		
		n1 += (iL * w);
		d1 += iL;

		n2 += ( iL2 * w );
		d2 += iL2;

	}

	double p1 = 0;
	double p2 = 0;
	
	if ( d1 != 0 )
		p1 = n1 / d1;
	else 
		ERROR("oh no1 : " << tof)
	if ( d2 != 0 )
		p2 = n2 / d2;
	else
		ERROR("oh no2")

	double nTof = p1 - p2;
	
	return nTof;

}


double PhaseSpaceRecentering::lh( double x, double mu, double sigma ){

	double a = sigma * TMath::Sqrt( 2 * TMath::Pi() );
	double b = ( x - mu );
	double c = 2 * sigma*sigma;
	double d = (1/a) * TMath::Exp( -b*b / c );

	return d;
}