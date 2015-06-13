#ifndef TSF_FIT_RUNNER_H
#define TSF_FIT_RUNNER_H

// STL
#include <memory>
#include <map>
using namespace std;

// ROOBARB
#include "Logger.h"
#include "HistoAnalyzer.h"
#include "HistoBins.h"
using namespace jdb;

// Local
#include "TSF/FitSchema.h"
#include "TSF/Fitter.h"
#include "PhaseSpaceRecentering.h"




namespace TSF{
	class FitRunner : public HistoAnalyzer
	{
	protected:

		string centerSpecies;
		string psrMethod;
		double dedxSigmaIdeal, tofSigmaIdeal;
		PhaseSpaceRecentering * psr;

		shared_ptr<FitSchema> schema;

		HistoBins* binsPt;
		HistoBins* binsEta;
		HistoBins* binsCharge;

		unique_ptr<Reporter> zbReporter, zdReporter;

		vector<string> activePlayers;

		double zdSigFix = 0;

		map<string, vector<double> > zbSigMem;
		map< string, double> lockedZbSig;

	public:
		FitRunner( XmlConfig * _cfg, string _np  );

		~FitRunner();

		virtual void make();


		static string yieldName( string plc, int iCen, int charge, int iEta );
		static string sigmaName( string plc, int iCen, int charge, int iEta );
		static string muName( string plc, int iCen, int charge, int iEta );
		static string fitName( int iPt, int iCen, int charge, int iEta );


	protected:

		void makeHistograms();
		void fillFitHistograms(int iPt, int iCen, int iCharge, int iEta, double norm );
		void reportFit( Fitter * fitter, int iPt );
		void drawSet( string v, Fitter * fitter, int iPt );

		void prepare( double avgP, int iCen );
		void choosePlayers( double avgP, string plc, double roi );


		double p( double pt, double eta ){
			return pt * cosh( eta );
		}
		double averagePt( int ptBin ){
			if ( ptBin < 0 || ptBin > binsPt->nBins() ){
				return 0;
			}
			double avgPt = ((*binsPt)[ ptBin ] + (*binsPt)[ ptBin + 1 ]) / 2.0;
			return avgPt;
		}
		double averageP( int ptBin, int etaBin ){
			if ( ptBin < 0 || ptBin > binsPt->nBins() ){
				return 0;
			}
			if ( etaBin < 0 || etaBin > binsEta->nBins() ){
				return 0;
			} 

			double avgPt = ((*binsPt)[ ptBin ] + (*binsPt)[ ptBin + 1 ]) / 2.0;
			double avgEta = ((*binsEta)[ etaBin ] + (*binsEta)[ etaBin + 1 ]) / 2.0;

			return avgPt;//p( avgPt, avgEta );

		}

		double zbSigma( ){	
			return tofSigmaIdeal;
		}
		double zbMean( string plc, double p ){
			map< string, double> means = psr->centeredTofMap( centerSpecies, p );
			return means[ plc ];
		}
		double zdMean( string plc, double p ){
			map< string, double> means = psr->centeredDedxMap( centerSpecies, p );
			return means[ plc ];
		}
		double zdSigma( ){
			return dedxSigmaIdeal;
		}

	};

}




#endif