// NDPI Tiler Console.cpp : main project file.
#Copyright (c) 2012 Jake Cobb, David Gutman

#Permission is hereby granted, free of charge, to any person obtaining a copy of
#this software and associated documentation files (the "Software"), to deal in
#the Software without restriction, including without limitation the rights to
#use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
#the Software, and to permit persons to whom the Software is furnished to do so,
#subject to the following conditions:

#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
#FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
#COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
#Copyright (c) 2012   ** Lee Cooper ** , David Gutman, Jake Cobb
#  
#Permission is hereby granted, free of charge, to any person obtaining a copy of
#this software and associated documentation files (the "Software"), to deal in
#the Software without restriction, including without limitation the rights to
#use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
#the Software, and to permit persons to whom the Software is furnished to do so,
#subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
#FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
#COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.



#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <sstream>
#include "C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Include\atl\atlconv.h"
#include "NDPRead.h"
#include "TIFF_utilities.cpp"

#define DUMMYREAD 10 //for dummy read to GetMap to acquire physical coordinates of image center

using namespace System;
using namespace std;

void invertRGB(unsigned char * RGB, long n, long m) {
	//correct inversion (images are written from bottom-up).
	
	unsigned char * strip = (unsigned char *)malloc(3*n);
	for(int i = 0; i < m/2; i++) {
			memcpy(strip, RGB+i*3*n, 3*n); //copy row 'i' to 'strip'
			memcpy(RGB+i*3*n, RGB+(m-i-1)*3*n, 3*n); //copy row 'm-i' to row 'i'
			memcpy(RGB+(m-i-1)*3*n, strip, 3*n); //copy strip to row 'm-i'
	}

	free(strip);
}

void BGRtoRGB(unsigned char * BGR, long bufferlength) {
	//correct channel ordering from ndpi standard (BGR) to RGB.

	unsigned char swap, *tiffbuff = BGR;
	for(int i = 0; i < bufferlength; i+=3) {
		swap = *tiffbuff; //record blue channel
		*tiffbuff = *(tiffbuff+2); //write red channel over blue channel
		*(tiffbuff+2) = swap; //write blue channel over red channel
		tiffbuff = tiffbuff + 3; //skip to next pixel
	}
}

unsigned char * GetThumb(LPWSTR ndpifile, float magnification, long *thn, long *thm) {
	long physX, physY, physW, physH, n, m; //inputs to GetMap
	float nativemag, factor; //native scanning resolution, thumbnail magnification factor scaling
	long result; //result of function calls, to check for successful completion

	//get scan magnification
	nativemag = GetSourceLens(ndpifile);
	if(nativemag == 0) {
		printf("ERROR in GetThumb: could not retrieve scan resolution."); return NULL;
	}

	//get image dimensions at native magnification
	result = GetSourcePixelSize(ndpifile, &n, &m);
	if(!result) {
		printf("ERROR in GetThumb: could not retrieve image dimensions."); return NULL;
	}

	//set camera resolution
	factor = nativemag/magnification;
	long nfactor = (long)(n/factor); //horizontal width must be an integral multiple of 4.
	long mfactor = (long)(m/factor);
	result = SetCameraResolution(nfactor, mfactor);
	if(!result) {
		printf("ERROR in GetThumb: could not set camera resolution."); return NULL;
	}
	
	//extract thumbnail
	long thumbsize = 3*nfactor*mfactor;
	unsigned char * thumb = (unsigned char *)malloc(thumbsize);
	result = GetMap(ndpifile, &physX, &physY, &physW, &physH, thumb, &thumbsize, thn, thm);
	if(!result) {
		printf("ERROR in GetThumb: could extract thumbnail."); return NULL;
	}
	if(thumbsize != 3*mfactor*nfactor) { //allocated buffer 'thumb' not large enough
		free(thumb);
		thumb = (unsigned char *)malloc(thumbsize);
		GetMap(ndpifile, &physX, &physY, &physW, &physH, thumb, &thumbsize, thn, thm);
		if(!result) {
			printf("ERROR in GetThumb: could extract thumbnail."); return NULL;
		}
	}
	if(*thn != nfactor) { //check if we need to strip padding bytes from result
		*thn = nfactor;
	}
			
	//correct ndpi channel order
	BGRtoRGB(thumb, thumbsize);

	//correct inversion
	invertRGB(thumb, *thn, *thm);

	//return corrected buffer containing thumbnail
	return thumb;
}

int main(array<System::String ^> ^args)
{	
	LPWSTR Undpi_in;
	std::string ndpi_in, outdir; //filename+path of ndpi, output directory for tiles
	char outputfile[1024]; //to hold output filename
	size_t slash; //to locate filename and folder
	long tilesize;
	long m, n; //image dimensions (pixels)
	long result; //result of NDP.Read calls, to check for successful completion
	long pm, pn; //image dimensions (nanometers)
	long cx, cy; //center of image (nanometers)
	long mtile, ntile; //tile dimensions (pixels)
	long pmtile, pntile; //tile dimensions (nanometers)
	double xnm, ynm; //nanometers-per-pixel in each dimension
	float nativemag, outmag; //scan magnification	
	char dummy[3*DUMMYREAD*DUMMYREAD]; //dummy variables for acquiring center coordinates using "GetMap"
	long dummysize = 3*DUMMYREAD*DUMMYREAD;
	long dummyn, dummym;

	printf("1\n");

	//convert function argument from System::String to char *
	array<Byte> ^chars = System::Text::Encoding::ASCII->GetBytes(args[0]);
	pin_ptr<Byte> charsPointer = &(chars[0]);
	char *inputfile = reinterpret_cast<char *>(static_cast<unsigned char *>(charsPointer));
	string native(inputfile, chars->Length);

	printf("2\n");

	ifstream input(inputfile, ifstream::in);
	if(!input.is_open()) {
		printf("ERROR in Tiler main: could not open file \"%s\"\n", inputfile);
	}
	else {
		printf("3\n");

		while(!input.eof()) {
			//get line from input file
			input >> ndpi_in >> outdir >> tilesize >> outmag;

			//convert input file from c string to LPWSTR
			USES_CONVERSION;
			Undpi_in = A2W(ndpi_in.c_str());

			//get base image dimensions (pixels)
			result = GetSourcePixelSize(Undpi_in, &n, &m);
			if(!result) {
				printf("ERROR in Tiler main: could not retrieve image dimensions."); return -1;
			}

			//get base image dimensions (nanometers)
			pn = GetImageWidth(Undpi_in);
			pm = GetImageHeight(Undpi_in);
			if(!pm | !pn) {
				printf("ERROR in Tiler main: could not retrieve physical image dimensions."); return -1;
			}

			//get scan resolution
			nativemag = GetSourceLens(Undpi_in);
			if(nativemag == 0) {
				printf("ERROR in Tiler main: could not retrieve scan resolution."); return -1;
			}

			//compare output magnification to scan magnification
			if(outmag > nativemag) {
				printf("ERROR: output magnification > scan magnification.\n");
			}			

			//calculate nanometers-per-pixel
			xnm = ((double)pn)/((double)n);
			ynm = ((double)pm)/((double)m);

			//get center position (physical) of image at scan resolution
			result = SetCameraResolution(DUMMYREAD, DUMMYREAD);
			if(!result) {
				printf("ERROR in Tiler main: could not set camera resolution."); return -1;
			}
			result = GetMap(Undpi_in, &cx, &cy, &pn, &pm, dummy, &dummysize, &dummyn, &dummym);
			if(!result) {
				printf("ERROR in Tiler main: could not acquire center coordinates."); return -1;
			}

			//set ROI size to tilesize x tilesize
			result = SetCameraResolution(tilesize, tilesize);
			if(!result) {
				printf("ERROR in Tiler main: could not set camera resolution."); return -1;
			}

			//output job to console
			printf("Tiling %s, %4.0fX scan mag, %4.0fX output mag %d x %d.\n", ndpi_in.c_str(), nativemag, outmag, n, m);

			//check if output folder exists
			String^ sysoutdir = gcnew String(outdir.c_str());
			if(!System::IO::Directory::Exists(sysoutdir)){
				System::IO::Directory::CreateDirectory(sysoutdir);
			}

			//allocate image tile
			long tilebuff = 3*tilesize*tilesize; //tile buffer size
			unsigned char * tile = (unsigned char *)malloc(3*tilesize*tilesize); //allocate tile

			//tile image
			for(long i = 0; i < (long)(m/(nativemag/outmag)); i+=tilesize) {
				for(long j = 0; j < (long)(n/(nativemag/outmag)); j+=tilesize) {

					//center of tile with upper left corner (i,j)
					long xcenter, ycenter;

					//calculate physical coordinates of tile's center
					xcenter = cx - pn/2 + (long)((j + tilesize/2)*(xnm/(outmag/nativemag)));
					ycenter = cy - pm/2 + (long)((i + tilesize/2)*(ynm/(outmag/nativemag)));
					
					//extract tile
					result = GetImageData(Undpi_in, xcenter, ycenter, 
						1, outmag, &pntile, &pmtile, tile, &tilebuff);
					if(!result) {
						printf("ERROR in Tiler main: could not retrieve extract tile."); return -1;
					}

					//check tilebuff
					if(tilebuff != 3*tilesize*tilesize){
						free(tile);
						tile = (unsigned char *)malloc(tilebuff);

						//extract tile
						result = GetImageData(Undpi_in, xcenter, ycenter, 
							1, outmag, &pntile, &pmtile, tile, &tilebuff);
						if(!result) {
							printf("ERROR in Tiler main: could not retrieve extract tile."); return -1;
						}
					}

					//correct ndpi channel order
					BGRtoRGB(tile, tilebuff);

					//invert tile
					invertRGB(tile, tilesize, tilesize);

					//calculate extracted tile size
					ntile = (long)ceil(pntile/(xnm/((outmag/nativemag))) - 0.5); //round up
					mtile = (long)ceil(pmtile/(ynm/((outmag/nativemag))) - 0.5); //round up
					
					//construct output filename
					if(outdir.rfind("\\") != outdir.length()-1) {  //check if outdir ends in '\', correct if necessary
						outdir.append("\\");
					}
					slash = ndpi_in.find_last_of("/\\"); //get position of slash before filename
					if(slash != ndpi_in.npos) { //path was provided, trim ndpi_in to filename-only
						ndpi_in = ndpi_in.substr(slash+1, ndpi_in.length()-slash);	
					}
					sprintf(outputfile, "%s%s-%010d-%010d.tif", outdir.c_str(), ndpi_in.c_str(), i, j);

					//write to file
					TIFFwriteRGB(tile, outputfile, ntile, mtile);

					//output to console
					printf("\twrote %s\n", outputfile);
				}
			}

			//free tile
			free(tile);

			//clean up internal NDP.Read structures
			CleanUp();

			//output to console
			printf("Tiling %s complete.\n", ndpi_in.c_str());
		}
	}
	input.close();

	return 0;
}
