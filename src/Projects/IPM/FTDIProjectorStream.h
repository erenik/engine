/// Emil Hedemalm
/// 2014-03-27
/** Stream-buffering class for extracting image data from an FTDI chip coupled to a projector/camera device.
	Original C-based code by Felix Schmidt.
	FTDI: http://www.ftdichip.com/
	API: http://www.dlpdesign.com/drivers/D2XXPG21.pdf
*/


#ifdef WINDOWS

//==============================================================================
// Include Files

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <unistd.h>
#include <time.h>
#include <stdint.h>
// Windows includes... not good! o.o
// #include <windows.h>
// #include <minwindef.h>

// #include "ftdi/ftd2xx.h"
#include "opencv2/core/types_c.h"

//==============================================================================
// Constants

#define MAX_FRAME_SIZE 1048576 // = 1MiB
#define HEADER_SIZE 22 //2*5 intro/outro magic bytes + 2*6 image properties

#define INTREG_CAPTEN 0
#define INTREG_WIDTH 1
#define INTREG_HEIGHT 2
#define INTREG_WIDOFF 3
#define INTREG_HEIOFF 4
#define INTREG_SKIPLIN 5
#define INTREG_SKIPFR 6
#define INTREG_SAMDIV 7
#define INTREG_SAMCHAN 8
#define INTREG_SAMORC 9
#define INTREG_SAMORCEN 10
#define INTREG_WRITEEN 11
#define INTREG_SHOT 12

//==============================================================================
// Types

typedef FT_HANDLE IiP_HANDLE;

typedef struct _IiP_configfile
{
    unsigned short width;           // width of image
	unsigned short height;          // height of image
	unsigned short widthOffset;     // image offset from lateral edge
	unsigned short heightOffset;    // image offset from upper edge
	unsigned short skippedLines;    // lines to skip
	unsigned short skippedFrames;   // frames to skip
	unsigned short sampleDivider;    // sample clock divider:
	                                // 0 => SR= 150MHz
	                                // 1-15 => SR= 0.5 * 150 / sampleDivider
	unsigned short sampleChannel;    // sampleChannel
                                    // 0 => A-Channel
		                            // 1 => B-Channel
	unsigned short sampleORcolorEn;  // enables color of out-of-range-pixels
	unsigned short sampleORcolor;    // sampleORcolor 0 => black; 1 => white
}
IiP_configfile;

typedef union
{
	unsigned int uint; 			// 32Bit
	struct
    {
		unsigned char b0:   8;
		unsigned char b1:   8;
		unsigned char b2:   8;
		unsigned char b3:   8;
    }BYTE;
}BYTE_2_UINT;

typedef union
{
	WORD wordw; 			// 16Bit
	struct
    {
		unsigned char b0:   8;
		unsigned char b1:   8;
    }BYTE;
}BYTE_2_WORD;

typedef union
{
	DWORD dwordw; 			// 32Bit
	struct
    {
		unsigned char b0:   8;
		unsigned char b1:   8;
		unsigned char b2:   8;
		unsigned char b3:   8;
    }BYTE;
}BYTE_2_DWORD;

typedef union
{
	long longw; 			// 32Bit
	struct
    {
		unsigned char b0:   8;
		unsigned char b1:   8;
		unsigned char b2:   8;
		unsigned char b3:   8;
    }BYTE;
}BYTE_2_LONG;

enum IiP_FRAME { A, B};

//==============================================================================
// Globals

/*extern unsigned char frameA[MAX_FRAME_SIZE];
extern unsigned char frameB[MAX_FRAME_SIZE];
extern IiP_FRAME frameToCapture;
extern IiP_FRAME frameToProcess;
*/
//==============================================================================
// Prototypes

extern IiP_HANDLE IiP_init(void);
extern        int IiP_register(IiP_HANDLE iipHandle, unsigned char address, unsigned short value);
extern        int IiP_close(IiP_HANDLE iipHandle);
extern        int IiP_shot(IiP_HANDLE iipHandle);
extern      char* IiP_capture(IiP_HANDLE iipHandle, char* frame_ptr, int frameSize);
extern        int IiP_config(IiP_HANDLE iipHandle, IiP_configfile iipConfig);



// Main.cpp from the original project. 

//#include "stdafx.h"
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2\imgproc\imgproc_c.h"
//#include "Intact_inProjector.h"
//
//
////IiP_FRAME frameToCapture;
////IiP_FRAME frameToProcess;
//int frameSize;
//char frameA[MAX_FRAME_SIZE];
//char frameB[MAX_FRAME_SIZE];
//char frameOut[MAX_FRAME_SIZE];
//IplImage image;
//char testA[128];
//char testB[128];
//char* test;
//
//unsigned short width, height, widthOffset, heightOffset, skippedLines, skippedFrames, sampleChannel, sampleDivider;
//
//volatile struct _semaphore
//{
//	IiP_FRAME frameToCapture;
//	BOOL done;
//} semaphore;
//
////HANDLE  hRunMutex;                   // "Keep Running" mutex 
////HANDLE  hBufferAMutex;                // "Buffer A Mutex" mutex
////HANDLE  hBufferBMutex;                // "Buffer B Mutex" mutex
////HANDLE  hCaptureMutex;                // "Buffer A Mutex" mutex
////HANDLE  hProcessMutex;                // "Buffer B Mutex" mutex
//
//int init_image(void);
//void dataAcqusition(void* arg);
// 
//int main(void)
//{
//	IiP_configfile iipConfig;
//	IiP_HANDLE iipHandle=0;
//	int key, frameCount = 0;
//	IplImage imageOut;
//
//	width         = 400;
//	height        = 220;
//	widthOffset   = 10;
//	heightOffset  = 5;
//	skippedLines  = 1;
//	skippedFrames = 1;
//	sampleChannel = 1;
//	sampleDivider = 4;
//
//	// init image structure
//	init_image();
//	imageOut = image;
//	imageOut.imageData = frameOut;
//	imageOut.imageDataOrigin = frameOut;
//	
//	// Start Intact in Projector
//	iipHandle = IiP_init();
//
//	iipConfig.width           = width;
//	iipConfig.height          = height;
//	iipConfig.widthOffset     = widthOffset;
//	iipConfig.heightOffset    = heightOffset;
//	iipConfig.skippedLines    = skippedLines;
//	iipConfig.skippedFrames   = skippedFrames;
//	iipConfig.sampleChannel   = sampleChannel;
//	iipConfig.sampleDivider   = sampleDivider;
//	iipConfig.sampleORcolor   = 0;
//	iipConfig.sampleORcolorEn = 0;
//
//	IiP_config(iipHandle, iipConfig);
//
//	IiP_register(iipHandle, INTREG_WRITEEN, 1);
//	
//	frameSize = (width * height) + HEADER_SIZE;
//
//	semaphore.frameToCapture = A;
//	semaphore.done = FALSE;
//
//	IiP_register(iipHandle, INTREG_CAPTEN, 1);
//	
//	_beginthread(dataAcqusition, 0, iipHandle);
//	//image.imageData = IiP_capture(iipHandle, frameA, frameSize);
//	//image.imageDataOrigin = image.imageData;
//	
//	while (TRUE)
//	{
//		while(semaphore.done != TRUE);
//		if (semaphore.frameToCapture == A)
//			semaphore.frameToCapture = B;
//		else
//			semaphore.frameToCapture = A;
//		semaphore.done = FALSE;
//		
//		cvSmooth(&image, &imageOut, CV_BLUR, 10);
//		
//		cvNamedWindow("Sample Program", CV_WINDOW_AUTOSIZE);
//		cvShowImage("Sample Program", &imageOut);
//				
//		key = cvWaitKey(10);
//		if( (char)key == 27 )
//			break;
//
//		frameCount++;
//
//	}
//	
//	IiP_register(iipHandle, INTREG_WRITEEN, 0);
//	
//
//	//ReleaseMutex(hRunMutex);
//	//WaitForSingleObject(hBufferAMutex, INFINITE);
//	//WaitForSingleObject(hBufferBMutex, INFINITE);
//
//	//CloseHandle(hRunMutex);
//	//CloseHandle(hBufferAMutex);
//	//CloseHandle(hBufferBMutex);
//
//	//IiP_shot(iipHandle);
//	
//	/*while (frameCount < 10)
//	{
//		//Read the video stream
//		//capture = cvCaptureFromCAM(0);
//		frame = cvQueryFrame( capture );
//		 
//		// create a window to display detected faces
//		cvNamedWindow("Sample Program", CV_WINDOW_AUTOSIZE);
//		 
//		// display face detections
//		cvShowImage("Sample Program", frame);
//		 
//		// clean up and release resources
//		cvReleaseImage(&frame);
//		 
//		
//		
//
//		int c = cvWaitKey(10);
//		if( (char)c == 27 )
//			exit(0);		
//	}*/
//
//	IiP_close(iipHandle);
//
//	return 0;
//	
//}
//
//void dataAcqusition(IiP_HANDLE iipHandle)
//{
//	int frameCount = 1;
//
//	while (TRUE)
//	{
//		while(semaphore.done == TRUE); //wait
//
//		if (semaphore.frameToCapture == A)
//		{
//			image.imageData = IiP_capture(iipHandle, frameA, frameSize);
//			image.imageDataOrigin = image.imageData;
//			
//			sprintf(testA, "testA %d", frameCount);
//			//printf("capture: testA %d", frameCount);
//		}
//		else
//		{
//			image.imageData = IiP_capture(iipHandle, frameB, frameSize);
//			image.imageDataOrigin = image.imageData;
//			
//			sprintf(testB, "testB %d", frameCount);
//			//printf("capture: testB %d", frameCount);
//		}
//		
//		semaphore.done = TRUE;
//		//printf(" done\n");
//
//		frameCount++;
//	}
//}
//
//int init_image(void)
//{
//	image.nSize = sizeof(image);      /* sizeof(IplImage) */
//	image.ID = 0;                     /* version (=0)*/
//    image.nChannels = 1;              /* Most of OpenCV functions support 1,2,3 or 4 channels */
//    //int alphaChannel;               /* Ignored by OpenCV */
//    image.depth = IPL_DEPTH_8U;       /* Pixel depth in bits: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16S,
//                                         IPL_DEPTH_32S, IPL_DEPTH_32F and IPL_DEPTH_64F are supported.  */
//    //char colorModel[4];             /* Ignored by OpenCV */
//    //char channelSeq[4];             /* ditto */
//    image.dataOrder = 0;              /* 0 - interleaved color channels, 1 - separate color channels.
//                                         cvCreateImage can only create interleaved images */
//    image.origin = 0;                   /* 0 - top-left origin,
//                                         1 - bottom-left origin (Windows bitmaps style).  */
//    //int  align;                     /* Alignment of image rows (4 or 8).
//                                      /* OpenCV ignores it and uses widthStep instead.    */
//	image.width = width;              /* Image width in pixels.                           */
//	image.height = height;            /* Image height in pixels.                          */
//    image.roi = NULL;                 /* Image ROI. If NULL, the whole image is selected. */
//	image.maskROI = NULL;             /* Must be NULL. */
//	image.imageId = NULL;             /* "           " */
//    image.tileInfo = NULL;            /* "           " */
//    image.imageSize = width*height;   /* Image data size in bytes
//                                        (==image->height*image->widthStep
//                                        in case of interleaved data)*/
//    //char *imageData;                /* Pointer to aligned image data.         */
//    image.widthStep = width;          /* Size of aligned image row in bytes.    */
//    //int  BorderMode[4];             /* Ignored by OpenCV.                     */
//    //int  BorderConst[4];            /* Ditto.                                 */
//    //char *imageDataOrigin;          /* Pointer to very origin of image data
//                                      /* (not necessarily aligned) -
//                                         needed for correct deallocation */
//	return 0;
//}
//

#endif

