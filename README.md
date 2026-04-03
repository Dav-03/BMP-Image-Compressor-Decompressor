# BMP-Image-Compressor-Decompressor
**Overview**

This project is a C++ command-line tool that compresses and decompresses 24-bit BMP images using per-channel Huffman coding. It demonstrates low-level systems programming concepts including bit manipulation, custom binary formats, and memory management.

The compressor supports a quality parameter (1–10) that allows trade-offs between compression ratio and image fidelity, ranging from lossy to near-lossless compression.

**Features**
-  Huffman compression applied separately to Red, Green, and Blue channels
-  Custom binary file format (.zzz) for storing compressed data and metadata
-  Adjustable quality setting for compression control
-  Bit-level encoding and decoding (manual bit packing)
-  Full image reconstruction via decompression
-  Low-level memory handling using malloc and mmap

**How It Works**
1. Read BMP file headers and pixel data
2. Apply quality scaling to reduce color precision
3. Build frequency tables for each color channel
4. Construct Huffman trees based on frequency distributions
5. Generate binary codes via DFS traversal
6. Encode pixel data into a compressed bitstream
7. Store compressed data along with metadata (headers + frequency tables)

During decompression:

-  Huffman trees are rebuilt from stored frequency tables
-  Bitstream is decoded back into pixel values
-  Colors are rescaled based on the quality setting
-  Original BMP structure is reconstructed

**Build Instructions**

Compile using g++:

g++ compress.cpp -o compress
g++ decompress.cpp -o decompress

**Usage**

**Compress an image**

./compress input.bmp <quality>
-  quality: Integer from 1 to 10
  -  Lower = higher compression (more loss)
  -  Higher = better quality (less loss)

**Decompress an image**

./decompress newfile.zzz output.bmp

**Example**

./compress flowers.bmp 5

./decompress newfile.zzz output.bmp

**File Format**

The custom .zzz file contains:
-  File identifier ("DAV1D")
-  Compression quality
-  Frequency tables for R, G, B channels
-  Original BMP headers
-  Bitstream size
-  Compressed pixel data

**Technologies Used**

-  C++ (Systems Programming)
-  File I/O and binary parsing
-  Huffman Coding
-  Bit-level data encoding
-  Memory management (malloc, mmap)

**Key Concepts Demonstrated**

-  Lossy vs. lossless compression trade-offs
-  Data structures (trees, frequency tables)
-  Recursive traversal (DFS for code generation)
-  Custom serialization/deserialization
-  Low-level performance-oriented programming

**Future Improvements**

-  Support for additional image formats (PNG, JPEG)
-  Parallel compression for performance
-  Improved compression ratio using advanced techniques
-  Visualization of compression statistics

**Author**

David Morfin
