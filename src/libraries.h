#ifndef __LIBRARIES_H
	#define __LIBRARIES_H

	// lualib.lib genesisd.lib 
	#pragma comment(lib, "lualib.lib")
	#pragma message("Linking with lualib.lib")

	// lualib.lib genesis.lib
	#ifdef NDEBUG
		#pragma comment (lib, "GenesisR.lib")
		#pragma message("Linking with GenesisR.lib" )

//		#pragma comment (lib, "coldetcvr.lib")
//		#pragma message("Linking with ColdetCV-R.lib" )
	#else
		#pragma comment (lib, "GenesisD.lib")
		#pragma message("Linking with GenesisD.lib")

//		#pragma comment (lib, "coldetcvd.lib")
//		#pragma message("Linking with ColdetCV-D.lib" )
	#endif

#endif