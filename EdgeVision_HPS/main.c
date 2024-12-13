/************************************************************************
 **
** SOEBEL Filter
**
** Created: Dec 11, 2024
** D&Slab
**
***********************************************************************/

#include "EdgeVision.h"

extern unsigned char input_row[];
extern unsigned char output_row;
/**************************
**************************
**
**   MAIN 
**
*************************
************************/

int main(int argc, char* argv[])
{
  if (argc > 5 || argc < 3 ||
  (strcmp("-o",argv[1]) != 0 &&
  strcmp("-w",argv[1]) != 0))
  {
    print_footer();
    printf("Error: Program accepts minimum 1 and maximum 3 input files\n");
    printf("Usage: %s -o/-w input1.bmp [input2.bmp input3.bmp]\n", argv[0]);
    printf("Example: %s -o/-w image.bmp\n", argv[0]);
    printf("Example: %s -o/-w image1.bmp image2.bmp image3.bmp\n", argv[0]);
    print_footer();
    return 1;
  }

  if(strcmp("-o",argv[1]) == 0)
    writeOutPutfile();

  createDirectory("output");

  int totalImg;
  totalImg = 2;
  double total_cpu_time_used = 0;
  while(totalImg < argc)
  {
      print_image_header(argv[totalImg]);

      int COLS, ROWS, BYTES_PER_PIXEL;
      clock_t start, end;
      double cpu_time_used;
      start = clock();


      size_t inputLen = strlen(argv[totalImg]);
      if (inputLen < 4) 
      {
        printf("Invalid input filename\n");
        return 1;
      }
      // Get the base filename
      char *baseFileName = argv[totalImg];
      char *lastSlash = strrchr(baseFileName, '/');
      if (lastSlash != NULL) {
          baseFileName = lastSlash + 1;  // Skip the last slash to get just the filename
      }
      size_t baseNameLen = strlen(baseFileName);
      char outputFileName[baseNameLen + 30];
      if (snprintf(outputFileName, sizeof(outputFileName), "output/%.*s_HPSoutput.bmp",
        (int)(baseNameLen - 4), baseFileName) >= sizeof(outputFileName))
      {
        printf("Output filename too long\n");
        return 1;
      }
      BITMAPINFOHEADER bitmapInfoHeader;
      BITMAPFILEHEADER bitmapFileHeader; //our bitmap file header
      unsigned char *bitmapData;
      unsigned char *bitmapFinalImage;
      bitmapData = LoadBitmapFile(argv[totalImg],&bitmapInfoHeader, &bitmapFileHeader);

      if(NULL == bitmapData)
      {
        printf("No image found!\n");
        return 1;
      }


      BYTES_PER_PIXEL = bitmapInfoHeader.biBitCount / 8;
      COLS = bitmapInfoHeader.biWidth;
      ROWS = bitmapInfoHeader.biHeight;

      bitmapFinalImage = (unsigned char*)malloc(ROWS * COLS * BYTES_PER_PIXEL);
      if (!bitmapFinalImage) {
          // Handle allocation failure
          return 0;
      }
      Sobel(bitmapData, bitmapFinalImage, COLS, ROWS, BYTES_PER_PIXEL);
   
      SaveBitmapFile(outputFileName, bitmapFinalImage, &bitmapInfoHeader, &bitmapFileHeader);

      // Clean up
      free(bitmapData);
      free(bitmapFinalImage);

      end = clock();
      cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
      total_cpu_time_used += cpu_time_used;
      totalImg++;
      printf("Runtime: %f seconds for %s\n", cpu_time_used, baseFileName);
      printf("Total Runtime: %f seconds", total_cpu_time_used);
      printf("\n%s\n", "----------------------------------------------------------------");
  }
  return 0;
}


