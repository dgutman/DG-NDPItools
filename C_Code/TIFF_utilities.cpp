#include "C:\Program Files\GnuWin32\include\tiff.h"
#include "C:\Program Files\GnuWin32\include\tiffio.h"
#include <math.h>
typedef unsigned char uint8;

uint8* RGBA2gray(uint32* image, uint32 npixels, double r, double g, double b) {
	//accepts an RGBA image 'image' and returns the 8-bit grayscale result,
	//scaled according to 'r', 'g', and 'b'.  Typical values are r=0.2989, 
	//g=0.5870, b=0.1140.
	
	uint8* gray = (uint8*) malloc(npixels * sizeof (uint8)); //allocate grayscale image array.
	
	if(gray != NULL) {
		uint32* RGBA_ptr = image; //point to first element in RGBA image.
		uint8* gray_ptr = gray;
		uint32 i;

		for(i = 0; i < npixels; i++) {
			*gray_ptr = (uint8)(r*TIFFGetR(*RGBA_ptr) + g*TIFFGetG(*RGBA_ptr) + b*TIFFGetB(*RGBA_ptr));
			gray_ptr++;
			RGBA_ptr++;
		}
		return(gray);
	}
	else {
		return(NULL);
	}
}

uint8* RGB2gray(uint8* image, size_t npixels, double r, double g, double b) {
	//accepts an RGBA image 'image' and returns the 8-bit grayscale result,
	//scaled according to 'r', 'g', and 'b'.  Typical values are r=0.2989, 
	//g=0.5870, b=0.1140.
	
	uint8* gray = (uint8*) malloc(npixels * sizeof (uint8)); //allocate grayscale image array.
	
	if(gray != NULL) {
		uint8* RGB_ptr = image; //point to first element in RGBA image.
		uint8* gray_ptr = gray;
		uint32 i;

		for(i = 0; i < npixels; i++) {
			*gray_ptr = (uint8)(r*(*RGB_ptr) + g*(*(RGB_ptr+1)) + b*(*(RGB_ptr+2)));
			gray_ptr++;
			RGB_ptr+=3;
		}
		return(gray);
	}
	else {
		return(NULL);
	}
}

uint8* gray2BW(uint8* image, uint32 width, uint32 height, uint8 thresh) {
	//accepts a grayscale image 'image', thresholds the intensity and returns the 1-bit binary result.
	//width and height are required as input since each image row contains extra bits if the width is not evenly 
	//divisible by 8 (due to the way TIFF encodes BW images).

	uint8* bw = (uint8*) malloc(height*(uint32)ceil(width/8.0)*sizeof(uint8)); //8 pixels per byte

	if(bw != NULL) {
		uint8* grayptr = image;
		uint8* bwptr = bw;

		uint32 i, j;
		for (i = 0; i < height; i++) { //for each row
			for (j = 0; j < width; j++) { //for each pixel in current row
				*(bwptr + j/8) <<= 1; //shift byte contents left one bit.
				if(*grayptr >= thresh) *(bwptr + j/8) |= 01; //threshold met.  introduce '1'.
				grayptr++;
			}

			*(bwptr + j/8) <<= (8*(uint32)ceil(width/8.0)-width); //shift zeros in for remainder pixels (when 8 div width != 0).
			bwptr+= (uint32)ceil(width/8.0);
		}
		return(bw);
	}
	else {
		return(NULL);
	}
}

uint8* dec_RGB(uint8* image, uint32 width, uint32 height, uint32 factor) {
	//accepts an RGB image 'image' and decimates by 'factor' in each direction.
	//returns decimated image which is width/factor-height/factor (integer math).
	//user is expected to choose reasonable values for factor

	uint8* dec_RGB = (uint8*) malloc(3*(width/factor)*(height/factor));

	if(dec_RGB != NULL) { //check if allocation was successful.
		uint8* dec_ptr = dec_RGB;

		for(uint32 i = 0; i < height - height%factor; i += factor) {
			for(uint32 j = 0; j < width - width%factor; j += factor) {
				*(dec_ptr++) = image[3*(i*width + j)]; //red channel
				*(dec_ptr++) = image[3*(i*width + j)+1]; //green channel
				*(dec_ptr++) = image[3*(i*width + j)+2]; //blue channel
			}
		}
		
		return(dec_RGB);
	}
	else {
		return(NULL);
	}
}

uint32* TIFFreadRGBA(const char* filename, uint32 & width, uint32 & height) {
    TIFF* in = TIFFOpen(filename, "r");
    if (in) {
        size_t npixels;
        uint32* raster;

        TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);

        npixels = width * height;

        raster = (uint32*) malloc(npixels * sizeof (uint32));
        if (raster != NULL ) {	//memory was allocated by system
            if (TIFFReadRGBAImageOriented(in, width, height, raster, ORIENTATION_TOPLEFT, 0)) {	//read was successful, free raster
				TIFFClose(in);
				return(raster);
            }
			else { //read was unsuccessfull
				printf("Read failed.\n");

				TIFFClose(in);
				free(raster);
				return(NULL);
			}
        }
		else {
			printf("Could not allocate sufficient memory for read.  Read aborted.\n"); 

			TIFFClose(in);
			return(NULL);
		}
    }
	else {
		return(0);
	}
}

uint8* TIFFreadRGB(const char* filename, uint32 & width, uint32 & height) {
    TIFF* in = TIFFOpen(filename, "r");
    if (in) {
        size_t npixels;
        uint8* raster;

        TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);

        npixels = width * height;

        raster = (uint8*) malloc(3 * npixels);

        if (raster != NULL ) {	//memory was allocated by system
			for (size_t i = 0; i < TIFFNumberOfStrips(in);  i++) {
				TIFFReadEncodedStrip(in, i, raster+i*TIFFStripSize(in), (tsize_t) -1);
			}

			return(raster);
        }
		else {
			printf("Could not allocate sufficient memory for read.  Read aborted.\n"); 

			TIFFClose(in);
			return(NULL);
		}
    }
	else {
		return(0);
	}
}

uint8* TIFFreadgray(const char* filename, uint32 & width, uint32 & height) {
	TIFF* in = TIFFOpen(filename, "r");
	if(in) {
		size_t npixels;
		uint8* raster;

		TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
		
		npixels = width * height;

		raster = (uint8*) malloc(3 * npixels);

		if(raster != NULL) {	//image memory was allocated
			for (size_t i = 0; i < TIFFNumberOfStrips(in); i++) {
				TIFFReadEncodedStrip(in, i, raster+i*TIFFStripSize(in), (tsize_t) -1);
			}
			return(raster);
		}
		else {
			printf("Could not allocate sufficient memory for read.  Read aborted.\n"); 

			TIFFClose(in);
			return(NULL);
		}
	}
	else {
		return(0);
	}
}

int TIFFwriteRGBA2RGB(uint32* image, const char* filename, uint32 width, uint32 height) {
	TIFF* out = TIFFOpen(filename, "w");

	TIFFSetField (out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 3);   // set number of channels per pixel
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB); 
	TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(out, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	
	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, height); //entire image is one strip, alternatively strip smaller to improve previewer performance

    int	pixel_count = width * height; //strip alpha component from each pixel
    unsigned char *src, *dst;
    src = (unsigned char *) image;
    dst = (unsigned char *) image;
    while( pixel_count > 0 ) {
        *(dst++) = *(src++);
        *(dst++) = *(src++);
        *(dst++) = *(src++);
        src++;
        pixel_count--;
    }

	TIFFWriteEncodedStrip(out, 0, image, 3 * height * width);

	//
	//check if returned -1
	//

	TIFFClose(out);

	return(1);
}

int TIFFwriteRGB(uint8* image, const char* filename, uint32 width, uint32 height) {
	TIFF* out = TIFFOpen(filename, "w");

	TIFFSetField (out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 3);   // set number of channels per pixel
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	TIFFSetField(out, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, 1); //entire image is one strip, alternatively strip smaller to improve previewer performance

	for (uint32 i = 0; i < height;  i++) {
		TIFFWriteEncodedStrip(out, i, image + 3*i*width, 3*width);
	}

	TIFFClose(out);

	return(1);
}

int TIFFwritegray(uint8* image, const char* filename, uint32 width, uint32 height) {
	TIFF* out = TIFFOpen(filename, "w");

	TIFFSetField (out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 1);   // set number of channels per pixel
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	//TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	//TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(out, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, height); //entire image is one strip, alternatively strip smaller to improve previewer performance

	TIFFWriteEncodedStrip(out, 0, image, height * width);
	//
	//check if returned -1
	//

	TIFFClose(out);

	return(1);
}

int TIFFwriteBW(uint8* image, const char* filename, uint32 width, uint32 height) {
	TIFF* out = TIFFOpen(filename, "w");

	TIFFSetField (out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 1);   // set number of channels per pixel
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 1);    // set the size of the channels
	TIFFSetField(out, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);  //bit ordering (multiple pixels per byte)

	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

	TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_CCITTRLE);
	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, height); //entire image is one strip, alternatively strip smaller to improve previewer performance

	int result = TIFFWriteEncodedStrip(out, 0, image, height*(uint32)ceil(width/8.0)); //8 pixels per byte

	TIFFClose(out);

	return(1);
}

/*uint32* rigidwarpRGBA(uint32* in, uint32 m, uint32 n, double theta, int x_delta, int y_delta, uint32 background) {
	//produces output image (return value) sized m x n, that is the rigid warping of 'in' by parameters
	//theta, x_delta, y_delta.
	//'x_delta' and 'y_delta' parameters are defined in the more intuitive way, not as raw offsets over the whole 
	//Euclidean plane (+/-) but as pixel offset croppings.
	
	uint32* out = (uint32*) malloc(m * n * sizeof (uint32)); //allocate output image

	int i, j; //iterators over output image space

	int x_off = n/2 - (int)(cos(theta)*(double)(n/2) + sin(theta)*(double)(m/2)); //center offsets.
	int y_off = -(m/2) - (int)(sin(theta)*(double)(n/2) - cos(theta)*(double)(m/2));

	for (i = 0; i < m; i++) { //for each row in output space
		for (j = 0; j < n; j++) { //for each column in output space			
			int x_in = (int)(cos(theta)*(j - x_off) + sin(theta)*(-i - y_off) - x_delta); //row coordinate of corresponding input pixel
			int y_in = (int)(-sin(theta)*(j - x_off) + cos(theta)*(-i - y_off) - y_delta); //column coordinate of corresponding input pixel

			if (x_in < 0 | x_in > n-1 | y_in > 0 | y_in < -(m-1)) { //pixel maps to input image
				out[i*(n) + j] = background;
			}
			else { //fill in background value
				out[i*(n) + j] = in[-y_in*(n) + x_in];
			}
		}
	}
	return(out);
}*/
