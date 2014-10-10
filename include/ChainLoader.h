
#ifndef CHAINLOADER_H
#define CHAINLOADER_H

#include "dirent.h"
#include "allroot.h"

namespace jdb{

	class ChainLoader{

	public:
		//static void load( TChain * chain, char* ntdir, uint maxFiles = 1000 );
		static void load( TChain * chain, const char* ntdir, uint maxFiles = 1000 );
	};


}


#endif

