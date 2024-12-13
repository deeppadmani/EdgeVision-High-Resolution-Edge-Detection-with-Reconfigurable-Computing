#ifndef EDGEVISION_H
#define EDGEVISION_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include "hps_0.h"  // Include the hps_0.h header
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"

unsigned char biColourPalette[1024];

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef char BYTE;

#pragma pack(push,1)
typedef struct {
  WORD  bfType;                       // The type of the image
 
  DWORD bfSize;                       //size of the file
  WORD  bfReserved;                 // reserved type
  WORD  bfReserved2;               
  DWORD bfOffBits;                   //offset bytes from the file header to the image data
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;                   //the size of the header
  LONG  biWidth;                 //the width in pixels
  LONG  biHeight;                //the height in pixels
  WORD  biPlanes;                //the no. of planes in the bitmap
  WORD  biBitCount;              //bits per pixel
  DWORD biCompression;           //compression specifications
  DWORD biSizeImage;            //size of the bitmap data
  LONG  biXPelsPerMeter;        //horizontal res(pixels per meter)
  LONG  biYPelsPerMeter;        //vertical res(pixels per meter) 
  DWORD biClrUsed;              //colours used in the image
  DWORD biClrImportant;        //num of important colours used
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack(pop)


#define SIZE_BUFFER 3

// Base addresses as defined in the header file
#define LW_BRIDGE_BASE 0xFF200000  // Lightweight HPS-to-FPGA Bridge base address
#define LW_BRIDGE_SPAN 0x200000    // Lightweight bridge span

#define PIXEL_IN_PIO_BASE 0x50000  // Offset for pixel_in_pio
#define PIXEL_OUT_PIO_BASE 0x40000 // Offset for pixel_out_pio

unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader,BITMAPFILEHEADER *bitmapFileHeader);
void SaveBitmapFile(char *filename, unsigned char *bitmapData, BITMAPINFOHEADER *bitmapInfoHeader, BITMAPFILEHEADER *bitmapFileHeader);
int createDirectory(const char *path);
void print_image_header(const char* filename);
void print_footer();
void writeOutPutfile();
uint32_t prepareDataforTx(uint8_t *inputData, uint8_t size);
int configure_fpga();
void write_to_fpga(uint32_t data);
uint8_t read_from_fpga();
void cleanup_fpga();

#endif /* EDGEVISION_H */