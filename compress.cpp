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


// int quality = 5;
// int qualityDivisor = 11 - quality;

//frequency tables
int RedFrequency[256];
int GreenFrequency[256];
int BlueFrequency[256];
 

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

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

//struct for the binary code and lenth of them
struct binaryPattern
{
   LONG pattern;
   LONG length;
};

//binary code storage
binaryPattern* RedBinaryCodes[256];
binaryPattern* GreenBinaryCodes[256];
binaryPattern* BlueBinaryCodes[256];

//function for the sort
bool comp(Node* a, Node* b){
   return a -> frequency < b -> frequency;
}

//recursive DFS to determine binary code
void DFS(Node* node, LONG bit, LONG length, binaryPattern* binaryCodes[256]){
   if(!node){
      return;
   }

   if(node -> letter){
      //checks for single colors images
      if(length == 0){
         binaryCodes[node->value] -> pattern = 0;
         binaryCodes[node->value] -> length = 1;
      }
      else{
         binaryCodes[node->value] -> pattern = bit;
         binaryCodes[node->value] -> length = length;
      }
      return;
   }

   DFS(node->Left, bit << 1, length + 1, binaryCodes);
   
   DFS(node->Right, (bit << 1) | 1, length + 1, binaryCodes);
}


class bp{
   public:
   //buffer stores the concatenated binary codes
   int position = 0;
   BYTE Data[1000000] = {0};

   //pushes bits onto bit stream
   void push(BYTE bit){
      int bytepos = position / 8;
      int pos_in_byte = position - bytepos * 8;
      bit <<= 7 - pos_in_byte;
      Data[bytepos] |= bit;
      position++;
   }
};

//writes an entire pattern and length into the buffer
void concatenateCodes(bp& buffer, LONG Pattern, int Length){
   for(int i = Length - 1; i >= 0; i--){
      BYTE bit = (Pattern >> i) & 1;
      buffer.push(bit);
   }

}

int main(int argc, char* argv[]){

   const char* inputfileName = argv[1];
   int quality = atoi(argv[2]);

   int qualityDivisor = 11 - quality;
   
   FILE* inputFile = fopen(inputfileName, "rb");
    
   struct FILEHEADER header;
   struct INFOHEADER infoheader;

   fread(&header.bfType, 2, 1, inputFile);
   fread(&header.bfSize, 4, 1, inputFile);
   fread(&header.bfReserved1, 2, 1, inputFile);
   fread(&header.bfReserved2, 2, 1, inputFile);
   fread(&header.bfOffBits, 4, 1, inputFile);
   fread(&infoheader, sizeof(struct INFOHEADER), 1, inputFile);
    
   int Width = infoheader.biWidth;
   int Height = infoheader.biHeight;
   int Row_Size = (Width) * 3;
   int padding = (4 - (Width * 3) % 4) % 4;
   int Row_total = Row_Size + padding;
   int data_size = Row_total * Height;

   //current pixel data
   BYTE* Data = (BYTE*)malloc(data_size);
   fseek(inputFile, header.bfOffBits, SEEK_SET);
   fread(Data, 1, data_size, inputFile);
   fclose(inputFile);

   //this is where were going to store the final pixel data
   BYTE* finalData = (BYTE*)mmap(NULL, data_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

   //fill the frequency array with zeros
   for(int i = 0; i < 256; i++){
      RedFrequency[i] = 0;
      GreenFrequency[i] = 0;
      BlueFrequency[i] = 0;
   }
   
   for(int y = 0; y < Height; y++){
      for(int x = 0; x < Width; x++){
         int index = y * Row_total + x * 3;
         BYTE R = (BYTE) Data[index + 2];
         BYTE G = (BYTE) Data[index + 1];
         BYTE B = (BYTE) Data[index + 0];

         BYTE R_altered = R / qualityDivisor;
         BYTE G_altered = G / qualityDivisor;
         BYTE B_altered = B / qualityDivisor;

         RedFrequency[R_altered]++;
         BlueFrequency[B_altered]++;
         GreenFrequency[G_altered]++;

         finalData[index + 2] = R;
         finalData[index + 1] = G;
         finalData[index + 0] = B + 125;
      }
   }

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

   //initialize for the red binary codes
   for(int i = 0; i < 256; i++){
      RedBinaryCodes[i] = new binaryPattern;
      RedBinaryCodes[i] -> length = 0;
      RedBinaryCodes[i] -> pattern = 0;
   }

   //make huffman codes
   if(redCount == 1){
      DFS(RedArray[0], 0, 0, RedBinaryCodes);
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

   //initialize for the green binary codes
   for(int i = 0; i < 256; i++){
      GreenBinaryCodes[i] = new binaryPattern;
      GreenBinaryCodes[i] -> length = 0;
      GreenBinaryCodes[i] -> pattern = 0;
   }

   //make huffman codes
   if(greenCount == 1){
      DFS(GreenArray[0], 0, 0, GreenBinaryCodes);
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

   //initialize for the blue binary codes
   for(int i = 0; i < 256; i++){
      BlueBinaryCodes[i] = new binaryPattern;
      BlueBinaryCodes[i] -> length = 0;
      BlueBinaryCodes[i] -> pattern = 0;
   }

   //make huffman codes
   if(blueCount == 1){
      DFS(BlueArray[0], 0, 0, BlueBinaryCodes);
   }
//--------------------------------------------------------------------------------------------------------------------
   bp compressed;

   for(int y = 0; y < Height; y++){
      for(int x = 0; x < Width; x++){
         int index = y * Row_total + x * 3;
         BYTE R = (BYTE) Data[index + 2];
         BYTE G = (BYTE) Data[index + 1];
         BYTE B = (BYTE) Data[index + 0];

         BYTE R_altered = R / qualityDivisor;
         BYTE G_altered = G / qualityDivisor;
         BYTE B_altered = B / qualityDivisor;

         concatenateCodes(compressed, RedBinaryCodes[R_altered] -> pattern, RedBinaryCodes[R_altered] -> length);
         concatenateCodes(compressed, GreenBinaryCodes[G_altered] -> pattern, GreenBinaryCodes[G_altered] -> length);
         concatenateCodes(compressed, BlueBinaryCodes[B_altered] -> pattern, BlueBinaryCodes[B_altered] -> length);
         
         
      }
   }

   //bytes used in total
   int bytes_used = (compressed.position + 7) / 8;
   

   //write out to file 
   FILE* output = fopen("newfile.zzz", "wb");

   //file identifier
   fwrite("DAV1D", 1, 5, output);

   //compression quality
   fwrite(&quality, sizeof(int), 1, output);

   //frequency tables
   fwrite(RedFrequency, sizeof(int), 256, output);
   fwrite(GreenFrequency, sizeof(int), 256, output);
   fwrite(BlueFrequency, sizeof(int), 256, output);

   //images file headers
   fwrite(&header, sizeof(FILEHEADER), 1, output);
   fwrite(&infoheader, sizeof(INFOHEADER), 1, output);

   //bitstream size
   fwrite(&bytes_used, sizeof(int), 1, output);

   //compressed pixel data
   fwrite(compressed.Data, 1, bytes_used, output);
   fclose(output);

   //print the frequency tables
   // for(int i = 0; i < 256; i++){
   //    printf("%d\n", RedFrequency[i]);
   // }



   free(Data);
   munmap(finalData, data_size);
   return 0;
}



