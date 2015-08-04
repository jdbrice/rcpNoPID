#ifndef MC_MAKER_ENERGY_LOSS_H
#define MC_MAKER_ENERGY_LOSS_H

#include <fstream>
using namespace std;

// RcpMaker
#include "Spectra/InclusiveSpectra.h"

// Root
#include "TF1.h"

class EnergyLoss : public InclusiveSpectra
{
public:
	EnergyLoss( XmlConfig * config, string nodePath, string fileList ="", string jobPrefix ="" );
	~EnergyLoss() {}

	virtual void preEventLoop();
	virtual void postEventLoop();

	void analyzeTrack( int iTrack );

	void exportParams( int cbin, TF1 * f, ofstream &out );
	
};



#endif