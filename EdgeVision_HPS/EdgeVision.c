#include "EdgeVision.h"

/* Global variables */
unsigned char input_row[SIZE_BUFFER];
unsigned char output_row;
unsigned char line_buffer[SIZE_BUFFER][SIZE_BUFFER];

void sobel()
{
  unsigned int X, Y;
  int sumX, sumY;
  int SUM, rowOffset, colOffset;

  char Gx[3][3] ={{1 ,0 ,-1},
		  { 2, 0, -2},
		  { 1, 0,-1}};

  char Gy[3][3] ={{1, 2, 1},
		  {0, 0, 0},
		  {-1, -2, -1}};


  /* Shifting line buffer */
   for(Y=2;Y>0;Y--)
   {
        for(X=0;X< 3;X++)
        {
        line_buffer[X][Y-1]=line_buffer[X][Y];
        }
   }

    //Reading new data into the line buffer
    for(X=0; X<SIZE_BUFFER; X++)
         line_buffer[X][2] = input_row[X];

    sumX = 0;
    sumY = 0;

  // Convolution starts here
  //-------X GRADIENT APPROXIMATION------
  //-------Y GRADIENT APPROXIMATION------
   for(rowOffset = -1; rowOffset <= 1; rowOffset++)
   {
        for(colOffset = -1; colOffset <=1; colOffset++)
        {
            sumX = sumX + line_buffer[1 +rowOffset][1-colOffset] * Gx[1+rowOffset][1+colOffset];
            sumY = sumY + line_buffer[1 +rowOffset][1-colOffset] * Gy[1+rowOffset][1+colOffset];
        }
    }

    if(sumX < 0)      sumX = -sumX;
    if(sumX > 255)    sumX = 255;

    if(sumY < 0)      sumY = -sumY;
    if(sumY > 255)    sumY = 255;

    SUM = sumX + sumY;

    if(SUM > 255)    SUM = 255;

    /* Write filter result  to output port */
    output_row =  (unsigned char)(255  - (unsigned char)(SUM));
}


/***********************
 **
 ** Load BMP file into memory
 **
 **********************/
unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader,BITMAPFILEHEADER *bitmapFileHeader)
{

   /* Variables declaration */
    FILE *filePtr;               // out file pointer
    unsigned char *bitmapImage;  // store image data
    size_t bytesRead;
    
    //open filename in read binary mode
    filePtr = fopen(filename,"rb");
    if (filePtr == NULL)
        return NULL;
   
    //read the bitmap file header
    bytesRead = fread(bitmapFileHeader, sizeof(BITMAPFILEHEADER),1,filePtr);
    
    printf("%c bitmap identifies\n",bitmapFileHeader->bfType);
    printf("%d bitmap identifies the size of image\n",bitmapFileHeader->bfSize);
   
    //verify that this is a bmp file by check bitmap id
    if (bitmapFileHeader->bfType !=0x4D42)
    {
        fclose(filePtr);
        return NULL;
    }

    //read the bitmap info header
    bytesRead = fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER),1,filePtr);

    //read colour palette
    bytesRead = fread(&biColourPalette,1,bitmapInfoHeader->biClrUsed*4,filePtr);

    printf("\nIMAGE DETAILS:\n");
    printf("--------------");
    printf("\nSize of info header : %d",bitmapInfoHeader->biSize);
    printf("\nHorizontal width : %x",bitmapInfoHeader->biWidth);
    printf("\nVertical height : %x",bitmapInfoHeader->biHeight);
    printf("\nNum of planes : %d",bitmapInfoHeader->biPlanes);
    printf("\nBits per pixel : %d",bitmapInfoHeader->biBitCount);
    printf("\nCompression specs : %d",bitmapInfoHeader->biCompression);
    printf("\nSize of the image : %d",bitmapInfoHeader->biSizeImage);
    printf("\nThe num of colours used : %x",bitmapInfoHeader->biClrUsed);
    printf("\nThe Bit Offset : %d",bitmapFileHeader->bfOffBits);
    printf("\nBytes per pixel : %d ",bitmapInfoHeader->biBitCount/8);

    //move file point to the begging of bitmap data
    fseek(filePtr, bitmapFileHeader->bfOffBits, SEEK_SET);

      //allocate enough memory for the bitmap image data
    bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

      //verify memory allocation
    if (!bitmapImage)
    {
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }

    //read in the bitmap image data
    bytesRead = fread(bitmapImage,1, bitmapInfoHeader->biSizeImage,filePtr);
    printf("LOG: bytesRead :  %d\n", (int)bytesRead);

    //make sure bitmap image data was read
    if (bitmapImage == NULL)
	{
        printf("Data could not be read");
        fclose(filePtr);
        return NULL;
	}

    fclose(filePtr);
    return bitmapImage;
}

void SaveBitmapFile(char *filename, unsigned char *bitmapData, BITMAPINFOHEADER *bitmapInfoHeader, BITMAPFILEHEADER *bitmapFileHeader) 
{
    FILE *filePtr;
    int bytesperline = 0;
    int k,l;

    k=sizeof(BITMAPFILEHEADER);
    l=sizeof(BITMAPINFOHEADER);

    if(-1 == createDirectory("output")) return;


    bytesperline = bitmapInfoHeader->biWidth * (bitmapInfoHeader->biBitCount/8);
    if( bytesperline & 0x0003)
    { 
        bytesperline |= 0x0003;
        ++bytesperline;
    }

    int byteperpixel = 0;
    byteperpixel = bitmapInfoHeader->biBitCount/8;

    unsigned char *tk;
    tk = (unsigned char *)calloc(1,bytesperline);

    bitmapFileHeader->bfSize= bitmapInfoHeader->biSize + (long)bytesperline* bitmapInfoHeader->biHeight;
    bitmapFileHeader->bfOffBits = k+l+ 4* bitmapInfoHeader->biClrUsed;
    bitmapFileHeader->bfSize = k+l+bitmapInfoHeader->biSizeImage;


    // Open the BMP file for writing
    filePtr = fopen(filename, "wb");
    if (!filePtr) {
        perror("Error opening output BMP file");
        return;
    }


    fwrite(bitmapFileHeader, 1, sizeof(BITMAPFILEHEADER), filePtr);
    fwrite(bitmapInfoHeader, 1,sizeof(BITMAPINFOHEADER), filePtr);
    fwrite(biColourPalette,bitmapInfoHeader->biClrUsed*4,1,filePtr);

    printf("\nOUTPUT IMAGE DETAILS:\n");
    printf("---------------------");
    printf("\nSize of info header : %d",bitmapInfoHeader->biSize);
    printf("\nHorizontal width : %x",bitmapInfoHeader->biWidth);
    printf("\nVertical height : %x",bitmapInfoHeader->biHeight);
    printf("\nNum of planes : %d",bitmapInfoHeader->biPlanes);
    printf("\nBits per pixel : %d",bitmapInfoHeader->biBitCount);
    printf("\nCompression specs : %d",bitmapInfoHeader->biCompression);
    printf("\nSize of the image : %d",bitmapInfoHeader->biSizeImage);
    printf("\nThe num of colours used : %x\n",bitmapInfoHeader->biClrUsed);
    printf("\n%s\n", "----------------------------------------------------------------");

    int i, n = 0,j,x;
    for(x=0; x<  byteperpixel ; x++)
    {
        for(i=0; i<= (bitmapInfoHeader->biHeight-1); i++)
        { 
            for(j=0;j<=bytesperline-1;j++)
            {
                tk[j] = bitmapData[n++];
            }
            // Write pixel data
            fwrite(tk, 1,bytesperline , filePtr);
        }
    }

    fclose(filePtr);
}


int createDirectory(const char *path) {
    struct stat st = {0};

    // Check if directory exists
    if (stat(path, &st) == -1) {
        // Create directory with full permissions
        if (mkdir(path, 0777) == 0) {
            printf("Directory '%s' created successfully\n", path);
            return 0;
        } else {
            printf("Failed to create directory '%s': %s\n", path, strerror(errno));
            return -1;
        }
    }

    printf("Directory '%s' already exists\n", path);

    return 0;
}

void print_image_header(const char* filename) {
    printf("\n%s\n", "================================================================");
    printf("             Sobel Filter Processing: %-30s\n", filename);
    printf("%s\n", "================================================================");
}

void print_footer() {
    printf("\n%s\n", "================================================================");
    
}


void writeOutPutfile()
{
    FILE* output_file = freopen("output.txt", "w", stdout);
    if (output_file == NULL) {
        perror("Error redirecting stdout");
        return;
    }
}