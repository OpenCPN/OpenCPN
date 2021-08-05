///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Source code for "Creating Efficient Triangle Strips"
// (C) 2000, Pierre Terdiman (p.terdiman@wanadoo.fr)
//
// Version is 2.0.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "StripStdafx.h"

#include "Striper.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//																	Striper Class Implementation
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Striper::Striper() : mAdj(null), mTags(null), mStripLengths(null), mStripRuns(null), mSingleStrip(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Striper::~Striper()
{
	FreeUsedRam();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A method to free possibly used ram
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input	:	-
// Output	:	-
// Return	:	Self-reference
// Exception:	-
// Remark	:	-
Striper& Striper::FreeUsedRam()
{
	RELEASE(mSingleStrip);
	RELEASE(mStripRuns);
	RELEASE(mStripLengths);
	RELEASEARRAY(mTags);
	RELEASE(mAdj);
	return *this;
}

void ZeroMemory(void* addr, udword size);
void CopyMemory(void* dest, const void* src, udword size);
void FillMemory(void* dest, udword size, ubyte val);

void ZeroMemory(void* addr, udword size)
        {
                memset(addr, 0, size);
        }

void CopyMemory(void* dest, const void* src, udword size)
        {
                memcpy(dest, src, size);
        }

void FillMemory(void* dest, udword size, ubyte val)
        {
                memset(dest, val, size);
        }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A method to initialize the striper
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input	:	create,		the creation structure
// Output	:	-
// Return	:	true if success
// Exception:	-
// Remark	:	-
bool Striper::Init(STRIPERCREATE& create)
{
	// Release possibly already used ram
	FreeUsedRam();

	// Create adjacencies
	{
		mAdj = new Adjacencies;
		if(!mAdj)	return false;

		ADJACENCIESCREATE ac;
		ac.NbFaces	= create.NbFaces;
		ac.DFaces	= create.DFaces;
		ac.WFaces	= create.WFaces;
		bool Status = mAdj->Init(ac);
		if(!Status)	{ RELEASE(mAdj); return false; }

		Status = mAdj->CreateDatabase();
		if(!Status)	{ RELEASE(mAdj); return false; }

		mAskForWords		= create.AskForWords;
		mOneSided			= create.OneSided;
		mSGIAlgorithm		= create.SGIAlgorithm;
		mConnectAllStrips	= create.ConnectAllStrips;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A method to create the triangle strips
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input	:	result,		the result structure
// Output	:	-
// Return	:	true if success
// Exception:	-
// Remark	:	-
bool Striper::Compute(STRIPERRESULT& result)
{
	// You must call Init() first
	if(!mAdj)	return false;

	// Get some bytes
	mStripLengths			= new CustomArray;				if(!mStripLengths)	return false;
	mStripRuns				= new CustomArray;				if(!mStripRuns)		return false;
	mTags					= new bool[mAdj->mNbFaces];		if(!mTags)			return false;
	udword* Connectivity	= new udword[mAdj->mNbFaces];	if(!Connectivity)	return false;

	// mTags contains one bool/face. True=>the face has already been included in a strip
	ZeroMemory(mTags, mAdj->mNbFaces*sizeof(bool));

	// Compute the number of connections for each face. This buffer is further recycled into
	// the insertion order, ie contains face indices in the order we should treat them
	ZeroMemory(Connectivity, mAdj->mNbFaces*sizeof(udword));
	if(mSGIAlgorithm)
	{
		// Compute number of adjacent triangles for each face
		for(udword i=0;i<mAdj->mNbFaces;i++)
		{
			AdjTriangle* Tri = &mAdj->mFaces[i];
			if(!IS_BOUNDARY(Tri->ATri[0]))	Connectivity[i]++;
			if(!IS_BOUNDARY(Tri->ATri[1]))	Connectivity[i]++;
			if(!IS_BOUNDARY(Tri->ATri[2]))	Connectivity[i]++;
		}

		// Sort by number of neighbors
		RadixSorter RS;
		udword* Sorted = RS.Sort(Connectivity, mAdj->mNbFaces).GetIndices();

		// The sorted indices become the order of insertion in the strips
		CopyMemory(Connectivity, Sorted, mAdj->mNbFaces*sizeof(udword));
	}
	else
	{
		// Default order
		for(udword i=0;i<mAdj->mNbFaces;i++)	Connectivity[i] = i;
	}

	mNbStrips			= 0;	// #strips created
	udword TotalNbFaces	= 0;	// #faces already transformed into strips
	udword Index		= 0;	// Index of first face

	while(TotalNbFaces!=mAdj->mNbFaces)
	{
		// Look for the first face [could be optimized]
		while(mTags[Connectivity[Index]])	Index++;
		udword FirstFace = Connectivity[Index];

		// Compute the three possible strips from this face and take the best
		TotalNbFaces += ComputeBestStrip(FirstFace);

		// Let's wrap
		mNbStrips++;
	}

	// Free now useless ram
	RELEASEARRAY(Connectivity);
	RELEASEARRAY(mTags);

	// Fill result structure and exit
	result.NbStrips		= mNbStrips;
	result.StripLengths	= (udword*)	mStripLengths	->Collapse();
	result.StripRuns	=			mStripRuns		->Collapse();

	if(mConnectAllStrips)	ConnectAllStrips(result);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A method to compute the three possible strips starting from a given face
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input	:	face,		the first face
// Output	:	-
// Return	:	udword,		the #faces included in the strip
// Exception:	-
// Remark	:	mStripLengths and mStripRuns are filled with strip data
udword Striper::ComputeBestStrip(udword face)
{
	udword* Strip[3];		// Strips computed in the 3 possible directions
	udword* Faces[3];		// Faces involved in the 3 previous strips
	udword Length[3] = {0,0,0};		// Lengths of the 3 previous strips

	udword FirstLength[3];	// Lengths of the first parts of the strips are saved for culling
        udword jLook = 1;

	// Starting references
	udword Refs0[3];
	udword Refs1[3];
	Refs0[0] = mAdj->mFaces[face].VRef[0];
	Refs1[0] = mAdj->mFaces[face].VRef[1];

	// Bugfix by Eric Malafeew!
	Refs0[1] = mAdj->mFaces[face].VRef[2];
	Refs1[1] = mAdj->mFaces[face].VRef[0];

	Refs0[2] = mAdj->mFaces[face].VRef[1];
	Refs1[2] = mAdj->mFaces[face].VRef[2];

	// Compute jLook strips
	for(udword j=0;j<jLook;j++)
	{
		// Get some bytes for the strip and its faces
		Strip[j] = new udword[mAdj->mNbFaces+2+1+2];	// max possible length is NbFaces+2, 1 more if the first index gets replicated
		Faces[j] = new udword[mAdj->mNbFaces+2];
		FillMemory(Strip[j], (mAdj->mNbFaces+2+1+2)*sizeof(udword), 0xff);
		FillMemory(Faces[j], (mAdj->mNbFaces+2)*sizeof(udword), 0xff);

		// Create a local copy of the tags
		bool* Tags	= new bool[mAdj->mNbFaces];
		CopyMemory(Tags, mTags, mAdj->mNbFaces*sizeof(bool));

		// Track first part of the strip
		Length[j] = TrackStrip(face, Refs0[j], Refs1[j], &Strip[j][0], &Faces[j][0], Tags);

		// Save first length for culling
		FirstLength[j] = Length[j];
//		if(j==1)	FirstLength[j]++;	// ...because the first face is written in reverse order for j==1

		// Reverse first part of the strip
		for(udword i=0;i<Length[j]/2;i++)
		{
			Strip[j][i]				^= Strip[j][Length[j]-i-1];
			Strip[j][Length[j]-i-1]	^= Strip[j][i];
			Strip[j][i]				^= Strip[j][Length[j]-i-1];
		}
		for(udword i=0;i<(Length[j]-2)/2;i++)
		{
			Faces[j][i]				^= Faces[j][Length[j]-i-3];
			Faces[j][Length[j]-i-3]	^= Faces[j][i];
			Faces[j][i]				^= Faces[j][Length[j]-i-3];
		}

		// Track second part of the strip
		udword NewRef0 = Strip[j][Length[j]-3];
		udword NewRef1 = Strip[j][Length[j]-2];
		udword ExtraLength = TrackStrip(face, NewRef0, NewRef1, &Strip[j][Length[j]-3], &Faces[j][Length[j]-3], Tags);
		Length[j]+=ExtraLength-3;

		// Free temp ram
		RELEASEARRAY(Tags);
	}

	// Look for the best strip among the three
	udword Longest	= Length[0];
	udword Best		= 0;
	if(Length[1] > Longest)	{	Longest = Length[1];	Best = 1;	}
	if(Length[2] > Longest)	{	Longest = Length[2];	Best = 2;	}

	udword NbFaces = Longest-2;

	// Update global tags
	for(udword j=0;j<Longest-2;j++)	mTags[Faces[Best][j]] = true;

	// Flip strip if needed ("if the length of the first part of the strip is odd, the strip must be reversed")
	if(mOneSided && FirstLength[Best]&1)
	{
		// Here the strip must be flipped. I hardcoded a special case for triangles and quads.
		if(Longest==3 || Longest==4)
		{
			// Flip isolated triangle or quad
			Strip[Best][1] ^= Strip[Best][2];
			Strip[Best][2] ^= Strip[Best][1];
			Strip[Best][1] ^= Strip[Best][2];
		}
		else
		{
			// "to reverse the strip, write it in reverse order"
			for(udword j=0;j<Longest/2;j++)
			{
				Strip[Best][j]				^= Strip[Best][Longest-j-1];
				Strip[Best][Longest-j-1]	^= Strip[Best][j];
				Strip[Best][j]				^= Strip[Best][Longest-j-1];
			}

			// "If the position of the original face in this new reversed strip is odd, you're done"
			udword NewPos = Longest-FirstLength[Best];
			if(NewPos&1)
			{
				// "Else replicate the first index"
				for(udword j=0;j<Longest;j++)	Strip[Best][Longest-j] = Strip[Best][Longest-j-1];
				Longest++;
			}
		}
	}

	// Copy best strip in the strip buffers
	for(udword j=0;j<Longest;j++)
	{
		udword Ref = Strip[Best][j];
		if(mAskForWords)	mStripRuns->Store((uword)Ref);	// Saves word reference
		else				mStripRuns->Store(Ref);			// Saves dword reference
	}
	mStripLengths->Store(Longest);

	// Free local ram
	for(udword j=0;j<jLook;j++)
	{
		RELEASEARRAY(Faces[j]);
		RELEASEARRAY(Strip[j]);
	}

	// Returns #faces involved in the strip
	return NbFaces;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A method to extend a strip in a given direction, starting from a given face
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input	:	face,				the starting face
//				oldest, middle,		the two first indices of the strip == a starting edge == a direction
// Output	:	strip,				a buffer to store the strip
//				faces,				a buffer to store the faces of the strip
//				tags,				a buffer to mark the visited faces
// Return	:	udword,				the strip length
// Exception:	-
// Remark	:	-
udword Striper::TrackStrip(udword face, udword oldest, udword middle, udword* strip, udword* faces, bool* tags)
{
	udword Length = 2;														// Initial length is 2 since we have 2 indices in input
	strip[0] = oldest;														// First index of the strip
	strip[1] = middle;														// Second index of the strip

	bool DoTheStrip = true;
	while(DoTheStrip)
	{
		udword Newest = mAdj->mFaces[face].OppositeVertex(oldest, middle);	// Get the third index of a face given two of them
		strip[Length++] = Newest;											// Extend the strip,...
		*faces++ = face;													// ...keep track of the face,...
		tags[face] = true;													// ...and mark it as "done".

		ubyte CurEdge = mAdj->mFaces[face].FindEdge(middle, Newest);		// Get the edge ID...

		udword Link = mAdj->mFaces[face].ATri[CurEdge];						// ...and use it to catch the link to adjacent face.
		if(IS_BOUNDARY(Link))	DoTheStrip = false;							// If the face is no more connected, we're done...
		else
		{
			face = MAKE_ADJ_TRI(Link);										// ...else the link gives us the new face index.
			if(tags[face])	DoTheStrip=false;								// Is the new face already done?
		}
		oldest = middle;													// Shift the indices and wrap
		middle = Newest;
	}
	return Length;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A method to link all strips in a single one.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input	:	result,		the result structure
// Output	:	the result structure is updated
// Return	:	true if success
// Exception:	-
// Remark	:	-
bool Striper::ConnectAllStrips(STRIPERRESULT& result)
{
	mSingleStrip = new CustomArray;
	if(!mSingleStrip) return false;

	mTotalLength	= 0;
	uword* wrefs	= mAskForWords ? (uword*)result.StripRuns : null;
	udword* drefs	= mAskForWords ? null : (udword*)result.StripRuns;

	// Loop over strips and link them together
	for(udword k=0;k<result.NbStrips;k++)
	{
		// Nothing to do for the first strip, we just copy it
		if(k)
		{
			// This is not the first strip, so we must copy two void vertices between the linked strips
			udword LastRef	= drefs ? drefs[-1] : (udword)wrefs[-1];
			udword FirstRef	= drefs ? drefs[0] : (udword)wrefs[0];
			if(mAskForWords)	mSingleStrip->Store((uword)LastRef).Store((uword)FirstRef);
			else				mSingleStrip->Store(LastRef).Store(FirstRef);
			mTotalLength += 2;

			// Linking two strips may flip their culling. If the user asked for single-sided strips we must fix that
			if(mOneSided)
			{
				// Culling has been inverted only if mTotalLength is odd
				if(mTotalLength&1)
				{
					// We can fix culling by replicating the first vertex once again...
					udword SecondRef = drefs ? drefs[1] : (udword)wrefs[1];
					if(FirstRef!=SecondRef)
					{
						if(mAskForWords)	mSingleStrip->Store((uword)FirstRef);
						else				mSingleStrip->Store(FirstRef);
						mTotalLength++;
					}
					else
					{
						// ...but if flipped strip already begin with a replicated vertex, we just can skip it.
						result.StripLengths[k]--;
						if(wrefs)	wrefs++;
						if(drefs)	drefs++;
					}
				}
			}
		}

		// Copy strip
		for(udword j=0;j<result.StripLengths[k];j++)
		{
			udword Ref = drefs ? drefs[j] : (udword)wrefs[j];
			if(mAskForWords)	mSingleStrip->Store((uword)Ref);
			else				mSingleStrip->Store(Ref);
		}
		if(wrefs)	wrefs += result.StripLengths[k];
		if(drefs)	drefs += result.StripLengths[k];
		mTotalLength += result.StripLengths[k];
	}

	// Update result
	result.NbStrips		= 1;
	result.StripRuns	= mSingleStrip->Collapse();
	result.StripLengths	= &mTotalLength;

	return true;
}
