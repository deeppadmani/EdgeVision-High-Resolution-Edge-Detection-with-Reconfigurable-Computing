#include "EdgeVision.h"

/* Global variables */
unsigned char input_row[SIZE_BUFFER];
unsigned char output_row;
unsigned char line_buffer[SIZE_BUFFER][SIZE_BUFFER];



void Sobel(unsigned char *input, unsigned char *output, int width, int height, int bytesPerPixel) 
{
    // Input validation
    if (!input || !output || width <= 0 || height <= 0 || bytesPerPixel <= 0) {
        return;
    }

    int Gx[3][3] = {{1, 0, -1},
                    {2, 0, -2},
                    {1, 0, -1}};

    int Gy[3][3] = {{1, 2, 1},
                    {0, 0, 0},
                    {-1, -2, -1}};
                    
    // Clear output buffer first
    memset(output, 0, width * height * bytesPerPixel);

    for (int row = 1; row < height - 1; row++) 
    {
        for (int col = 1; col < width - 1; col++) 
        {
            for (int channel = 0; channel < bytesPerPixel; channel++) 
            {
                int sumX = 0, sumY = 0;
                
                // Calculate index for current pixel's center position
                const int centerIdx = (row * width * bytesPerPixel) + (col * bytesPerPixel) + channel;

                for (int i = -1; i <= 1; i++) 
                {
                    for (int j = -1; j <= 1; j++) 
                    {
                        const int idx = ((row + i) * width * bytesPerPixel) + ((col + j) * bytesPerPixel) + channel;
                        
                        // Bounds check
                        if (idx >= 0 && idx < (width * height * bytesPerPixel)) 
                        {
                            sumX += input[idx] * Gx[i + 1][j + 1];
                            sumY += input[idx] * Gy[i + 1][j + 1];
                        }
                    }
                }

                int magnitude = abs(sumX) + abs(sumY);
                if (magnitude > 255) magnitude = 255;
                
                output[centerIdx] = (255 - (unsigned char)magnitude);
            }
        }
    }
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
    printf("\nLOG: bytesRead :  %d\n", (int)bytesRead);

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
    if (!filename || !bitmapData || !bitmapInfoHeader || !bitmapFileHeader) {
        return;
    }

    FILE *filePtr;
    
    // Calculate the correct bytes per line including padding
    int bytesperline = bitmapInfoHeader->biWidth * (bitmapInfoHeader->biBitCount/8);
    if (bytesperline % 4 != 0) {
        bytesperline = (bytesperline + 3) & ~3;  // Round up to nearest multiple of 4
    }

    // Calculate correct file size
    int headerSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    int paletteSize = bitmapInfoHeader->biClrUsed * 4;
    int imageSize = bytesperline * bitmapInfoHeader->biHeight;

    // Update header information
    bitmapFileHeader->bfType = 0x4D42;  // 'BM'
    bitmapFileHeader->bfSize = headerSize + paletteSize + imageSize;
    bitmapFileHeader->bfReserved = 0;
    bitmapFileHeader->bfReserved2 = 0;
    bitmapFileHeader->bfOffBits = headerSize + paletteSize;

    // Update info header
    bitmapInfoHeader->biSizeImage = imageSize;

    // Open the file
    filePtr = fopen(filename, "wb");
    if (!filePtr) {
        perror("Error opening output BMP file");
        return;
    }

    // Write headers
    fwrite(bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
    fwrite(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

    // Write color palette if present
    if (bitmapInfoHeader->biClrUsed > 0) {
        fwrite(biColourPalette, 4, bitmapInfoHeader->biClrUsed, filePtr);
    }

    // Print debug information
    printf("\nOUTPUT IMAGE DETAILS:\n");
    printf("---------------------\n");
    printf("Size of info header: %d\n", bitmapInfoHeader->biSize);
    printf("Horizontal width: %d\n", bitmapInfoHeader->biWidth);
    printf("Vertical height: %d\n", bitmapInfoHeader->biHeight);
    printf("Bits per pixel: %d\n", bitmapInfoHeader->biBitCount);
    printf("Image size: %d\n", bitmapInfoHeader->biSizeImage);
    printf("Colors used: %d\n", bitmapInfoHeader->biClrUsed);
    printf("----------------------------------------------------------------\n");

    // Allocate buffer for a single line including padding
    unsigned char *lineBuffer = (unsigned char *)calloc(bytesperline, 1);
    if (!lineBuffer) {
        fclose(filePtr);
        return;
    }

    // Write image data line by line
    int bytesPerPixel = bitmapInfoHeader->biBitCount / 8;
    int width = bitmapInfoHeader->biWidth;
    int height = bitmapInfoHeader->biHeight;
    
    for (int y = 0; y < height; y++) {
        // Copy pixel data to line buffer
        for (int x = 0; x < width; x++) {
            for (int b = 0; b < bytesPerPixel; b++) {
                int srcIndex = (y * width * bytesPerPixel) + (x * bytesPerPixel) + b;
                int destIndex = (x * bytesPerPixel) + b;
                lineBuffer[destIndex] = bitmapData[srcIndex];
            }
        }
        // Write the line including padding
        fwrite(lineBuffer, 1, bytesperline, filePtr);
    }

    // Clean up
    free(lineBuffer);
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
    FILE* output_file = freopen("HPS_output.txt", "w", stdout);
    if (output_file == NULL) {
        perror("Error redirecting stdout");
        return;
    }
}
