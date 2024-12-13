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


static volatile uint32_t *pixel_in_pio = NULL;
static volatile uint8_t *pixel_out_pio = NULL;
static void *lw_bridge_base = NULL;
static int fd = -1;

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

  int k,i,j,totalImg;
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
      char outputFileName[baseNameLen + 24];
      if (snprintf(outputFileName, sizeof(outputFileName), "output/%.*s_output.bmp",
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
      COLS = bitmapInfoHeader.biWidth * BYTES_PER_PIXEL;
      ROWS = bitmapInfoHeader.biHeight * BYTES_PER_PIXEL;


      printf("bytes per pixel : %d \n",BYTES_PER_PIXEL);
      bitmapFinalImage = (unsigned char*)malloc(ROWS*COLS);

      i=0;
      j=0;

      for(k=0;k<BYTES_PER_PIXEL;k++)
      {
        for(i=0;i<ROWS;i++)
        {
          for(j=0;j<COLS;j+=BYTES_PER_PIXEL)
          {
            // Calculate indices first
            int prev_index = ((i - 1) * COLS) + j + k;
            int curr_index = (i * COLS) + j + k;
            int next_index = ((i + 1) * COLS) + j + k;

            // Add bounds checking
            if (prev_index >= 0 && prev_index < bitmapInfoHeader.biSizeImage &&
              curr_index >= 0 && curr_index < bitmapInfoHeader.biSizeImage &&
              next_index >= 0 && next_index < bitmapInfoHeader.biSizeImage)
            {
              input_row[0] = (i == 0) ? 0 : bitmapData[prev_index];
              input_row[1] = bitmapData[curr_index];
              input_row[2] = (i == ROWS - 1) ? 0 : bitmapData[next_index];
            }
            else
            {
              // Handle edge case
              input_row[0] = 0;
              input_row[1] = 0;
              input_row[2] = 0;
            }
            sobel();
            bitmapFinalImage[curr_index] = output_row;
          }
        }
        input_row[0]=0;
        input_row[1]=0;
        input_row[2]=0;
      }
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


