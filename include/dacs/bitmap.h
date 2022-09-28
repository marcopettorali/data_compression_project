#ifndef BITMAP_H_
#define BITMAP_H_

#endif /*BITMAP_H_*/

#include <malloc.h>
#include <stdio.h>

#include "basics.h"

typedef struct sbitmap
   { uint *data;
     uint n;        // # of bits
     uint pop;	    // # bits set
     uint pop0;	    // # bits not set
     uint *sdata;   // superblock counters for 1
     //uint *sdata0;  // superblock counters for 0
     uint sSize;	//		size of sdata vector
     byte *bdata;   // block counters for 1
     //byte *bdata0;  // block counters for 0
     uint bSize; 	//     size of bdata vector
     uint mem_usage;
   } *bitmap;

  // In theory, we should have superblocks of size s=log^2 n divided into
  // blocks of size b=(log n)/2. This takes 
  // O(n log n / log^2 n + n log log n / log n + log n sqrt n log log n) bits
  // In practice, we can have any s and b, and the needed amount of bits is
  // (n/s) log n + (n/b) log s + b 2^b log b bits
  // Optimizing it turns out that s should be exactly s = b log n
  // Optimizing b is more difficult but could be done numerically.
  // However, the exponential table does no more than popcounting, so why not
  // setting up a popcount algorithm tailored to the computer register size,
  // defining that size as b, and proceeding.

//unsigned char OnesInByte[] = 

uint popcount (register uint x);

  
/******************************************************************/
// FUNCIONS DE EDU ...
/******************************************************************/
/* 
	Creates a bitmap and structures to rank and select 
*/

//bitmap createBitmapEdu (uint *string, uint n){  return createBitmap(string,n);}

bitmap createBitmap (uint *string, uint n);


/*
  Number of 1s in range [0,posicion]
*/
//uint rank1Edu(bitmap B, unsigned int position) {
//uint rank1Edu(bitmap B, unsigned int position) { return rank(B,position);}
uint rank(bitmap B, unsigned int position);

/********************************************************************************************/
/**************************************************************************************/

static uint binsearch (uint *data, uint size, uint val);

uint bselect (bitmap B, uint j);
uint bselect0 (bitmap B, uint j);

// destroys the bitmap, freeing the original bitstream
void destroyBitmap (bitmap B);
   
// Prints the bit vector
void showBitVector(uint *V, uint vectorSize);
  
void saveBitmap (char *filename, bitmap b);
/* loads the Rank structures from disk, and sets Bitmap->data ptr to "string"
*/
bitmap loadBitmap (char *filename, uint *string, uint n);
   
/********************************************************************************************/
/********************************************************************************************/




	// creates a bitmap structure from a bitstring, which is shared

bitmap createBitmapGONZA (uint *string, uint n);

	// rank(i): how many 1's are there before position i, not included

//uint rank (bitmap B, uint i)
uint rankGONZA (bitmap B, uint i);
