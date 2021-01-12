#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

 /*
 * des takes two arguments on the command line:
 *    des -enc -ecb      -- encrypt in ECB mode
 *    des -enc -ctr      -- encrypt in CTR mode
 *    des -dec -ecb      -- decrypt in ECB mode
 *    des -dec -ctr      -- decrypt in CTR mode
 * des also reads some hardcoded files:
 *    message.txt            -- the ASCII text message to be encrypted,
 *                              read by "des -enc"
 *    encrypted_msg.bin      -- the encrypted message, a binary file,
 *                              written by "des -enc"
 *    decrypted_message.txt  -- the decrypted ASCII text message
 *    key.txt                -- just contains the key, on a line by itself, as an ASCII 
 *                              hex number, such as: 0x34FA879B
*/

/////////////////////////////////////////////////////////////////////////////
// Type definitions
/////////////////////////////////////////////////////////////////////////////
typedef uint64_t KEYTYPE;
typedef uint32_t SUBKEYTYPE;
typedef uint64_t BLOCKTYPE;

struct BLOCK {
    BLOCKTYPE block;        // the block read
    int size;               // number of "real" bytes in the block, should be 8, unless it's the last block
    struct BLOCK *next;     // pointer to the next block
};

struct BLOCKHEAD {
    struct BLOCK *first;   // first block
    struct BLOCK *last;    // last block
};

typedef struct BLOCK* BLOCKNODE;
typedef struct BLOCKHEAD* BLOCKLIST;
KEYTYPE theKey;

BLOCKLIST newBlockList() {
   BLOCKLIST blocks = (BLOCKLIST)malloc(sizeof(struct BLOCKHEAD));
   blocks->first = NULL;
   blocks->last = NULL;
   return blocks;
}

BLOCKNODE newBlock(uint64_t block, int size){
  BLOCKNODE new_node = (BLOCKNODE) malloc(sizeof(struct BLOCK));
  new_node->block = block;
  new_node->size = size;
  new_node->next = NULL;
  return new_node;
}

void addBlockLast(BLOCKLIST list, BLOCKTYPE block, int size) {
   BLOCKNODE currBlock = newBlock(block, size);
   if (list->first == NULL) {
      list->first = currBlock;
      list->last = currBlock;
    } else {
      list->last->next = currBlock;
      list->last = currBlock;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Debugging
/////////////////////////////////////////////////////////////////////////////
int debug = 1;

void print_blocklist(BLOCKLIST head) {
   BLOCKNODE curr = head->first;
   int x = 0;
   while (curr != NULL) {
           printf("\tBLOCK [%d]: 0x%llx, size=%d\n", x, curr->block,curr->size);
      x++;
      curr = curr->next;
   }
}


/////////////////////////////////////////////////////////////////////////////
// Initial and final permutation
/////////////////////////////////////////////////////////////////////////////
uint64_t init_perm[] = {
   58,50,42,34,26,18,10,2,
   60,52,44,36,28,20,12,4,
   62,54,46,38,30,22,14,6,
   64,56,48,40,32,24,16,8,
   57,49,41,33,25,17,9,1,
   59,51,43,35,27,19,11,3,
   61,53,45,37,29,21,13,5,
   63,55,47,39,31,23,15,7
};

int final_perm[] = {
   40,8,48,16,56,24,64,32,
   39,7,47,15,55,23,63,31,
   38,6,46,14,54,22,62,30,
   37,5,45,13,53,21,61,29,
   36,4,44,12,52,20,60,28,
   35,3,43,11,51,19,59,27,
   34,2,42,10,50,18,58,26,
   33,1,41,9, 49,17,57,25
};

// This function does the initial permutation shuffling

BLOCKTYPE initial_permutation(BLOCKTYPE ptext){
   BLOCKTYPE newPerm = 0x0;
   BLOCKTYPE bitToShuff = 0x0;

   //////////////////Debug Purposes/////////////////////////////////////////////////////
   //Test by setting ptext to 0x0123456789ABCDEF
   //ptext = 0x0123456789ABCDEF; //comment this whole line out to activate the real function
   //Expected output: is 0xCC00CCFFF0AAF0AA
   /////////////////////////////////////////////////////////////////////////////////////

   int j = 63; //counts which bit to set each round
   int i = 0; //counts which spot to read from perm table

   for(i=0; i<64; i++){
      bitToShuff = 0x1;
      bitToShuff = bitToShuff << (64 - init_perm[i]); //shift bit to the spot the init_perm[] says it needs to go
      bitToShuff = bitToShuff & ptext; //read what plain text bit is set too.
      if (bitToShuff != 0x0){ //A 1 bit was found so it needs to be set.
         bitToShuff = 0x1; //create 1 bit to set
         bitToShuff = bitToShuff << j;  //shift to next bit position that needs to be set
         newPerm = newPerm ^ bitToShuff; //Set the bit by XORing.  All other values on newPerm were initially set to zero so it shouldn't affect them.
      }
   j--; //move to set next bit position for next round. 
   }
   return newPerm; //return the new permutation
}

/////////////////////////////////////////////////////////////////////////////
// Subkey generation
/////////////////////////////////////////////////////////////////////////////

// This function returns the i:th subkey, 48 bits long. To simplify the assignment 
// you can use a trivial implementation: just take the input key and xor it with i.
uint64_t getSubKey(int i) {
   // return the first 48 bits of the 56 bit DES key, xor:ed with i.
   KEYTYPE key = theKey; //set user input key to local key
   KEYTYPE mask;
   
   //XOR key with i
   key = key ^ i;

   //Add bitmask to keep only 48bits
   mask = 0xFFFFFFFFFFFF; //mask is has 16 leading zeros plus 48 1's
   key = key & mask; //Zeros out anything over 48 bits
   
   return key;
}

// For extra credit, write the real DES key expansion routine!
void generateSubKeys(KEYTYPE key) {

   // TODO for extra credit
}

/////////////////////////////////////////////////////////////////////////////
// P-boxes
/////////////////////////////////////////////////////////////////////////////
uint64_t expand_box[] = {
   32,1,2,3,4,5,4,5,6,7,8,9,
   8,9,10,11,12,13,12,13,14,15,16,17,
   16,17,18,19,20,21,20,21,22,23,24,25,
   24,25,26,27,28,29,28,29,30,31,32,1
};

uint32_t Pbox[] = 
{
   16,7,20,21,29,12,28,17,1,15,23,26,5,18,31,10,
   2,8,24,14,32,27,3,9,19,13,30,6,22,11,4,25,
};      

/////////////////////////////////////////////////////////////////////////////
// S-boxes
/////////////////////////////////////////////////////////////////////////////
uint64_t sbox_1[4][16] = {
   {14,  4, 13,  1,  2, 15, 11,  8,  3, 10 , 6, 12,  5,  9,  0,  7},
   { 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8},
   { 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0},
   {15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13}};

uint64_t sbox_2[4][16] = {
   {15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5 ,10},
   { 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5},
   { 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15},
   {13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9}};

uint64_t sbox_3[4][16] = {
   {10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8},
   {13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1},
   {13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7},
   { 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12}};

uint64_t sbox_4[4][16] = {
   { 7, 13, 14,  3,  0 , 6,  9, 10,  1 , 2 , 8,  5, 11, 12,  4 ,15},
   {13,  8, 11,  5,  6, 15,  0,  3,  4 , 7 , 2, 12,  1, 10, 14,  9},
   {10,  6,  9 , 0, 12, 11,  7, 13 ,15 , 1 , 3, 14 , 5 , 2,  8,  4},
   { 3, 15,  0,  6, 10,  1, 13,  8,  9 , 4 , 5, 11 ,12 , 7,  2, 14}};
 
uint64_t sbox_5[4][16] = {
   { 2, 12,  4,  1 , 7 ,10, 11,  6 , 8 , 5 , 3, 15, 13,  0, 14,  9},
   {14, 11 , 2 ,12 , 4,  7, 13 , 1 , 5 , 0, 15, 10,  3,  9,  8,  6},
   { 4,  2 , 1, 11, 10, 13,  7 , 8 ,15 , 9, 12,  5,  6 , 3,  0, 14},
   {11,  8 ,12 , 7 , 1, 14 , 2 ,13 , 6 ,15,  0,  9, 10 , 4,  5,  3}};

uint64_t sbox_6[4][16] = {
   {12,  1, 10, 15 , 9 , 2 , 6 , 8 , 0, 13 , 3 , 4 ,14 , 7  ,5 ,11},
   {10, 15,  4,  2,  7, 12 , 9 , 5 , 6,  1 ,13 ,14 , 0 ,11 , 3 , 8},
   { 9, 14 ,15,  5,  2,  8 ,12 , 3 , 7 , 0,  4 ,10  ,1 ,13 ,11 , 6},
   { 4,  3,  2, 12 , 9,  5 ,15 ,10, 11 ,14,  1 , 7  ,6 , 0 , 8 ,13}};
 
uint64_t sbox_7[4][16] = {
   { 4, 11,  2, 14, 15,  0 , 8, 13, 3,  12 , 9 , 7,  6 ,10 , 6 , 1},
   {13,  0, 11,  7,  4 , 9,  1, 10, 14 , 3 , 5, 12,  2, 15 , 8 , 6},
   { 1 , 4, 11, 13, 12,  3,  7, 14, 10, 15 , 6,  8,  0,  5 , 9 , 2},
   { 6, 11, 13 , 8,  1 , 4, 10,  7,  9 , 5 , 0, 15, 14,  2 , 3 ,12}};
 
uint64_t sbox_8[4][16] = {
   {13,  2,  8,  4,  6 ,15 ,11,  1, 10,  9 , 3, 14,  5,  0, 12,  7},
   { 1, 15, 13,  8 ,10 , 3  ,7 , 4, 12 , 5,  6 ,11,  0 ,14 , 9 , 2},
   { 7, 11,  4,  1,  9, 12, 14 , 2,  0  ,6, 10 ,13 ,15 , 3  ,5  ,8},
   { 2,  1, 14 , 7 , 4, 10,  8, 13, 15, 12,  9,  0 , 3,  5 , 6 ,11}};

/////////////////////////////////////////////////////////////////////////////
// I/O
/////////////////////////////////////////////////////////////////////////////

// Pad the list of blocks, so that every block is 64 bits, even if the
// file isn't a perfect multiple of 8 bytes long. In the input list of blocks,
// the last block may have "size" < 8. In this case, it needs to be padded. See 
// the slides for how to do this (the last byte of the last block 
// should contain the number if real bytes in the block, add an extra block if
// the file is an exact multiple of 8 bytes long.) The returned
// list of blocks will always have the "size"-field=8.
// Example:
//    1) The last block is 5 bytes long: [10,20,30,40,50]. We pad it with 2 bytes,
//       and set the length to 5: [10,20,30,40,50,0,0,5]. This means that the 
//       first 5 bytes of the block are "real", the last 3 should be discarded.
//    2) The last block is 8 bytes long: [10,20,30,40,50,60,70,80]. We keep this 
//       block as is, and add a new final block: [0,0,0,0,0,0,0,0]. When we decrypt,
//       the entire last block will be discarded since the last byte is 0

BLOCKLIST pad_last_block(BLOCKLIST blocks) {
    
    int bytes_to_pad = 8 - blocks->last->size;
    //create a block to do padding with
    BLOCKTYPE padded_Block = bytes_to_pad;
    BLOCKTYPE block = 0; //for adding final block if necessary

    //needs padding
    if(bytes_to_pad > 0){
      //shift last block number of bytes needed and pad with zeros. 
      blocks->last->block = blocks->last->block << (8 * bytes_to_pad); 
      //OR padded block with last block to identify what needs to be removed at the end
      blocks->last->block = padded_Block | blocks->last->block;
      //update size field to 8 bytes now its padded
      blocks->last->size = 8;
    } 
   else{
    //doesn't need padding add final block
   addBlockLast(blocks, block, 8);
   }
   return blocks;
}

//This function expands the right block in the feistal network

BLOCKTYPE rightBlockExpansion(BLOCKTYPE smallRblock ){
   BLOCKTYPE newPerm = 0x0;
   BLOCKTYPE bitToShuff = 0x0;
   /////////////////////////////////////////////Debugging purposes//////////////////////////////////////////////////////
   //Test by setting smallRblock to 0xF0AAF0AA
   //smallRblock = 0xF0AAF0AA; //comment this whole line out to activate the real function
   //Expected output: is 0x7A15557A1555
   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   int j = 47; //counts which bit to set each round
   int i = 0; //counts which spot to read from perm table
   
   for(i=0; i<48; i++){
      bitToShuff = 0x1;
      bitToShuff = bitToShuff << (32 - expand_box[i] ); //shift bit to the spot the expand_box[] says it needs to go
      bitToShuff = bitToShuff & smallRblock; //read what plain text bit is set too.
      if (bitToShuff != 0x0){ //A 1 bit was found so it needs to be set.
         bitToShuff = 0x1; //create 1 bit to set
         bitToShuff = bitToShuff << j;  //shift to next bit position that needs to be set
         newPerm = newPerm ^ bitToShuff; //Set the bit by XORing.  All other values on newPerm were initially set to zero so it shouldn't affect them.
      }
   j--; //move to set next bit position for next round. 
   }
   return newPerm; //return the new permutation
}

//This function takes the rightBlock Xor'ed with a key of type Blocktype and sends it through the series of S-boxes

BLOCKTYPE thruSboxSeries(BLOCKTYPE aBlock){
   uint64_t sb_input;
   uint32_t reducedBlock = 0x0;
   uint32_t temp = 0x0;
   int row;
   int column;
   uint64_t sboxMask = 0xFC0000000000; //bitmask to read first 6 bits should be 16 leading 0's 111111 plus 42 trailing 0's
   uint64_t rowMask = 0x21;  //Bit mask should be 100001 for getting the end bits
   uint64_t columnMask = 0x1E;  //bit mask should be 011110 For getting the column bits in the middle 
   int i = 0;
   int j = 7;

   ////////////////////////Debug purposes ////////////////////////////////////////////////////////////////////////////////////
   //Testing: Set ablock to 0x6C0000000000 this should be 16 leading zeros plus 011011 plus 42 trailing zeros
   //Expected output for sb_input on first round should be: 0x1B the binary equivalent of 011011
   //Expected output for sbox1 should be: 5 and the row and column should be 1 and 13
   //aBlock = 0x6C0000000000;  //Comment ot whole line to activate real function
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
   
   for(i=0; i<8; i++){ //Each loop grab 6 bits then shift and grab next 6 bits
      //peel out 6 bit val going into sbox #i
      sboxMask = sboxMask >> (i * 6); //shift mask according to what block of 6 bits is needed (shift zero positins 1st round)  
      sb_input = sboxMask & aBlock; //AND against aBlock to peel out the 6 bits.
      sb_input = sb_input >> (j * 6); //Shift sb_input to get it to a least significant bits
 
      //get row val
      row = (int) sb_input & rowMask; // zero's out the middle column bits leaving the row end bits
      row = (row >> 4)|(row & 0x1); //(row >>4) = ex: 100001=>000010 (row&0x1) keeps last bit ex: 100001=>000001 
                                    //OR'ing the two gets a 2bit value consisting the two end bits for the row value.  
      //get column val.
      column = (int) sb_input & columnMask; // apply column mask to peel out middle bits for column
      column = column >> 1; //took off last rowbit so shift once to compensate

      temp = 0x0; //zero out temp block each time.
      sboxMask = 0xFC0000000000; //Reset sboxMask each loop
      
      //Choose which sbox to use
      switch (i)
      {
      case 0:
         temp = (uint32_t)sbox_1[row][column]; //retrieves sbox 1 val and sets it to new value
         temp = temp << 28; // shifts temp value to first set of 4 bits.
         reducedBlock = temp ^ reducedBlock; //Assign 1st 4 bits to reduced block
         break;

      case 1:
         temp = (uint32_t)sbox_2[row][column]; //retrieves sbox 2 val and sets it to new value
         temp = temp << 24; // shifts temp value to second set of 4 bits.
         reducedBlock = temp ^ reducedBlock; //Assign 2nd 4 bits to reduced block
         break;

      case 2:
         temp = (uint32_t)sbox_3[row][column]; //retrieves sbox 3 val and sets it to new value
         temp = temp << 20; // shifts temp value to Third set of 4 bits.
         reducedBlock = temp ^ reducedBlock; //Assign 3rd 4 bits to reduced block
         break;

      case 3:
         temp = (uint32_t)sbox_4[row][column]; //retrieves sbox 4 val and sets it to new value
         temp = temp << 16; // shifts temp value to Fourth set of 4 bits.
         reducedBlock = temp ^ reducedBlock; //Assign 4th 4 bits to reduced block
         break;

      case 4:
         temp = (uint32_t)sbox_5[row][column]; //retrieves sbox 5 val and sets it to new value
         temp = temp << 12; // shifts temp value to Fifth set of 4 bits.
         reducedBlock = temp ^ reducedBlock; //Assign 5th 4 bits to reduced block
         break;

      case 5:
         temp = (uint32_t)sbox_6[row][column]; //retrieves sbox 6 val and sets it to new value
         temp = temp << 8; // shifts temp value to Sixth set of 4 bits.
         reducedBlock = temp ^ reducedBlock; //Assign 6th 4 bits to reduced block
         break;

      case 6:
         temp = (uint32_t)sbox_7[row][column]; //retrieves sbox 7 val and sets it to new value
         temp = temp << 4; // shifts temp value to Seventh set of 4 bits.
         reducedBlock = temp ^ reducedBlock; //Assign 7th 4 bits to reduced block
         break;

      case 7:
         temp = (uint32_t)sbox_8[row][column]; //retrieves sbox 8 val and sets it to new value temp value is set to first 4 bits.
         reducedBlock = temp ^ reducedBlock; //Assign 1st 4 bits to reduced block
         break;

      default:
         break;
      }   
      j--;  //decrement j so 6 less bits to shift next round. 
   }
   
   return reducedBlock;
}


//This function does the permutation inside but at the end of the Round function using the p-table array

uint32_t rndFuncPerm(uint32_t rblock){
BLOCKTYPE newPerm = 0x0;
   BLOCKTYPE bitToShuff = 0x0;
   
   ///////////////////////////////////////////Debugging Purposes////////////////////////////////////////////////////
   //Test by setting rblock to 0x5C82B597
   //rblock = 0x5C82B597; //comment this whole line out to activate the real function
   //Expected output: is 0x234AA9BB
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   int j = 31; //counts which bit to set each round
   int i = 0; //counts which spot to read from perm table
   
   for(i=0; i<32; i++){
      bitToShuff = 0x1;
      bitToShuff = bitToShuff << (32 - Pbox[i] ); //shift bit to the spot the expand_box[] says it needs to go
      bitToShuff = bitToShuff & rblock; //read what plain text bit is set too.
      if (bitToShuff != 0x0){ //A 1 bit was found so it needs to be set.
         bitToShuff = 0x1; //create 1 bit to set
         bitToShuff = bitToShuff << j;  //shift to next bit position that needs to be set
         newPerm = newPerm ^ bitToShuff; //Set the bit by XORing.  All other values on newPerm were initially set to zero so it shouldn't affect them.
      }
   j--; //move to set next bit position for next round. 
   }
   return newPerm; //return the new permutation
}


//This function runs the Final inverse Permutaion.
BLOCKTYPE runFinalPermutation(BLOCKTYPE lastBlock){
   BLOCKTYPE newPerm = 0x0;
   BLOCKTYPE bitToShuff = 0x0;

   ///////////////////////////////////////////////////Debugging Purposes/////////////////////////////////////////////////
   //Test by setting lastBlock to 0xA4CD99543423234
   //lastBlock = 0xA4CD99543423234; //comment this whole line out to activate the real function
   //Expected output: is 0x85E813540F0AB405
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   int j = 63; //counts which bit to set each round
   int i = 0; //counts which spot to read from perm table

   for(i=0; i<64; i++){
      bitToShuff = 0x1;
      bitToShuff = bitToShuff << (64 - final_perm[i]); //shift bit to the spot the init_perm[] says it needs to go
      bitToShuff = bitToShuff & lastBlock; //read what plain text bit is set too.
      if (bitToShuff != 0x0){ //A 1 bit was found so it needs to be set.
         bitToShuff = 0x1; //create 1 bit to set
         bitToShuff = bitToShuff << j;  //shift to next bit position that needs to be set
         newPerm = newPerm ^ bitToShuff; //Set the bit by XORing.  All other values on newPerm were initially set to zero so it shouldn't affect them.
      }
   j--; //move to set next bit position for next round. 
   }
   return newPerm; //return the new permutation
}


// Reads the message to be encrypted, an ASCII text file, and returns a linked list 
// of BLOCKs, each representing a 64 bit block. In other words, read the first 8 characters
// from the input file, and convert them (just a C cast) to 64 bits; this is your first block.
// Continue to the end of the file.

BLOCKLIST read_cleartext_message(FILE *msg_fp) {
   BLOCKLIST blocks = newBlockList();
   unsigned char byte = 0;
   int retVal = fscanf(msg_fp, "%c", &byte);
   BLOCKTYPE block = 0;
   int counter = 0;
   //if value is not null then keep grabbing next value in message
   while (retVal > 0) {
      //if counter reaches 8 then you have an 8 byte block so add it to the blocklist
      if (counter == 8) {
         addBlockLast(blocks, block, counter);
         counter = 0;
         block = 0;
      }
      //add byte of message to block by shifting over older stuff and OR-ing new byte 
      block = (block << 8) | byte; 
      counter++; // one byte added to block
      retVal = fscanf(msg_fp, "%c", &byte); //read next byte
   }
   
   //add left over change to blocklist
   if (counter > 0) {
      addBlockLast(blocks, block, counter);
   }
   
   blocks = pad_last_block(blocks);
   
   return blocks;
}


// Reads the encrypted message, and returns a linked list of blocks, each 64 bits. 
// Note that, because of the padding that was done by the encryption, the length of 
// this file should always be a multiple of 8 bytes. The output is a linked list of
// 64-bit blocks.
BLOCKLIST read_encrypted_file(FILE *msg_fp) {
    BLOCKLIST blocks = newBlockList();
    uint64_t block;
    while(fread(&block, sizeof(block), 1, msg_fp) > 0){
      addBlockLast(blocks, block, 8);
      block = 0;
    }
    return blocks;
}

// Reads 56-bit key into a 64 bit unsigned int. We will ignore the most significant byte,
// i.e. we'll assume that the top 8 bits are all 0. In real DES, these are used to check 
// that the key hasn't been corrupted in transit. The key file is ASCII, consisting of
// exactly one line. That line has a single hex number on it, the key, such as 0x08AB674D9.
KEYTYPE read_key(FILE *key_fp) {
   char buff[17];
   fgets(buff, 17, key_fp);
   KEYTYPE key = strtol(buff, 0, 16);
   
   return key;
}

// Write the encrypted blocks to file. The encrypted file is in binary, i.e., you can
// just write each 64-bit block directly to the file, without any conversion.
void write_encrypted_message(FILE *msg_fp, BLOCKLIST msg) {
    assert(msg != NULL);
    BLOCKNODE curr = msg->first;
    char buffer[8];
    while(curr != NULL){
        memcpy(&buffer,&(curr->block),curr->size);
        fwrite(buffer, sizeof(char), 8, msg_fp);
        curr = curr->next;
    }
    return;
}

// Write the encrypted blocks to file. This is called by the decryption routine.
// The output file is a plain ASCII file, containing the decrypted text message.
void write_decrypted_message(FILE *msg_fp, BLOCKLIST msg) {
   assert(msg != NULL);
   BLOCKNODE cur = msg->first;
   int size;
   unsigned int charac;
   while(cur != NULL){
      for(int i=8; i>=1; i--){
         charac = (unsigned int) (((cur->block) >> ((i-1)*8)) & 0xFF);
         fprintf(msg_fp,"%c",charac);
      }
      cur = cur->next;
   }
}


/////////////////////////////////////////////////////////////////////////////
// Encryption
/////////////////////////////////////////////////////////////////////////////

// Encrypt one block. This is where the main computation takes place. It takes
// one 64-bit block as input, and returns the encrypted 64-bit block. The 
// subkeys needed by the Feistel Network is given by the function getSubKey(i).
BLOCKTYPE des_enc(BLOCKTYPE v){   
   //Implementation of Feistal network
   //step 0 create initial permutation to plain text
   v = initial_permutation(v);
   //step 1 create left and right blocks
   uint32_t lBlock;
   uint32_t rBlock;
   uint32_t temprBlock;
   uint32_t templBlock;
   uint32_t bitmask;
   BLOCKTYPE expRblock;  //this is for the expanded right block to send through the round function.
   int i =0;

   //step 1 split left and right blocks up
         bitmask = 0x00000000ffffffff; //This is equivalent to last 32 bits being 1's
         rBlock = (uint32_t)v & bitmask; //grabs last 32 bits and assigns to right block
         lBlock = (uint32_t)(v >> 32) & bitmask; //shifts left 32 bits to the right side then assigns them to left block

   //step 2 for-loop for 16 rounds
      //create for loop round
      for(i=0; i<16; i++){
         
         templBlock = lBlock; //make copy of lBlock to switch later

         //step 1 of (16 round loop) send right block through the ROUNDFUNCTION where 48 bit key is passed in from getSubKey
         expRblock = rightBlockExpansion((BLOCKTYPE)rBlock); //expand rblock to 48bits
         expRblock = expRblock ^ getSubKey(i);//XOR RightBlock with key before sending through s-boxes
         temprBlock = thruSboxSeries(expRblock);//Send XOR'ed right box through s-boxes. temprBlock is now 32bits
         temprBlock = rndFuncPerm(temprBlock); // RoundFunction Permutation added
                           //End ROUNDFUNCTION

         //step 2 of (16 round loop) assign old right to new left block.
         lBlock = rBlock;
         
         //step 3 of (16 round loop) XOR Left block with output from round function and assign it to knew right block.
         rBlock = templBlock ^ temprBlock;
         
      }//End of 16 rounds 
      
   //step 3 final swap
   expRblock = 0x0;  //Reusing this variable to temp hold both left and right blocks which are switched and sent through IP^1
   expRblock = expRblock ^ rBlock; //This sets therblock into the right side of expRblock
   expRblock = expRblock << 32; //This shifts the rblock to the left side of expRblock
   expRblock = expRblock ^ lBlock;  //This adds the lbock to the right side of expRblock thus completeing the swap.
   
   //step 4 inverse initial permutation
   expRblock = runFinalPermutation(expRblock);
   //step 5 return encrypted block
   return expRblock;
}

// Encrypt the blocks in ECB mode. The blocks have already been padded 
// by the input routine. The output is an encrypted list of blocks.
BLOCKLIST des_enc_ECB(BLOCKLIST msg) {
    assert(msg != NULL);
    // ****************************************************des_enc_ECB Done*********************
    // Should call des_enc in here repeatedly
   
   BLOCKNODE curr = msg->first;
   int x = 0;
   while (curr != NULL) {
      curr->block = des_enc(curr->block);
      x++;
      curr = curr->next;
   }
   return msg;
}

// Same as des_enc_ECB, but encrypt the blocks in Counter mode.
// SEE: https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation#Counter_(CTR)
// Start the counter at 0.
BLOCKLIST des_enc_CTR(BLOCKLIST msg) {
    assert(msg != NULL);
   BLOCKTYPE seed = 0x1111111111111111;
   BLOCKTYPE vector = 0x0;

    // Should call des_enc in here repeatedly
   BLOCKNODE curr = msg->first;  //set current block to point to first block in the Linked

    //Step 1  create while loop. 
   while (curr != NULL) {
      //Step 2 Encrypt seed to get vector
      vector = des_enc(seed); //plug seed value into encryption algorithm
      //Step 3 XOR vector with Plaintext block and save cipher text in it's place
      curr->block = curr->block ^ vector; // Curr->block is next set of plain text and is XOR'ed with Vector and saved 
      seed = seed + 0x1; //increment seed by 1
      curr = curr->next; //Proceed to next block
   }
   return msg;
}

/////////////////////////////////////////////////////////////////////////////
// Decryption
/////////////////////////////////////////////////////////////////////////////
// Decrypt one block.
BLOCKTYPE des_dec(BLOCKTYPE v){
    // Same as encryption but reverse subkey order.

    //Implementation of Feistal network
   //step 0 create initial permutation to plain text
   v = initial_permutation(v);
   //step 1 create left and right blocks
   uint32_t lBlock;
   uint32_t rBlock;
   uint32_t temprBlock;
   uint32_t templBlock;
   uint32_t bitmask;
   BLOCKTYPE expRblock;  //this is for the expanded right block to send through the round function.
   int i =0;

   //step 1 split left and right blocks up
         bitmask = 0x00000000ffffffff; //This is equivalent to last 32 bits being 1's
         rBlock = (uint32_t)v & bitmask; //grabs last 32 bits and assigns to right block
         lBlock = (uint32_t)(v >> 32) & bitmask; //shifts left 32 bits to the right side then assigns them to left block

   //step 2 for-loop for 16 rounds
      //create for loop round
      for(i=0; i<16; i++){
         
         templBlock = lBlock; //make copy of lBlock to switch later

         //step 1 of (16 round loop) send right block through the ROUNDFUNCTION where 48 bit key is passed in from getSubKey
         expRblock = rightBlockExpansion((BLOCKTYPE)rBlock); //expand rblock to 48bits
         expRblock = expRblock ^ getSubKey(15-i);//XOR RightBlock with key before sending through s-boxes (flip keys with 15-i) dec
         temprBlock = thruSboxSeries(expRblock);//Send XOR'ed right box through s-boxes. temprBlock is now 32bits
         temprBlock = rndFuncPerm(temprBlock); // RoundFunction Permutation added
                           //End ROUNDFUNCTION

         //step 2 of (16 round loop) assign old right to new left block.
         lBlock = rBlock;
         
         //step 3 of (16 round loop) XOR Left block with output from round function and assign it to knew right block.
         rBlock = templBlock ^ temprBlock;
         
      }//End of 16 rounds 
      
   //step 3 final swap
   expRblock = 0x0;  //Reusing this variable to temp hold both left and right blocks which are switched and sent through IP^1
   expRblock = expRblock ^ rBlock; //This sets therblock into the right side of expRblock
   expRblock = expRblock << 32; //This shifts the rblock to the left side of expRblock
   expRblock = expRblock ^ lBlock;  //This adds the lbock to the right side of expRblock thus completeing the swap.
   
   //step 4 inverse initial permutation
   expRblock = runFinalPermutation(expRblock);
   //step 5 return encrypted block
   return expRblock;
}

// Decrypt the blocks in ECB mode. The input is a list of encrypted blocks,
// the output a list of plaintext blocks.
BLOCKLIST des_dec_ECB(BLOCKLIST msg) {
    assert(msg != NULL);
    // Should call des_dec in here repeatedly
   BLOCKNODE curr = msg->first;
   
   while (curr != NULL) {
      curr->block = des_dec(curr->block);
      curr = curr->next;
   }
   return msg;
}

// Decrypt the blocks in Counter mode
BLOCKLIST des_dec_CTR(BLOCKLIST msg) {
    assert(msg != NULL);

   BLOCKTYPE seed = 0x1111111111111111;
   BLOCKTYPE vector = 0x0;

    // Should call des_enc in here repeatedly
   BLOCKNODE curr = msg->first;  //set current block to point to first block in the Linked

    //Step 1  create while loop. 
   while (curr != NULL) {
      //Step 2 Encrypt seed to get vector
      vector = des_enc(seed); //plug seed value into encryption algorithm
      //Step 3 XOR vector with Plaintext block and save cipher text in it's place
      curr->block = curr->block ^ vector; // Curr->block is next set of plain text and is XOR'ed with Vector and saved 
      seed = seed + 0x1; //increment seed by 1
      curr = curr->next; //Proceed to next block
   } 
   return msg; 
}

/////////////////////////////////////////////////////////////////////////////
// Main routine
/////////////////////////////////////////////////////////////////////////////

void encrypt (int argc, char **argv) {
      FILE *msg_fp = fopen("message.txt", "r");
      BLOCKLIST msg = read_cleartext_message(msg_fp);
      fclose(msg_fp);

      BLOCKLIST encrypted_message;
      if (strcmp(argv[2], "-ecb") == 0) {   
         encrypted_message = des_enc_ECB(msg);
      } else if (strcmp(argv[2], "-ctr")== 0) {   
         encrypted_message = des_enc_CTR(msg);
      } else {
         printf("No such mode.\n");
         exit(-1);
      };
      
      FILE *encrypted_msg_fp = fopen("encrypted_msg.bin", "wb");
      write_encrypted_message(encrypted_msg_fp, encrypted_message);
      fclose(encrypted_msg_fp);
}

void decrypt (int argc, char **argv) {
      FILE *encrypted_msg_fp = fopen("encrypted_msg.bin", "rb");
      BLOCKLIST encrypted_message = read_encrypted_file(encrypted_msg_fp);
      fclose(encrypted_msg_fp);

      BLOCKLIST decrypted_message;
      if (strcmp(argv[2], "-ecb") == 0) {   
         decrypted_message = des_dec_ECB(encrypted_message);
      } else if (strcmp(argv[2], "-ctr")== 0) {   
         decrypted_message = des_dec_CTR(encrypted_message);
      } else {
         printf("No such mode.\n");
         exit(-1);
      };

      FILE *decrypted_msg_fp = fopen("decrypted_message.txt", "w");
      write_decrypted_message(decrypted_msg_fp, decrypted_message);
      fclose(decrypted_msg_fp);
}

int main(int argc, char **argv){
   FILE *key_fp = fopen("key.txt","r");
   KEYTYPE key = read_key(key_fp);
   theKey = key;
   generateSubKeys(key);              // This does nothing right now.
   fclose(key_fp);
   
   if (argc != 3) {
     printf("Two arguments expected.\n");
     exit(-1);
   }

   if (strncmp(argv[1],"-enc",4) == 0) {
      encrypt(argc, argv);   
   } else if (strncmp(argv[1],"-dec",4) == 0) {
      decrypt(argc, argv);   
   } else {
     printf("First argument should be -enc or -dec\n"); 
   }
   return 0;
}