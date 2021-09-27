///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Source code for "Creating Efficient Triangle Strips"
// (C) 2000, Pierre Terdiman (p.terdiman@wanadoo.fr)
//
// Version is 2.0.
//
// This is a versatile and customized import/export array class I use for a long time.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Notes	:
//
// CustomArray is a special array class especially designed for import/export goals, even if you can use it in many other ways.
// Here are some basic usages and code examples:
//
// 1) Saving heterogeneous data to disk
//
//	Say you want to create a chunk-based file (e.g. a .3DS one). Such files may contain a chunk type (e.g. "FACE", i.e. a string) followed by
//	chars or ints (e.g. vertex references) or floats (e.g. vertices). Saving such a file may be painful, and is usually done with an ugly list
//	of putc/fputc. It also can be done with a single fwrite, but you must have stored your heterogeneous data beforehand, hence usually dealing
//	with multiple pointers (e.g. a char*, an int*, a float*...) Well, that's ugly. With the CustomArray class you can solve this problem in an
//	elegant way, with the following code:
//
//		CustomArray	Array;				// constructor does all the painful stuff for you, destructor frees everything as well
//
//		// Store heterogeneous data in any order...
//		Array.Store((char)1);
//		Array.Store(10.0f);
//		Array.Store((long)1);
//		Array.Store("Test");
//
//		// ...and just save to disk
//		Array.ExportToDisk("c:\\datas.bin");
//
//	Saved file is a linear binary file, made of following bytes:
//	- 01
//	- 10.0 in float
//	- 00 00 00 01 (or 01 00 00 00, depends on your computer's little/big endian way of dealing with data)
//	- the string "Test" (not ended by a null character)
//
//	And that's all. Very easy to use, and you don't have to know how many bytes you'll need. This is very useful when packing a file for example,
//	as explained in the second example.
//
//
// 2) Packing a buffer
//
//	Say you want to encode a buffer with an arithmetic coder, and your encoder gives you one packed byte at a time. You don't know the final packed size,
//	but you don't need it if you use a CustomArray.
//
//		CustomArray	Array;
//		int EncodedByte;
//
//		do
//		{
//			EncodedByte = EncodeSymbol();						// Get next packed byte or EOF (-1)
//			if(EncodedByte!=EOF)	Array.Store(EncodedByte);	// Save packed byte
//		}while(EncodedByte!=EOF);
//
//		Array.ExportToDisk("c:\\packed.bin");					// Save packed file
//
//
//	You even can store bits in your CustomArray, with the StoreBit method. Call the StoreBit method 8 times, and a new byte will be added to the array.
//	If you call StoreBit, say 6 times, and then call a standard Store method, 2 extra bits are automatically added to padd the address on a byte boundary.
//
//
//	Now let's say you want your saved file to begin with the size of the packed data. (you could take the file's length, but imagine the packed data
//	is part of a bigger file) As you don't know the final packed size before actually having packed it, this is painful: you must leave room for the
//	final packed size at the start of the file, and fill it at the end of the process. If you save your file with some putc/fputc this is a real pain.
//	The CustomArray class has a neat way of dealing with this problem: the PushAddress and PopAddressAndStore methods. Let's modify the example code:
//
//		CustomArray	Array;
//		int EncodedByte;
//		long PackedSize = 0;
//
//		Array.Store((long)0).PushAddress();						// Leave room for a long, save the address
//
//		do
//		{
//			EncodedByte = EncodeSymbol();						// Get next packed byte or EOF (-1)
//			if(EncodedByte!=EOF)	Array.Store(EncodedByte);	// Save packed byte
//			if(EncodedByte!=EOF)	PackedSize++;				// Update packed size
//		}while(EncodedByte!=EOF);
//
//		Array.PopAddressAndStore(PackedSize);					// Store packed size where the PushAddress has been done.
//
//		Array.ExportToDisk("c:\\packed.bin");					// Save packed file
//
//	Of course you can push more than one address (as many as you want):
//
//		Array.Store((long)0).PushAddress();						// Push address 1 on the stack
//		Array.Store((long)0).PushAddress();						// Push address 2 on the stack
//		Array.Store((long)0).PushAddress();						// Push address 3 on the stack
//		...
//		Array.Store((long)0).PushAddress();						// Push address N on the stack
//
//		Array.PopAddressAndStore((long)1);						// Pop address N and store value 1
//		...
//		Array.PopAddressAndStore((long)N-2);					// Pop address 3 and store value N-2
//		Array.PopAddressAndStore((long)N-1);					// Pop address 2 and store value N-1
//		Array.PopAddressAndStore((long)N);						// Pop address 1 and store value N
//
//
// 3) Creating a log file
//
//	The StoreASCII methods provide a very easy way to create log files (e.g. for debug purpose). Example:
//
//		Array.StoreASCII(10.0f);		// Store the string "10.0"
//		Array.StoreASCII((long)1234);	// Store the string "1234"
//		Array.StoreASCII(Boolean);		// Store the string "true" or "false", depends on the boolean value
//
//		Say "Score" is an unsigned long whose value is 500:
//		Array.StoreASCII("Your score is ").StoreASCII(Score).StoreASCII(".\n");		// Store the string "Your score is 500." (followed by a carriage return)
//
//		...and so on...
//
//
// 4) Getting the data back
//
//	This is easily done with the Collapse method, which creates a linear buffer filled with all stored data.
//
//
// 5) Importing data
//
//	You can initialize a CustomArray with your own buffer, or even with a file:
//
//		CustomArray	Array("c:\\data.bin");		// Read the whole file data.bin, fill the array with it
//
//		MyByte	= Array.GetByte();				// Get the first byte of the array, move the inner pointer one byte beyond
//		MyFloat	= Array.GetFloat();				// Get the next 4 bytes of the array, read as a float, move the inner pointer 4 bytes beyond
//		...
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// History	:		01/15/99: first version for Irion MAX Plug-In
//					02/04/99: ASCII support added
//					02/05/99: GetOffset(), Padd(), LinkTo(CustomArray* array) added
//					03/xx/99: Collapse() added, self-references added
//					03/xx/99: BOOL definition added in header if not defined yet, addies-stack mecanism added
//					04/xx/99: "push" renamed to "store", for a more coherent name since people were expecting a "pop" method to be used.............
//					04/xx/99: BOOL handled as a long. Use bool (in lower case) for a real boolean value.
//					05/xx/99: heap size is now 4Kb, and allocated ram is doubled for each new block. The more you eat, the more ram you're given.
//					09/xx/99: bit-storage methods added (StoreBit, EndBits) for data-compression
//					29/10/99: bug fixed: mLastAddress was not initialized. Was crashing if the first thing done was a PushAddress. Well, a PushAddress should NOT
//							  be the first thing done anyway!
//					04/11/99: address stack is now resizable.
//					06/01/00: local memory manager and local error codes removed, CustomArray class added to IrionBasics
//
// More notes:		- always keep in mind that an CustomArray eats CUSTOMARRAY_BLOCKSIZE bytes when initialized, even if you don't use it later.
//					  That's why you may use this class for very specific reasons, or even change this #define to match your own needs.
//
//					- I know I could've used templates.
//
//					- THIS IS NOT THREAD-SAFE.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __CUSTOMARRAY_H__
#define __CUSTOMARRAY_H__

#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MACROS

// Define BOOL if needed
#ifndef BOOL
typedef int	BOOL;
#endif

#define CUSTOMARRAY_BLOCKSIZE	(4*1024)		// 4 Kb => heap size


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//																					CustomArray Class Definition
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CustomArray
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Structures and enums
	struct CustomBlock{
		CustomBlock()		{ Addy = null; }
		~CustomBlock()		{ RELEASEARRAY(Addy); }
		void*				Addy;						// Stored data
		unsigned long		Size;						// Length of stored data
		unsigned long		Max;						// Heap size
	};

	struct CustomCell{
		CustomCell()		{ NextCustomCell = null; }

		struct CustomCell*	NextCustomCell;
		CustomBlock			Item;
	};

private:
	CustomCell*				mCurrentCell;				// Current block cell
	CustomCell*				mInitCell;					// First block cell

	void*					mCollapsed;					// Possible collapsed buffer
	void**					mAddresses;					// Stack to store addresses
	void*					mLastAddress;				// Last address used in current block cell
	unsigned short			mNbPushedAddies;			// #saved addies
	unsigned short			mNbAllocatedAddies;			// #allocated addies
	unsigned char			mBitCount;
	unsigned char			mBitMask;

	// Management methods
	CustomArray&			CheckArray(unsigned long bytesneeded);
	CustomArray&			NewBlock(CustomCell* previouscell, unsigned long size=0);
	bool					SaveCell(CustomCell* p, FILE* fp);
	CustomArray&			StoreASCIICode(char code);
	// Helpers
	udword					FileSize(const char* name);
public:
	// Constructor / destructor
	CustomArray(unsigned long startsize=CUSTOMARRAY_BLOCKSIZE, void* inputbuffer=null);
	CustomArray(const char* filename);
	~CustomArray();

	// Store methods
	CustomArray&			StoreBit(bool bit);
	CustomArray&			EndBits();

	CustomArray&			Store(BOOL Bo);
	CustomArray&			Store(bool Bo);
	CustomArray&			Store(char b);
	CustomArray&			Store(unsigned char b);
	CustomArray&			Store(short w);
	CustomArray&			Store(unsigned short w);
	CustomArray&			Store(long d);
	CustomArray&			Store(unsigned long d);
//	CustomArray&			Store(int d);
	CustomArray&			Store(unsigned int d);
	CustomArray&			Store(float f);
	CustomArray&			Store(double f);
	CustomArray&			Store(const char* String);

	CustomArray&			StoreASCII(BOOL Bo);
	CustomArray&			StoreASCII(bool Bo);
	CustomArray&			StoreASCII(char b);
	CustomArray&			StoreASCII(unsigned char b);
	CustomArray&			StoreASCII(short w);
	CustomArray&			StoreASCII(unsigned short w);
	CustomArray&			StoreASCII(long d);
	CustomArray&			StoreASCII(unsigned long d);
//	CustomArray&			StoreASCII(int d);
	CustomArray&			StoreASCII(unsigned int d);
	CustomArray&			StoreASCII(float f);
	CustomArray&			StoreASCII(double f);
	CustomArray&			StoreASCII(const char* String);

	// Management methods
	bool					ExportToDisk(const char* FileName);
	bool					ExportToDisk(FILE* fp);

	unsigned long			GetOffset();
	CustomArray&			Padd();
	CustomArray&			LinkTo(CustomArray* array);
	void*					GetAddress()	{ char* CurrentAddy = (char*)mCurrentCell->Item.Addy; CurrentAddy+=mCurrentCell->Item.Size; return CurrentAddy; }
	void*					Collapse(void* userbuffer=null);

	// Address methods
	bool					PushAddress();
	CustomArray&			PopAddressAndStore(BOOL Bo);
	CustomArray&			PopAddressAndStore(bool Bo);
	CustomArray&			PopAddressAndStore(char b);
	CustomArray&			PopAddressAndStore(unsigned char b);
	CustomArray&			PopAddressAndStore(short w);
	CustomArray&			PopAddressAndStore(unsigned short w);
	CustomArray&			PopAddressAndStore(long d);
	CustomArray&			PopAddressAndStore(unsigned long d);
//	CustomArray&			PopAddressAndStore(int d);
	CustomArray&			PopAddressAndStore(unsigned int d);
	CustomArray&			PopAddressAndStore(float f);
	CustomArray&			PopAddressAndStore(double f);

	// Read methods
	char					GetByte();
	short					GetWord();
	long					GetDword();
	float					GetFloat();
};

#endif // __CUSTOMARRAY_H__
