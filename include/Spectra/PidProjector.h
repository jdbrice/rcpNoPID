#ifndef PID_PROJECTOR_H
#define PID_PROJECTOR_H


// ROOBARB
#include "Utils.h"
#include "Logger.h"

using namespace jdb;

// ROOT
#include "TH2D.h"
#include "TH1D.h"
#include "TNtuple.h"
#include "TCut.h"

class PidProjector {
protected:

	// bin width
	double _zbBinWidth, _zdBinWidth;
	TFile * _f;

	TCut _deuteronCut;
	TCut _electronCut;
	double _zbCutMax = 1000;

public:
	static constexpr auto tag = "PidProjector";
	
	PidProjector( TFile * inFile, double zbBinWidth, double zdBinWidth ){
		INFO( tag, "( file=" << inFile << ", zbBinWidth=" << zbBinWidth << ", zdBinWidth=" << zdBinWidth <<" )" );
		_f = inFile;
		_zbBinWidth = zbBinWidth;
		_zdBinWidth = zdBinWidth;

		_deuteronCut = "";
		_electronCut = "";
	}

	~PidProjector(){
		INFO( tag, "()" );
	}

	string path( string name ){
		return "PidPoints/" + name;
	}

	void cutDeuterons( double protonCenter, double protonSigma, float nSigma = 3 ){
		
		// less than because we want to keep what isn't deuterons
		string cutstr = "zb - " + dts(protonCenter) + " <= " + dts( protonSigma * nSigma );
		INFO( tag, "Deuteron Cut : " << cutstr );
		TCut cut = cutstr.c_str(); 
		_deuteronCut = cut;

		_zbCutMax = protonCenter + protonSigma * nSigma;
	}

	void cutElectrons( 	double zb_Pi, double zd_Pi, double zb_Pi_sigma, double zd_Pi_sigma, 
						double zb_K, double zd_K, double zb_K_sigma, double zd_K_sigma, 
						double zb_E, double zd_E, double zb_E_sigma, double zd_E_sigma, 
						double nSigma_Pi, double nSigma_K, double nSigma_E ){

		TCut cut_Pi = ("sqrt( ((zb - "+dts(zb_Pi)+") / " + dts(zb_Pi_sigma) + ")^2 + ((zd - "+dts(zd_Pi)+") / " + dts(zd_Pi_sigma) + ")^2 ) < " + dts( nSigma_Pi )).c_str();
		TCut cut_K = ("sqrt( ((zb - "+dts(zb_K)+") / " + dts(zb_K_sigma) + ")^2 + ((zd - "+dts(zd_K)+") / " + dts(zd_K_sigma) + ")^2 ) < " + dts( nSigma_K )).c_str();
		TCut cut_E = ("sqrt( ((zb - "+dts(zb_E)+") / " + dts(zb_E_sigma) + ")^2 + ((zd - "+dts(zd_E)+") / " + dts(zd_E_sigma) + ")^2 ) > " + dts( nSigma_E )).c_str();

		_electronCut = cut_Pi || cut_K || cut_E;
	}

	TH2D * project2D( string name, string cut = "" ){

		TNtuple * data = (TNtuple*)_f->Get( path(name).c_str() );
		if ( !data ){
			ERROR( tag, "ubable to open " << name );
			return new TH2D( "err", "err", 1, 0, 1, 1, 0, 1 );
		}

		double zbMax = data->GetMaximum( "zb" );
		double zdMax = data->GetMaximum( "zd" );

		INFO( tag, "Max from tree zbMax = " << zbMax );
		if ( zbMax > _zbCutMax )
			zbMax = _zbCutMax;

		double zbMin = data->GetMinimum( "zb" );
		double zdMin = data->GetMinimum( "zd" );

		int zbNBins = (int) ((zbMax - zbMin) / _zbBinWidth + 0.5 );
		int zdNBins = (int) ((zdMax - zdMin) / _zdBinWidth + 0.5 );

		INFO( tag, "Projecting " << name << " in 2D " ) ;
		INFO ( tag, "zb from ( " << zbMin <<", " << zbMax << " ) / " << _zbBinWidth << ", = " << zbNBins << " bins" << ")" );
		INFO ( tag, "zd from ( " << zdMin <<", " << zdMax << " ) / " << _zdBinWidth << ", = " << zdNBins << " bins" << ")" );

		TCut allCuts = _deuteronCut && _electronCut && TCut( cut.c_str() );
		allCuts = allCuts * TCut( "w" ); // apply the track weight

		INFO( tag, "Cut string : " << allCuts );

		TH2D * h = new TH2D( (name + "_2D").c_str(), (name + "_2D").c_str(), zdNBins, zdMin, zdMax, zbNBins, zbMin, zbMax );
		h->GetDirectory()->cd();
		string hist = name + "_2D";

		data->Draw( ("zb:zd >> " + hist).c_str(), allCuts, "colz"  );

		return h;
	}

	TH1D * project1D( string name, string var, string cut = "" ){
		TNtuple * data = (TNtuple*)_f->Get( path(name).c_str() );
		if ( !data ){
			ERROR( tag, "ubable to open " << name );
			return new TH1D( "err", "err", 1, 0, 1 );
		}

		double max = data->GetMaximum( var.c_str() );
		double min = data->GetMinimum( var.c_str() );

		if ( "zb" == var && max > _zbCutMax )
			max = _zbCutMax + (_zbCutMax - min) * .10;
		
		double binWidth = _zbBinWidth;
		if ( "zd" == var )
			binWidth = _zdBinWidth;

		int nBins = (int) ((max - min) / binWidth + 0.5 );
		

		string hist = name + "_" + var + "_1D";
		TH1D * h = new TH1D( hist.c_str(), hist.c_str(), nBins, min, max );
		h->GetDirectory()->cd();

		INFO( tag, "Projecting " << name << " in 1D on " << var << " from ( " << min <<", " << max << " ) / " << binWidth << ", = " << nBins << " bins" << ")" )

		TCut allCuts = cut.c_str();
		if ( "zd" == var )
			allCuts = allCuts && _deuteronCut;

		TCut wCut = "w";
		allCuts = allCuts * wCut;
		INFO( tag, "Cut string : " << allCuts );

		data->Draw( ( var + " >>" + hist).c_str(),  allCuts );


		INFO( tag, "Integral(h) = " << h->Integral() );

		return h;
	}

	TH1D * projectEnhanced( string name, string var, string plc, double cl, double cr ){
		TNtuple * data = (TNtuple*)_f->Get( path(name).c_str() );
		if ( !data ){
			ERROR( tag, "unable to open " << name );
			return new TH1D( "err", "err", 1, 0, 1 );
		}

		// we cut on the variable we aren't projecting
		string cutVar = "zb";
		if ( "zb" == var )
			cutVar = "zd";

		string cut = cutVar + " < " + dts( cr ) + " && " + cutVar + " > " + dts( cl );

		// include deuteron cut if needed
		TCut allCuts = cut.c_str();
		if ( "zd" == var )
			allCuts = allCuts && _deuteronCut;

		double max = data->GetMaximum( var.c_str() );
		double min = data->GetMinimum( var.c_str() );

		if ( "zb" == var && max > _zbCutMax )
			max = _zbCutMax + (_zbCutMax - min) * .10;
		
		double binWidth = _zbBinWidth;
		if ( "zd" == var )
			binWidth = _zdBinWidth;
		int nBins = (int) ((max - min) / binWidth + 0.5 );
	
		string hist = name + "_" + var + "_1D_" + plc;
		TH1D * h = new TH1D( hist.c_str(), hist.c_str(), nBins, min, max );	
		h->Sumw2();	

		INFO( tag, "Projecting " << name << " in 1D enhanced around " << plc << " on " << var << " from ( " << min <<", " << max << " ) / " << binWidth << ", = " << nBins << " bins" << ")" )
		INFO( tag, "Cutting on " << cutVar << "( " << cl << ", " << cr << " )" );

		data->Draw( ( var + " >>" + hist).c_str(), allCuts );
		return h;
	}





};
#endif
