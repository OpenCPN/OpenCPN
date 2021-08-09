///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Source code for "Creating Efficient Triangle Strips"
// (C) 2000, Pierre Terdiman (p.terdiman@wanadoo.fr)
//
// Version is 2.0.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __STRIPER_H__
#define __STRIPER_H__

#include <stdint.h>

#include "StripStdafx.h"

//typedef uint32_t udword;   // DWORD = unsigned 32 bit value
//typedef uint16_t uword;    // WORD = unsigned 16 bit value
//typedef uint8_t ubyte;     // BYTE = unsigned 8 bit value

//#define null nullptr

//#include "Adjacency.h"
//#include "CustomArray.h"

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//																Class Striper
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct STRIPERCREATE{
				STRIPERCREATE()
				{
					DFaces				= null;
					WFaces				= null;
					NbFaces				= 0;
					AskForWords			= true;
					OneSided			= true;
					SGIAlgorithm		= true;
					ConnectAllStrips	= false;
				}
				udword					NbFaces;			// #faces in source topo
				udword*					DFaces;				// list of faces (dwords) or null
				uword*					WFaces;				// list of faces (words) or null
				bool					AskForWords;		// true => results are in words (else dwords)
				bool					OneSided;			// true => create one-sided strips
				bool					SGIAlgorithm;		// true => use the SGI algorithm, pick least connected faces first
				bool					ConnectAllStrips;	// true => create a single strip with void faces
	};

	struct STRIPERRESULT{
				udword					NbStrips;			// #strips created
				udword*					StripLengths;		// Lengths of the strips (NbStrips values)
				void*					StripRuns;			// The strips in words or dwords, depends on AskForWords
				bool					AskForWords;		// true => results are in words (else dwords)
	};

	class Striper
	{
	private:
				Striper&				FreeUsedRam();
				udword					ComputeBestStrip(udword face);
				udword					TrackStrip(udword face, udword oldest, udword middle, udword* strip, udword* faces, bool* tags);
				bool					ConnectAllStrips(STRIPERRESULT& result);

				Adjacencies*			mAdj;				// Adjacency structures
				bool*					mTags;				// Face markers

				udword					mNbStrips;			// The number of strips created for the mesh
				CustomArray*			mStripLengths;		// Array to store strip lengths
				CustomArray*			mStripRuns;			// Array to store strip indices

				udword					mTotalLength;		// The length of the single strip
				CustomArray*			mSingleStrip;		// Array to store the single strip

				// Flags
				bool					mAskForWords;
				bool					mOneSided;
				bool					mSGIAlgorithm;
				bool					mConnectAllStrips;

	public:
				Striper();
				~Striper();

				bool					Init(STRIPERCREATE& create);
				bool					Compute(STRIPERRESULT& result);
	};

#endif // __STRIPER_H__
