
// Implements operations over a bitmap

#include "bitmap.h"
/*
#include <malloc.h>

typedef struct sbitmap
   { uint *data;
     uint n;        // # of bits
     uint pop;	    // # bits set
     uint pop0;	    // # bits not set
     uint *sdata;   // superblock counters for 1
     uint *sdata0;  // superblock counters for 0
     uint sSize;	//		size of sdata vector
     byte *bdata;   // block counters for 1
     byte *bdata0;  // block counters for 0
     uint bSize; 	//     size of bdata vector
     uint mem_usage;
   } *bitmap;
   
*/

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
const unsigned char popc[] =   //number of ones in one byte value [0..255].
{
0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,
};

unsigned char popc0[256];

void initPopc0() {
register int i;
	for (i=0;i<256;i++) popc0[i]=8-popc[i];	
}

uint popcount (register uint x)

   { return popc[x&0xFF] + popc[(x>>8)&0xFF] + popc[(x>>16)&0xFF] + popc[x>>24];
   }


uint popcount0 (register uint x)

   { return popc0[x&0xFF] + popc0[(x>>8)&0xFF] + popc0[(x>>16)&0xFF] + popc0[x>>24];
   }
  
/******************************************************************/
// FUNCIONS DE EDU ...
/******************************************************************/
/* 
	Creates a bitmap and structures to rank and select 
*/

//bitmap createBitmapEdu (uint *string, uint n){  return createBitmap(string,n);}

bitmap createBitmap (uint *string, uint n){
    bitmap B;
    
    initPopc0();
    
	unsigned int nb;
	unsigned int ns;
	unsigned int countB, countS, blockIndex, superblockIndex, stotal, btotal;
    register unsigned int block;

	B = (bitmap) malloc (sizeof(struct sbitmap));
	B->data = string;
	B->n = n; 
	ns = (n/256)+1;
	nb = (n/32)+1;

  B->bSize = nb;
  B->sSize = ns;
	B->bdata =(byte *)malloc(nb*sizeof(byte));  //	Db = (unsigned char *)malloc(nb*sizeof(unsigned char));
	B->sdata = (uint*)malloc(ns*sizeof(uint));  // 	Ds = (unsigned int *)malloc(ns*sizeof(unsigned int));

	//B->bdata0 =(byte *)malloc(nb*sizeof(byte));  //	Db = (unsigned char *)malloc(nb*sizeof(unsigned char));
	//B->sdata0 = (uint*)malloc(ns*sizeof(uint));  // 	Ds = (unsigned int *)malloc(ns*sizeof(unsigned int));

	B->mem_usage = (ns*sizeof(uint)) + (nb*sizeof(byte)) + (sizeof(struct sbitmap));
			printf("\t Memoria utilizada (bloques): %u bytes\n",B->mem_usage);
	/* Ahora construimos los bloques */
    blockIndex = 0;
    superblockIndex = 0;
    countB = 0;
    countS = 0;

    while(blockIndex < nb) {
    	
       if(!(blockIndex%8)) {
          countS += countB;
          B->sdata[superblockIndex] = countS;
	  stotal = (superblockIndex)*256;
	  //B->sdata0[superblockIndex] = stotal - countS;
		//printf("countS: %i stotal: %i\n",countS,stotal);
	  superblockIndex++;
          countB = 0;
       }
       
       B->bdata[blockIndex] = countB;
       btotal = (blockIndex)*32;
       //B->bdata0[blockIndex] = btotal - countB;
		//printf("countB: %i btotal: %i\n",countB,btotal);

	   block = string[blockIndex++];
	   
	   countB += popcount(block);
	   	
    }
	
	B->pop = countS+countB;
	B->pop0 = n - B->pop;
	
/*	{int i;     fprintf(stderr,"\n sdata y sdata0: ");
     for (i=0;i<ns;i++) {fprintf(stderr,"%d -",B->sdata[i]);
			fprintf(stderr,"%d / ",B->sdata0[i]);
     	}
     fprintf(stderr,"\n bdata y bdata0: ");
     for (i=0;i<8;i++) {fprintf(stderr,"%d -",B->bdata[i]);
			fprintf(stderr,"%d / ",B->bdata0[i]);
     	}fprintf(stderr,"\n");
	}*/
    return B;
}	


/*
  Number of 1s in range [0,posicion]
*/
//uint rank1Edu(bitmap B, unsigned int position) {
//uint rank1Edu(bitmap B, unsigned int position) { return rank(B,position);}
uint rank(bitmap B, uint position) {
    register unsigned int block; 
    if((position+1)==0) return 0;   
    if (position > B->n) return B->pop;    
	//position -=1;
	
	block = B->data[position/32] << (31-position%32);

    return B->sdata[position/256] + B->bdata[position/32] + 
           popc[block & 0xff] + popc[(block>>8) & 0xff] +
           popc[(block>>16) & 0xff] + popc[block>>24];
}   


/********************************************************************************************/
/**************************************************************************************/

static uint binsearch (uint *data, uint size, uint val)

   { uint i,j,m;
     i = 0; j = size;
     while (i+1 < j)
	{ m = (i+j)/2;
	  if (data[m] >= val) j = m;
	  else i = m;
	}
     return i;
   }

static uint binsearch0 (uint *data, uint size, uint val)

   { uint i,j,m;
     i = 0; j = size;
     while (i+1 < j)
	{ m = (i+j)/2;
	  if ((m*256-data[m]) >= val) j = m;
	  else i = m;
	}
     return i;
   }

uint bselect (bitmap B, uint j)

   { uint spos,bpos,pos,word,x;
     byte *blk;
     if (j > B->pop) return B->n;
     spos = binsearch(B->sdata,(B->n+256-1)/256,j);
     
     //fprintf(stderr,"\n SPOS IS %d, and B->sdata[pos] = %d",spos,B->sdata[spos]);
     j -= B->sdata[spos];
     pos = spos<<8;
     blk = B->bdata + (pos>>5);
     bpos = 0;
    
    //while ((bpos < (1<<3)-1) && (blk[bpos+1] < j)) bpos++;
    while ( ((spos*8+bpos) < ((B->n-1)/W)) && (bpos < (1<<3)-1) && (blk[bpos+1] < j)) bpos++;
     
    
      //fprintf(stderr,"\n BPOS  = %d",bpos);
     pos += bpos<<5;
     word = B->data[pos>>5];
     j -= blk[bpos];
     //fprintf(stderr,"\n pos>>5 = %d ... pasou XXX con word = %d, and j= %d",pos>>5,word,j);
     while (1) 
    { x = popc[word & ((1<<8)-1)]; 
    	//fprintf(stderr,"\n word = %u popc vale %u",word & ((1<<8)-1),x);
	  if (j <= x) break;
	  j -= x; pos += 8;
	  word >>= 8;
	  
	}
	
     while (j) { if (word & 1) j--; word >>= 1; pos++; }

     //	fprintf(stderr,"\n\nBSELECT::: POSICIN FINAL = %u",pos-1);
     return pos-1;
     
   }


uint bselect0 (bitmap B, uint j)

   { uint spos,bpos,pos,word,x;
     byte *blk;
     //fprintf(stderr,"\n j IS %d, and B->pop = %d",j,B->pop);
     if (j > (B->n - B->pop)) return B->n;
     spos = binsearch0(B->sdata,(B->n+256-1)/256,j);
     
     //fprintf(stderr,"\n SPOS IS %d, and B->sdata[pos] = %d",spos,B->sdata[spos]);
     j -= (256*spos - B->sdata[spos]);
     pos = spos<<8;
     blk = B->bdata + (pos>>5);
     bpos = 0;
    
    //while ((bpos < (1<<3)-1) && (blk[bpos+1] < j)) bpos++;
    while ( ((spos*8+bpos) < ((B->n-1)/W)) && (bpos < (1<<3)-1) && (32*(bpos+1)-blk[bpos+1] < j)) bpos++;
     
    
      //fprintf(stderr,"\n BPOS  = %d",bpos);
     pos += bpos<<5;
     word = B->data[pos>>5];
     j -= (32*bpos - blk[bpos]);
     //fprintf(stderr,"\n pos>>5 = %d ... pasou XXX con word = %d, and j= %d",pos>>5,word,j);
     while (1) 
    { x = popc0[word & ((1<<8)-1)]; 
    	//fprintf(stderr,"\n word = %u popc vale %u",word & ((1<<8)-1),x);
	  if (j <= x) break;
	  j -= x; pos += 8;
	  word >>= 8;
	  
	}
	
     while (j) { if (!(word & 1)) j--; word >>= 1; pos++; }

     //fprintf(stderr,"\n\nBSELECT::: POSICIN FINAL = %u",pos-1);
     return pos-1;
   }

/*
uint bselect0old (bitmap B, uint j)

   { uint spos,bpos,pos,word,x;
     byte *blk;
     if (j > B->pop0) return B->n;
     spos = binsearch(B->sdata0,(B->n+256-1)/256,j);
     
     //fprintf(stderr,"\n SPOS IS %d, and B->sdata0[pos] = %d",spos,B->sdata0[spos]);
     j -= B->sdata0[spos];
     pos = spos<<8;
     blk = B->bdata0 + (pos>>5);
     bpos = 0;
    
    //while ((bpos < (1<<3)-1) && (blk[bpos+1] < j)) bpos++;
    while ( ((spos*8+bpos) < ((B->n-1)/W)) && (bpos < (1<<3)-1) && (blk[bpos+1] < j)) bpos++;
     
    
      //fprintf(stderr,"\n BPOS  = %d",bpos);
     pos += bpos<<5;
     word = B->data[pos>>5];
     j -= blk[bpos];
     //fprintf(stderr,"\n pos>>5 = %d ... pasou XXX con word = %d, and j= %d",pos>>5,word,j);
     while (1) 
    { x = 8 - popc[word & ((1<<8)-1)]; 
    	//fprintf(stderr,"\n word = %u popc vale %u",word & ((1<<8)-1),x);
	  if (j <= x) break;
	  j -= x; pos += 8;
	  word >>= 8;
	  
	}
	
     while (j) { if (!(word & 0x1)) j--; word >>= 1; pos++; }

     //	fprintf(stderr,"\n\nBSELECT::: POSICIN FINAL = %u",pos-1);
     return pos-1;
     
   }

*/

// destroys the bitmap, freeing the original bitstream
void destroyBitmap (bitmap B)

   { //free (B->data);
     free (B->bdata);
     free (B->sdata);
     //free (B->bdata0);
     //free (B->sdata0);
     free (B);
   }
   
   
// Prints the bit vector
void showBitVector(uint *V, uint vectorSize) {
     uint bitIndex=0;
     while(bitIndex<vectorSize) {
        fprintf(stderr,"%d",bitget(V,bitIndex));
        bitIndex++;
     }       
}

  
void saveBitmap (char *filename, bitmap b) {
	FILE * file;
	unlink(filename);
	if( (file = fopen(filename, "w")) ==NULL) {
		printf("Cannot open file %s\n", filename);
		exit(0);
	}
	fwrite(&(b->sSize), sizeof(uint),1,file);
	fwrite(b->sdata, sizeof(int),(b->sSize),file);
	fwrite(&(b->bSize), sizeof(uint),1,file);
	fwrite(b->bdata, sizeof(byte),(b->bSize),file);

	//write(file, &(b->sSize), sizeof(uint));
	//write(file, b->sdata0, sizeof(int) * (b->sSize));
	//write(file, &(b->bSize), sizeof(uint));
	//write(file, b->bdata0, sizeof(byte) * (b->bSize));


	fwrite(&(b->pop), sizeof(uint),1,file);
	fwrite(&(b->pop0), sizeof(uint),1,file);
	fwrite(&(b->n), sizeof(uint),1,file);
	fclose(file);		
}

/* loads the Rank structures from disk, and sets Bitmap->data ptr to "string"
*/
bitmap loadBitmap (char *filename, uint *string, uint n) {  
	bitmap B;
	FILE * file;
	
	if( (file = fopen(filename, "r")) ==NULL) {
		printf("Cannot read file %s\n", filename);
		exit(0);
	}		
	
	B = (bitmap) malloc (sizeof(struct sbitmap));
	B->data = string;
	
	fread(&(B->sSize), sizeof(uint),1,file);
	B->sdata = (uint *) malloc(sizeof(uint) * B->sSize);
	fread(B->sdata, sizeof(uint),B->sSize,file);

	fread(&(B->bSize), sizeof(uint),1,file);
	B->bdata = (byte *) malloc(sizeof(byte) * B->bSize);
	fread(B->bdata, sizeof(byte), B->bSize,file);	
	
	//read(file, &(B->sSize), sizeof(uint));
	//B->sdata0 = (uint *) malloc(sizeof(uint) * B->sSize);
	//read(file, B->sdata0, sizeof(uint) * B->sSize);

	//read(file, &(B->bSize), sizeof(uint));
	//B->bdata0 = (byte *) malloc(sizeof(byte) * B->bSize);
	//read(file, B->bdata0, sizeof(byte) * B->bSize);	

	fread(&(B->pop), sizeof(uint),1,file);
	fread(&(B->pop0), sizeof(uint),1,file);
	fread(&(B->n), sizeof(uint),1,file);	
	fclose(file);
	B->mem_usage = (sizeof(uint) * B->sSize) + (sizeof(byte) * B->bSize) + (sizeof(struct sbitmap));
	
	if (n != B->n) {printf("\n LoadBitmap failed: %u distinto de %u",n,B->n); exit(0);}
	return B;
		
} 
   
   
/********************************************************************************************/
/********************************************************************************************/




	// creates a bitmap structure from a bitstring, which is shared

bitmap createBitmapGONZA (uint *string, uint n)
//bitmap createBitmap (uint *string, uint n)

   { bitmap B;
     uint i,j,pop,bpop,pos;
     uint s,nb,ns,words;
     B = (bitmap) malloc (sizeof(struct sbitmap));
     B->data = string;
     
   
 	 B->n = n; words = (n+W-1)/W;
     ns = (n+256-1)/256; nb = 256/W; // adjustments          
     
     B->bSize = ns*nb;
     B->bdata = (unsigned char*) malloc (ns*nb*sizeof(byte));
     B->sSize = ns;
     B->sdata = (unsigned int*) malloc (ns*sizeof(int));

	 B->mem_usage = (ns*sizeof(int)) + (ns*nb*sizeof(byte)) + (sizeof(struct sbitmap));
#ifdef INDEXREPORT
     printf ("     Bitmap over %i bits took %i bits\n", n,n+ns*nb*8+ns*32);
#endif 
	  //fprintf (stderr,"     Bitmap over %i bits took %i bits\n", n,n+ns*nb*8+ns*32);
     pop = 0; pos = 0;
     for (i=0;i<ns;i++)
	{ bpop = 0;
	  B->sdata[i] = pop;
	  for (j=0;j<nb;j++)
	     { if (pos == words) break;
	       B->bdata[pos++] = bpop;
	       bpop += popcount(*string++);
	     }
	  pop += bpop;
	}
     B->pop = pop;
     
		//     //fprintf(stderr,"\n");
		//     for (i=0;i<ns;i++) {//fprintf(stderr,"%d ",B->sdata[i]);
		//     	}
		//     //fprintf(stderr,"\n");
		//     for (i=0;i<ns*nb;i++) {//fprintf(stderr,"%d ",B->bdata[i]);
		//     	}
		     
     return B;
   }

	// rank(i): how many 1's are there before position i, not included

//uint rank (bitmap B, uint i)
uint rankGONZA (bitmap B, uint i)

   { 
   	i++;
   	if (i > B->n) return B->pop;
   	return B->sdata[i>>8] + B->bdata[i>>5] +
	    popcount (B->data[i>>5] & ((1<<(i&0x1F))-1));
   }


   
   
   
   
   



		

