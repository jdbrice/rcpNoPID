#include "McMaker/TpcEffFitter.h"
#include "Common.h"

// ROOT
#include "TGraphAsymmErrors.h"
#include "TFile.h"
#include "TLatex.h"

#include "Logger.h"
#include "Reporter.h"
#include "RooPlotLib.h"
#include "FitConfidence.h"
using namespace jdb;


void TpcEffFitter::initialize(  ){

	DEBUG( classname(), "( " << config.getFilename() << ", " << nodePath << " )" )
	outputPath = config.getXString( nodePath + ".output:path", "" );
	
	INFO( classname(), "Opening HistoBook @ " << config.getXString( nodePath +  ".output.TFile:url", "TpcEff.root" ) );
	book = unique_ptr<HistoBook>( new HistoBook( config.getXString( nodePath +  ".output.TFile:url", "TpcEff.root" ), config, "", "" ) );
}



void TpcEffFitter::make(){
	DEBUG(classname(), "")

	RooPlotLib rpl;

	gStyle->SetOptFit( 111 );
	string params_file =  config.getXString( nodePath + ".output.params" );
	if ( "" == params_file ){
		ERROR( classname(), "Specifiy an output params file for the parameters" )
		return;
	}

	ofstream out( params_file.c_str() );

	out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
	out << "<config>" << endl;


	vector<string> labels = config.getStringVector( nodePath + ".CentralityLabels" );
	vector< int> cbins = config.getIntVector( nodePath + ".CentralityBins" );
	Reporter rp( config, nodePath + ".Reporter." );

	DEBUG( "Starting plc loop" )
	for ( string plc : Common::species ){
		if ( "E" == plc || "D" == plc )	// skip Electrons and Deuterons if I've got those in the list
			continue;

		for ( string c : Common::sCharges ){

			out << "\t<" << plc << "_" << c << ">" << endl;

			string fnMc = config.getXString( nodePath + ".input:url" ) + "TpcEff_" + plc + "_" + c + "_mc" + ".root";
			TFile * fmc = new TFile( fnMc.c_str(), "READ" );
			string fnRc = config.getXString( nodePath + ".input:url" ) + "TpcEff_" + plc + "_" + c + "_rc" + ".root";
			TFile * frc = new TFile( fnRc.c_str(), "READ" );

			DEBUG( classname(), "Mc File = " << fmc );
			DEBUG( classname(), "Rc File = " << frc );


			if ( !fmc->IsOpen() || !frc->IsOpen() )
				continue;
			

			// build an efficiency for each centrality
			for ( int b : cbins ){

				TH1 * hMc = (TH1*)fmc->Get( ("inclusive/pt_" + ts( b ) + "_" + c ).c_str() );
				TH1 * hRc = (TH1*)frc->Get( ("inclusive/pt_" + ts( b ) + "_" + c ).c_str() );

				INFO( classname(), "N bins MC = " << hMc->GetNbinsX() );
				INFO( classname(), "N bins RC = " << hRc->GetNbinsX() );

				// hMc->RebinX( 3 );
				// hRc->RebinX( 3 );

				for ( int i = 0; i <= hMc->GetNbinsX() + 1; i++ ){
					if ( hMc->GetBinContent( i ) < hRc->GetBinContent( i ) ){
						// set to 100%
						if ( i > 5 )
							hMc->SetBinContent( i, hRc->GetBinContent( i ) );
						else{
							hRc->SetBinContent( i, 0 );
							hMc->SetBinContent( i, 0 );
						}
					}
				}
					

				hMc->Sumw2();
				hRc->Sumw2();

				TGraphAsymmErrors g;

				g.SetName( (plc + "_" + c + "_" + ts(b)).c_str() );
				g.BayesDivide( hRc, hMc );

				INFO( classname(), "Adding TGraphAsymmErrors to book" );
				book->add( plc + "_" + c + "_" + ts(b),  &g );

				// do the fit
				TF1 * fitFunc = new TF1( "effFitFunc", "[0] * exp( - pow( [1] / x, [2] ) )", 0.0, 4.5 );
				fitFunc->SetParameters( .65, 0.05, 5.0, -0.05 );
				fitFunc->SetParLimits( 0, 0.5, 1.0 );
				fitFunc->SetParLimits( 1, 0.0, 0.5 );
				fitFunc->SetParLimits( 2, 0.0, 50 );

				// fist fit shape
				float min = config.getFloat( nodePath + ".Fit:min", 0.0 );
				float max = config.getFloat( nodePath + ".Fit:max", 2.0 );
				TFitResultPtr fitPointer = g.Fit( fitFunc, "RSWW", "", min, max );
				for ( int i = 0; i < config.getInt( nodePath + ".Fit:N", 1 ); i++ ){
					fitPointer = g.Fit( fitFunc, config.getXString( nodePath + ".Fit:opt", "RS" ).c_str(), "", min, max );
				}

				// fitFunc->FixParameter( 0, fitFunc->GetParameter( 0 ) );
				// fitFunc->SetParError( 0, 0.05 );
				
				// run the fit again to calculate uncertainties
				// fitPointer = g.Fit( fitFunc, "ERS", "", min, max );

				// ensure that uncertainty on efficiency is at least X%
				// MOVED TO Application
				// if ( fitFunc->GetParError( 0 ) < minP0Error ) {
				// 	INFO( classname(), "P0 uncertainty below threshold (" << (minP0Error * 100) << "%" );
				// 	INFO( classname(), "Setting P0 error to " << (minP0Error * 100) << "%" );
				// 	fitFunc->SetParError( 0, minP0Error );
				// }

				INFO( classname(), "FitPointer = " << fitPointer );
				TGraphErrors * bandSys = Common::choleskyBands( fitPointer, fitFunc, 200, 100, &rp );
				TGraphAsymmErrors * band = FitConfidence::fitUncertaintyBand( fitFunc, 0.05, 0.05, 100 );
				

				rp.newPage();
				rpl.style( &g ).set( "title", Common::plc_label( plc, c ) + " : " + labels[ b ] + ", Fit 68%CL (Blue Band), SysUnct (Red Band)" )
					.set( "yr", 0, 1.1 ).set( "optfit", 111 )
					.set( "xr", 0, 4.5 )
					.set("y", "Efficiency x Acceptance")
					.set( "x", "p_{T}^{MC} [GeV/c]" )
					.draw();

				TLatex latex;
				latex.SetTextSize(0.06);
				latex.SetTextAlign(13);  //align at top
				//[0] * exp( - pow( [1] / x, [2] ) )
				latex.DrawLatex(2,0.19,"#varepsilon(p_{T}) = p0 e^{ -(p1/p_{T})^{p2}}");

				INFO( classname(), "Stat Box" );
				gStyle->SetStatY( 0.5 );
				gStyle->SetStatX( 0.85 );

				gStyle->SetOptFit( 11 );
				
				fitFunc->SetLineColor( kRed );
				fitFunc->Draw("same");	

				INFO( classname(), "Drawing CL band" );
				
				band->SetFillColorAlpha( kRed, 0.7 );
				band->Draw( "same e3" );

				bandSys->SetFillColorAlpha( kBlue, 0.7 );
				bandSys->Draw( "same e3" );


				rp.savePage();
				string imgName = config.getXString( "env:cwd") + "/img/TpcEff_" + plc + "_" + c + "_" + ts(b) + ".png";
				INFO( classname(), "Exporting image to : " << imgName );
				rp.saveImage( imgName );

				imgName = config.getXString( "env:cwd") + "/img/TpcEff_" + plc + "_" + c + "_" + ts(b) + ".pdf";
				INFO( classname(), "Exporting image to : " << imgName );
				rp.saveImage( imgName );

				INFO( classname(), "Exporting Params" );
				exportParams( b, fitFunc, fitPointer, out );

			} // loop centrality bins

			out << "\t</" << plc << "_" << c << ">" << endl;

		} // loop on charge
	} // loop on plc

	out << "</config>" << endl;
	out.close();


}

void TpcEffFitter::exportParams( int bin, TF1 * f, TFitResultPtr result, ofstream &out ){
	INFO( classname(), "(bin=" << bin << ", f=" << f << ", fPtr=" << result << " )" )
	out << "\t\t<TpcEffParams bin=\"" << bin << "\" ";
	out << Common::toXml( f, result );
	out << "/>" << endl;
}