#include <algorithm>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cmath>
#include <iostream>
#include <sys/wait.h>
#include <string.h>
#include <string>
#include <time.h>
#include <sys/time.h>
using namespace std;

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

int RedFrequency[256];
int GreenFrequency[256];
int BlueFrequency[256];

BYTE Data[1000000];

struct Node
{
   BYTE value;
   int frequency;
   bool letter;
   Node* Left;
   Node* Right;
};

struct FILEHEADER
{
   WORD bfType;
   DWORD bfSize;
   WORD bfReserved1;
   WORD bfReserved2;
   DWORD bfOffBits;
};

struct INFOHEADER
{
   DWORD biSize;
   LONG biWidth;
   LONG biHeight;
   WORD biPlanes;
   WORD biBitCount;
   DWORD biCompression;
   DWORD biSizeImage;
   LONG biXPelsPerMeter;
   LONG biYPelsPerMeter;
   DWORD biClrUsed;
   DWORD biClrImportant;
};



bool comp(Node* a, Node* b){
   return a -> frequency < b -> frequency;
}

struct BitReader
{
   //bitstream pointer
   BYTE* data;
   int bitPos;

   //initialize the bit reader
   BitReader(BYTE* d){
      data = d; 
      bitPos = 0;
   }

   //the bit reader
   int readBit(){
      int bytepos = bitPos / 8;
      int PosInByte = bitPos - bytepos * 8;
      BYTE b = data[bytepos];
      //get the next bit from the byte
      int bit = (b >> (7 - PosInByte)) & 1;
      bitPos++;
      return bit;
   }
};


BYTE decoder(Node* root, BitReader& bitRead){
   Node* p = root;

   //checks if therese only one color
   if(p->letter){
      bitRead.readBit();
      return p->value;
   }
   //traverse tree a bit at a time until leafd is found
   while(!p->letter){
      int bit = bitRead.readBit();
      
      if(bit == 0){
         p = p->Left;
      }
      else{
         p = p->Right;
      }
   }
   return p->value;
}


int main(int argc, char* argv[]){
   const char* inputfileName = argv[1];
   const char* outputfileName = argv[2];

   char specialKey[6] = {0};
   int quality;
   int bytes_used;
   
   
   FILE* inputFile = fopen(inputfileName, "rb");

   struct FILEHEADER header;
   struct INFOHEADER infoheader;


   //read head and data info from zzz file
   fread(specialKey, 1, 5, inputFile);

   fread(&quality, sizeof(int), 1, inputFile);
   int qualityDivisor = 11 - quality;

   fread(RedFrequency, sizeof(int), 256, inputFile);
   fread(GreenFrequency, sizeof(int), 256, inputFile);
   fread(BlueFrequency, sizeof(int), 256, inputFile);

   fread(&header, sizeof(FILEHEADER), 1, inputFile);
   fread(&infoheader, sizeof(INFOHEADER), 1, inputFile);

   fread(&bytes_used, sizeof(int), 1, inputFile);

   fread(Data, 1, bytes_used, inputFile);

   fclose(inputFile);
    
   int Width = infoheader.biWidth;
   int Height = infoheader.biHeight;
   int Row_Size = (Width) * 3;
   int padding = (4 - (Width * 3) % 4) % 4;
   int Row_total = Row_Size + padding;
   int data_size = Row_total * Height;

   Node* RedArray[256];
   int redCount = 0;

   Node* GreenArray[256];
   int greenCount = 0;

   Node* BlueArray[256];
   int blueCount = 0;

//--------------------------------------------------------------------------------------------------------------------
   //red huffman tree
   //create nodes for reds that have a frequncy
   for(int i = 0; i < 256; i++){
      if(RedFrequency[i] > 0){
         RedArray[redCount] = new Node;
         RedArray[redCount] -> value = i;
         RedArray[redCount] -> frequency = RedFrequency[i];
         RedArray[redCount] -> letter = true;
         RedArray[redCount] -> Right = nullptr;
         RedArray[redCount] -> Left = nullptr;
         redCount++;
      }
   }

   //create parent nodes
   while(redCount > 1){
      //find the 2 smallest frequency
      sort(RedArray, RedArray + redCount, comp);
      
      //create the parent node
      Node* RedParent = new Node;
      RedParent -> value = 0;
      RedParent -> frequency = RedArray[0] -> frequency + RedArray[1] -> frequency;
      RedParent -> letter = false;
      RedParent -> Right = RedArray[1];
      RedParent -> Left = RedArray[0];

      //remove small nodes from list
      for(int i = 2; i < redCount; i++){
         RedArray[i-2] = RedArray[i];
      }
      redCount = redCount - 2;

      //insert parent
      RedArray[redCount] = RedParent;
      redCount++;
   }



//--------------------------------------------------------------------------------------------------------------------
   //Green huffman tree
   //create nodes for greens that have a frequncy
   for(int i = 0; i < 256; i++){
      if(GreenFrequency[i] > 0){
         GreenArray[greenCount] = new Node;
         GreenArray[greenCount] -> value = i;
         GreenArray[greenCount] -> frequency = GreenFrequency[i];
         GreenArray[greenCount] -> letter = true;
         GreenArray[greenCount] -> Right = nullptr;
         GreenArray[greenCount] -> Left = nullptr;
         greenCount++;
      }
   }

   //create parent nodes
   while(greenCount > 1){
      //find the 2 smallest frequency
      sort(GreenArray, GreenArray + greenCount, comp);
      
      //create the parent node
      Node* GreenParent = new Node;
      GreenParent -> value = 0;
      GreenParent -> frequency = GreenArray[0] -> frequency + GreenArray[1] -> frequency;
      GreenParent -> letter = false;
      GreenParent -> Right = GreenArray[1];
      GreenParent -> Left = GreenArray[0];

      //remove small nodes from list
      for(int i = 2; i < greenCount; i++){
         GreenArray[i-2] = GreenArray[i];
      }
      greenCount = greenCount - 2;

      //insert parent
      GreenArray[greenCount] = GreenParent;
      greenCount++;
   }


//--------------------------------------------------------------------------------------------------------------------
   //Blue huffman tree
   //create nodes for greens that have a frequncy
   for(int i = 0; i < 256; i++){
      if(BlueFrequency[i] > 0){
         BlueArray[blueCount] = new Node;
         BlueArray[blueCount] -> value = i;
         BlueArray[blueCount] -> frequency = BlueFrequency[i];
         BlueArray[blueCount] -> letter = true;
         BlueArray[blueCount] -> Right = nullptr;
         BlueArray[blueCount] -> Left = nullptr;
         blueCount++;
      }
   }

   //create parent nodes
   while(blueCount > 1){
      //find the 2 smallest frequency
      sort(BlueArray, BlueArray + blueCount, comp);
      
      //create the parent node
      Node* BlueParent = new Node;
      BlueParent -> value = 0;
      BlueParent -> frequency = BlueArray[0] -> frequency + BlueArray[1] -> frequency;
      BlueParent -> letter = false;
      BlueParent -> Right = BlueArray[1];
      BlueParent -> Left = BlueArray[0];

      //remove small nodes from list
      for(int i = 2; i < blueCount; i++){
         BlueArray[i-2] = BlueArray[i];
      }
      blueCount = blueCount - 2;

      //insert parent
      BlueArray[blueCount] = BlueParent;
      blueCount++;
   }

   //decode binary bits
   // allocate output pixel buffer
   BitReader br(Data);
   BYTE* outData = (BYTE*)malloc(data_size);


   // roots of trees
   Node* RedRoot = RedArray[0];
   Node* GreenRoot = GreenArray[0];
   Node* BlueRoot = BlueArray[0];


   for(int y = 0; y < Height; ++y){
      for(int x = 0; x < Width; ++x){
         int index = y * Row_total + x * 3;

         BYTE R_altered = decoder(RedRoot, br);
         BYTE G_altered = decoder(GreenRoot, br);
         BYTE B_altered = decoder(BlueRoot, br);

         //stretch the colors back out again
         BYTE R = R_altered * qualityDivisor;
         BYTE G = G_altered * qualityDivisor;
         BYTE B = B_altered * qualityDivisor;

         //write them out to the buffer
         outData[index + 2] = R;
         outData[index + 1] = G;
         outData[index + 0] = B;
      }
   }

   FILE* outBmp = fopen(outputfileName, "wb");

   //write header info and data into decompressed bmp
   fwrite(&header.bfType, 2, 1, outBmp);
   fwrite(&header.bfSize, 4, 1, outBmp);
   fwrite(&header.bfReserved1, 2, 1, outBmp);
   fwrite(&header.bfReserved2, 2, 1, outBmp);
   fwrite(&header.bfOffBits, 4, 1, outBmp);

   fwrite(&infoheader, sizeof(INFOHEADER), 1, outBmp);

   fseek(outBmp, header.bfOffBits, SEEK_SET);
   fwrite(outData, 1, data_size, outBmp);
   fclose(outBmp);

   free(outData);
   return 0;
}