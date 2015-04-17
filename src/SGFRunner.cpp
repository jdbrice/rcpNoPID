#include "SGFRunner.h"
#include "PidPhaseSpace.h"
#include "SGF.h"

SGFRunner::SGFRunner( XmlConfig * _cfg, string _np) 
: HistoAnalyzer( _cfg, _np ){
	
	// Initialize the Phase Space Recentering Object
	tofSigmaIdeal = cfg->getDouble( nodePath+"PhaseSpaceRecentering.sigma:tof", 0.0012);
	dedxSigmaIdeal = cfg->getDouble( nodePath+"PhaseSpaceRecentering.sigma:dedx", 0.06);
	psr = new PhaseSpaceRecentering( dedxSigmaIdeal,
									 tofSigmaIdeal,
									 cfg->getString( nodePath+"Bichsel.table", "dedxBichsel.root"),
									 cfg->getInt( nodePath+"Bichsel.method", 0) );
	psrMethod = cfg->getString( nodePath+"PhaseSpaceRecentering.method", "traditional" );
	// alias the centered species for ease of use
	centerSpecies = cfg->getString( nodePath+"PhaseSpaceRecentering.centerSpecies", "K" );

	/**
	 * Make the momentum transverse, eta, charge binning
	 */
	binsPt = new HistoBins( cfg, "bin.pt" );
	binsEta = new HistoBins( cfg, "bin.eta" );
	binsCharge = new HistoBins( cfg, "bin.charge" );

	/**
	 * Setup the PID Params
	 */
	if ( cfg->exists( nodePath + "TofPidParams"  ) ){
		useParams = true;
		for ( string plc : PidPhaseSpace::species ){

			logger->info(__FUNCTION__) << "Preparing PID params for : " << plc << endl;

			tofParams[ plc ] = unique_ptr<TofPidParams>(new TofPidParams( cfg, nodePath + "TofPidParams." + plc + "." ));
			dedxParams[ plc ] = unique_ptr<DedxPidParams>( new DedxPidParams( cfg, nodePath + "DedxPidParams." + plc + "." ));

		}
	} else {
		useParams = false;
	}

	

}

SGFRunner::~SGFRunner(){


}


void SGFRunner::makeHistograms(){

	/**
	 * The bins to fit over
	 */
	vector<int> centralityFitBins = cfg->getIntVector( nodePath + "FitRange.centralityBins" );
	vector<int> etaFitBins = cfg->getIntVector( nodePath + "FitRange.etaBins" );
	vector<int> chargeFit = cfg->getIntVector( nodePath + "FitRange.charges" );

	book->cd();
	book->makeAll( nodePath + "histograms" );
	for ( int iCen : centralityFitBins ){
		for ( int iCharge : chargeFit ){
			for ( int iEta : etaFitBins ){
				
				for ( string plc : PidPhaseSpace::species ){
					// yield Histos
					book->cd( plc + "_yield" );
					string name = yieldName( plc, iCen, iCharge, iEta );
					//"yield_" + plc + "_" + ts(iCen) + "_" + PidPhaseSpace::chargeString( iCharge ) + "_" + ts(iEta);
					book->clone( "/", "yield", plc+"_yield", name );

					// Mean Histos
					book->cd( plc + "_zbMu" );
					name = muName( plc, iCen, iCharge, iEta );
					//"mu_" + plc + "_" + ts(iCen) + "_" + PidPhaseSpace::chargeString( iCharge ) + "_" + ts(iEta);
					book->clone( "/", "yield", plc+"_zbMu", name );

					book->cd( plc + "_zdMu" );
					name = muName( plc, iCen, iCharge, iEta );
					//"mu_" + plc + "_" + ts(iCen) + "_" + PidPhaseSpace::chargeString( iCharge ) + "_" + ts(iEta);
					book->clone( "/", "yield", plc+"_zdMu", name );

					// Sigma Histos
					book->cd( plc + "_zbSigma" );
					name = sigmaName( plc, iCen, iCharge, iEta );
					//"sigma_" + plc + "_" + ts(iCen) + "_" + PidPhaseSpace::chargeString( iCharge ) + "_" + ts(iEta);
					book->clone( "/", "yield", plc+"_zbSigma", name );
					book->cd( plc + "_zdSigma" );
					name = sigmaName( plc, iCen, iCharge, iEta );
					//"sigma_" + plc + "_" + ts(iCen) + "_" + PidPhaseSpace::chargeString( iCharge ) + "_" + ts(iEta);
					book->clone( "/", "yield", plc+"_zdSigma", name );
				} // loop plc
			} // loop iEta
		} // loop iCharge
	} // loop iCen


}

void SGFRunner::make(){

	RooMsgService::instance().setGlobalKillBelow(ERROR);

	//Constraints on the mu 	 
	double zbDeltaMu = cfg->getDouble( nodePath + "ParameterFixing.deltaMu:zb", 1.5 );
	double zdDeltaMu = cfg->getDouble( nodePath + "ParameterFixing.deltaMu:zd", 1.5 );
	
	//Constraints on the sigma  
	double zbDeltaSigma = cfg->getDouble( nodePath + "ParameterFixing.deltaSigma:zb", 0.25 );
	double zdDeltaSigma = cfg->getDouble( nodePath + "ParameterFixing.deltaSigma:zd", 0.25 );

	//The bins to fit over
	vector<int> centralityFitBins = cfg->getIntVector( nodePath + "FitRange.centralityBins" );
	vector<int> etaFitBins = cfg->getIntVector( nodePath + "FitRange.etaBins" );
	vector<int> chargeFit = cfg->getIntVector( nodePath + "FitRange.charges" );

	// Make the histograms for storing the results
	makeHistograms();

	int firstPtBin = cfg->getInt( nodePath + "FitRange.ptBins:min", 0 );
	int lastPtBin = cfg->getInt( nodePath + "FitRange.ptBins:max", 1 );
	if ( lastPtBin >= binsPt->nBins() )
		lastPtBin = binsPt->nBins() - 1;

	 for ( int iCen : centralityFitBins ){
		for ( int iCharge : chargeFit ){
			for ( int iEta : etaFitBins ){
						
				/**
				 * Create the schema and fitter
				 */
				schema = shared_ptr<SGFSchema>(new SGFSchema( cfg, nodePath + "SGFSchema" ));
				SGF sgf( schema, inFile );

				for ( int iPt = firstPtBin; iPt <= lastPtBin; iPt++ ){

					double avgP = averageP( iPt, iEta );
					
					logger->info(__FUNCTION__) << "pt Bin : " << iPt << endl;
					logger->info(__FUNCTION__) << "Cen Bin : " << iCen << endl;
					logger->info(__FUNCTION__) << "Charge Bin : " << iCharge << endl;
					logger->info(__FUNCTION__) << "Eta Bin : " << iEta << endl;
					

					for ( string plc : PidPhaseSpace::species ){

						// zb Parameters
						double zbMu = zbMean( plc, avgP );
						double zbSig = zbSigma( plc, avgP );

						// zd Parameters
						double zdMu = zdMean( plc, avgP );
						double zdSig = zdSigma( plc, avgP );

						// update the schema
						logger->trace(__FUNCTION__) << plc << " : zb mu=" << zbMu << ", sig=" << zbSig << endl;
						logger->trace(__FUNCTION__) << plc << " : zd mu=" << zdMu << ", sig=" << zdSig << endl; 
						
						schema->setInitial( "zb", plc, zbMu, zbSig, zbDeltaMu, zbDeltaSigma );
						schema->setInitial( "zd", plc, zdMu, zdSig, zdDeltaMu, zdDeltaSigma );

						// check if the sigmas should be fixed
						double zbMinParP = cfg->getDouble( nodePath + "ParameterFixing." + plc + ":zbSigma", 5.0 );
						double zdMinParP = cfg->getDouble( nodePath + "ParameterFixing." + plc + ":zdSigma", 5.0 );

						if ( zbMinParP > 0 && avgP >= zbMinParP)
							schema->fixSigma( "zb", plc, zbSig );
						if ( zdMinParP > 0 && avgP >= zdMinParP )
							schema->fixSigma( "zd", plc, zdSig );
					} // loop on plc to set config

					sgf.fit( centerSpecies, iCharge, iCen, iPt, iEta );
				
					sgf.report( reporter );

					fillFitHistograms(iPt, iCen, iCharge, iEta );

				}// loop pt Bins
			} // loop eta bins
		} // loop charge bins
	} // loop centrality bins

}


void SGFRunner::fillFitHistograms(int iPt, int iCen, int iCharge, int iEta ){

	for ( string plc : PidPhaseSpace::species ){

		int iiPt = iPt + 1;

		// Yield
		string name = yieldName( plc, iCen, iCharge, iEta );
		book->cd( plc+"_yield");
		double sC = schema->var( "yield_"+plc )->getVal() / book->get( name )->GetBinWidth( iiPt );
		double sE = schema->var( "yield_"+plc )->getError() / book->get( name )->GetBinWidth( iiPt );
		
		book->get( name )->SetBinContent( iiPt, sC );
		book->get( name )->SetBinError( iiPt, sE );

		//Mu
		name = muName( plc, iCen, iCharge, iEta );
		book->cd( plc+"_zbMu");
		sC = schema->var( "zb_mu_"+plc )->getVal();
		sE = schema->var( "zb_mu_"+plc )->getError();
		
		book->get( name )->SetBinContent( iiPt, sC );
		book->get( name )->SetBinError( iiPt, sE );

		name = muName( plc, iCen, iCharge, iEta );
		book->cd( plc+"_zdMu");
		sC = schema->var( "zd_mu_"+plc )->getVal();
		sE = schema->var( "zd_mu_"+plc )->getError();
		
		book->get( name )->SetBinContent( iiPt, sC );
		book->get( name )->SetBinError( iiPt, sE );

		//Sigma
		name = sigmaName( plc, iCen, iCharge, iEta );;
		book->cd( plc+"_zbSigma");
		sC = schema->var( "zb_sigma_"+plc )->getVal();
		sE = schema->var( "zb_sigma_"+plc )->getError();
		
		book->get( name )->SetBinContent( iiPt, sC );
		book->get( name )->SetBinError( iiPt, sE );

		name = sigmaName( plc, iCen, iCharge, iEta );
		book->cd( plc+"_zdSigma");
		sC = schema->var( "zd_sigma_"+plc )->getVal();
		sE = schema->var( "zd_sigma_"+plc )->getError();
		
		book->get( name )->SetBinContent( iiPt, sC );
		book->get( name )->SetBinError( iiPt, sE );

	}
	// just makes it so that the RooFit Frames aren't saved into HistoBook File
	inFile->cd();
}



string SGFRunner::yieldName( string plc, int iCen, int charge, int iEta ){
	return "yield_" + plc + "_" + ts(iCen) + "_" + PidPhaseSpace::chargeString( charge ) + "_" + ts(iEta);
}
string SGFRunner::sigmaName( string plc, int iCen, int charge, int iEta ){
	return "sigma_" + plc + "_" + ts(iCen) + "_" + PidPhaseSpace::chargeString( charge ) + "_" + ts(iEta);
}
string SGFRunner::muName( string plc, int iCen, int charge, int iEta ){
	return "mu_" + plc + "_" + ts(iCen) + "_" + PidPhaseSpace::chargeString( charge ) + "_" + ts(iEta);
}



