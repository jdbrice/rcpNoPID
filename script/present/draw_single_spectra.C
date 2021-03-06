
#include "common.C"

#ifndef DRAW_SINGLE_SPECTRA_C
#define DRAW_SINGLE_SPECTRA_C

#include "SpectraLoader.h"
#include "Utils.h"
#include "Logger.h"


#include <sys/stat.h>

#include "RooPlotLib.h"

 bool file_exists (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

string file_name( string energy, string plc, string charge, string iCen ){
	string base = "/Users/danielbrandenburg/bnl/local/data/RcpAnalysis/spectra/";

	// for easy comparison
	// if ( "14.5" == energy )
	// 	base = "/Users/danielbrandenburg/bnl/local/data/RcpAnalysis/spectra/deepti/compare/";
		// base ="/Users/danielbrandenburg/bnl/local/data/RcpAnalysis/spectra/deepti_wo_fdc/";

	return base + "spectra_" + energy + "_" + plc + "_" + charge + "_" + iCen + ".dat";

}

string plc_label( string plc, string charge ){
	if ( "Pi" == plc && "p" == charge )
		return "#pi^{+}";
	if ( "Pi" == plc && "n" == charge )
		return "#pi^{-}";
	if ( "K" == plc && "p" == charge )
		return "K^{+}";
	if ( "K" == plc && "n" == charge )
		return "K^{-}";
	if ( "P" == plc && "p" == charge )
		return "p";
	if ( "P" == plc && "n" == charge )
		return "#bar{p}";
	return "";
}

int nPtBins = 28;
double ptBins[] = { 
0.0,
0.1,
0.5,
0.6, 
0.7, 
0.8, 
0.9, 
1.0, 
1.1, 
1.2, 
1.3, 
1.4, 
1.5, 
1.6, 
1.7, 
1.8, 
1.9, 
2.0	, 
2.2	, 
2.4	, 
2.6	, 
2.8	,
3.0 , 
3.5, 
4.5, 
5.0,
5.7, 
6.0, 
6.8 };

TH1 * normalize_binning( TH1 * in ){
	INFO( "Normalizing the bins for " << in->GetName() );
	string name = in->GetName();
	name += + "_normed";
	TH1 * out = new TH1D( name.c_str(), "", nPtBins, ptBins );


	DEBUG( "Input has " << in->GetNbinsX() << " bins" );
	for ( int i = 1; i <= in->GetNbinsX(); i++ ){
		out->SetBinContent( i + 2, in->GetBinContent( i ) );
		out->SetBinError( i + 2, in->GetBinError( i ) );
	}

	DEBUG( "returing normalized hitograms " << name  );
	return out;
}


TH1* draw_single_spectra( 	string energy = "14.5", 
							string plc="Pi", string charge="p", 
							string iCen="0",
							int color = kRed, 
							string draw_opt = "", 
							double scaler = 1.0 ){


	Logger::setGlobalLogLevel( Logger::llAll );
	gStyle->SetOptStat( 0 );

	INFO( "Got COLOR " << color );

	string fn = file_name( energy, plc, charge, iCen );
	if ( !file_exists( fn ) && iCen != "3"){
		return new TH1D( "err", "", nPtBins, ptBins );
	} else if ( !file_exists( fn ) && iCen == "3" ){
		fn = file_name( energy, plc, charge, "3+4" );
	}

	INFO( "Loading " << fn );
	SpectraLoader sl( fn );

	TH1* sys = normalize_binning( sl.sysHisto( fn + "_sys" ) );
	TH1* stat = normalize_binning( sl.statHisto( fn + "_stat" ) );

	sys->Scale( scaler );
	stat->Scale( scaler );

	RooPlotLib rpl;


	rpl.style( sys )
		.set( "title", plc_label( plc, charge ) + " ; pT [GeV/c]; dN^{2} / ( N_{evt} 2 #pi pT dpT dy )    " )
		.set( "color", color, 0.5 ).set( "markerstyle", 8 )
		.set( "draw", draw_opt + " e2" ).set( "yto", 2 )
		.draw();

	rpl.style( stat ).set( "color", color, 0.5 ).set( "markerstyle", 8 )
		.set( "draw", "same" )
		.draw();
	

	gPad->SetLogy(1);

	return sys;

}

#endif