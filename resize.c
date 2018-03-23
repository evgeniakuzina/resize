#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize int infile outfile\n");
        return 1;
    }
    
    int n = atoi(argv[1]);
    
    if (n < 0 || n > 100)
    {
        fprintf(stderr, "Integer should be between 0 and 100\n");
        return 2;
    }
    
     // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 3;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 4;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 5;
    }

     // determine padding for scanlines
    int padding_infile =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int padding_outfile =  (4 - ((bi.biWidth*n) * sizeof(RGBTRIPLE)) % 4) % 4;
    
    int old_width = bi.biWidth;
    int old_height = bi.biHeight;
    
    bi.biWidth = old_width*n;
    bi.biHeight = old_height*n;
    bi.biSizeImage = ((sizeof(RGBTRIPLE)*bi.biWidth)+padding_outfile)*abs(bi.biHeight);
    bf.bfSize = bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // iterate over infile's scanlines
    for (int i = 0, old_height_abs = abs(old_height); i < old_height_abs; i++)
    {
        for (int b = 0; b < n; b++)
        {
            fseek(inptr, (54 + ((old_width*3 + padding_infile) * i)), SEEK_SET);
            
            // iterate over pixels in scanline
            for (int j = 0; j < old_width; j++)
            {
                // temporary storage
                RGBTRIPLE triple;
             
                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
                    
                for (int c = 0; c < n; c++)
                {
                    // write RGB triple to outfile
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
    
            }    
            
            //skip over padding, if any
            fseek(inptr, padding_infile, SEEK_CUR);

            // then add it back (to demonstrate how)
            for (int k = 0; k < padding_outfile; k++)
            {
                fputc(0x00, outptr);
            }
            
        }      
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
