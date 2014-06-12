/// Emil Hedemalm
/// 2014-03-27
/** Stream-buffering class for extracting image data from an FTDI chip coupled to a projector/camera device.
	Original C-based code by Felix Schmidt.
	FTDI: http://www.ftdichip.com/
	API: http://www.dlpdesign.com/drivers/D2XXPG21.pdf
*/

#ifdef WINDOWS

#include "FTDIProjectorStream.h"

/*unsigned char frameA[MAX_FRAME_SIZE];
unsigned char frameB[MAX_FRAME_SIZE];
IiP_FRAME frameToCapture;
IiP_FRAME frameToProcess;
*/
IiP_HANDLE IiP_init(void)
{
	IiP_HANDLE iipHandle_ini;
	FT_STATUS ftStatus;
	UCHAR ftMask = 0xff;
	UCHAR ftMode = 0x40;
	
	// Open FTDI-Device
	ftStatus = FT_Open(0, &iipHandle_ini);//FT_OpenEx ("TTL232RG-VREG1V8", FT_OPEN_BY_DESCRIPTION , &iipHandle);
	if (ftStatus != FT_OK)
	{
		printf("Unable to open FTDI-device! code= %d\n", ftStatus);
		//sleep(2);
		return 0;
	}
	printf("FTDI-device opened successfully!\n");
	
	ftStatus = FT_SetBitMode(iipHandle_ini, ftMask, ftMode);
	if (ftStatus != FT_OK)
		printf("ERROR: Unable to set bit mode of FTDI-device! code= %d\n", ftStatus);
	
	IiP_register(iipHandle_ini, INTREG_CAPTEN, 0);
	Sleep (500);
	ftStatus = FT_Purge(iipHandle_ini, FT_PURGE_RX | FT_PURGE_TX);  // Purge both Rx and Tx buffers

	return iipHandle_ini;
}

int IiP_register(IiP_HANDLE iipHandle, unsigned char address, unsigned short value)
{
	FT_STATUS ftStatus;
	char TxBuffer[6];
	DWORD BytesWritten, RxBytes, TxBytes, EventDWord;
	BYTE_2_WORD sendValue;
	
	sendValue.wordw = value;
	
	TxBuffer[0]= (unsigned char) 0xAA;
	TxBuffer[1]= (unsigned char) 0x55;
	TxBuffer[2]= (unsigned char) 0xAA;
	TxBuffer[3]= address;
	TxBuffer[4]=sendValue.BYTE.b1;
	TxBuffer[5]=sendValue.BYTE.b0;
	
	ftStatus = FT_GetStatus(iipHandle, &RxBytes, &TxBytes, &EventDWord);
	ftStatus = FT_Write(iipHandle, TxBuffer, sizeof(TxBuffer), &BytesWritten);
	if (ftStatus != FT_OK)
		printf("Unable to write to FTDI-device! code= %d\n", ftStatus);
	else
		printf("Wrote %d to register %d. ByteWritten: %d\n", sendValue.wordw, address, BytesWritten);
	
	return (int)BytesWritten;
}

int IiP_config(IiP_HANDLE iipHandle, IiP_configfile iipConfig)
{
	
	IiP_register(iipHandle, (unsigned char) INTREG_WIDTH,    iipConfig.width);
	IiP_register(iipHandle, INTREG_HEIGHT,   iipConfig.height);
	IiP_register(iipHandle, INTREG_WIDOFF,   iipConfig.widthOffset);
	IiP_register(iipHandle, INTREG_HEIOFF,   iipConfig.heightOffset);
	IiP_register(iipHandle, INTREG_SKIPLIN,  iipConfig.skippedLines);
	IiP_register(iipHandle, INTREG_SKIPFR,   iipConfig.skippedFrames);
	IiP_register(iipHandle, INTREG_SAMDIV,   iipConfig.sampleDivider);
	IiP_register(iipHandle, INTREG_SAMCHAN,  iipConfig.sampleChannel);
	IiP_register(iipHandle, INTREG_SAMORC,   iipConfig.sampleORcolor);
	IiP_register(iipHandle, INTREG_SAMORCEN, iipConfig.sampleORcolorEn);
	
	return 0;
}

int IiP_close(IiP_HANDLE iipHandle)
{
	FT_STATUS ftStatus;
	
	// Close Device
	ftStatus = FT_Close(iipHandle);
	if (ftStatus == FT_OK)
		printf("FTDI-device closed successfully!\n");
	else
		printf("Unable to close FTDI-device!\n");
	//sleep(1);
	
	return 0;
}

char* IiP_capture(IiP_HANDLE iipHandle, char* frame_ptr, int frameSize)
{
	FT_STATUS ftStatus;
	DWORD TxBytes, RxBytes, EventDWord, BytesReceived=0;
	char* buffer_ptr;

	buffer_ptr = frame_ptr;
	unsigned int remainder = frameSize;

	while (remainder != 0)
	{
		FT_GetStatus(iipHandle, &RxBytes, &TxBytes, &EventDWord);
		//printf("Status: RxBytes = %d, TxBytes = %d, EventDWord = %d.\n", RxBytes, TxBytes, EventDWord);
		if (RxBytes > 0)
		{
			if (RxBytes > remainder)
				ftStatus = FT_Read(iipHandle, buffer_ptr, remainder, &BytesReceived);
			else
			{
				ftStatus = FT_Read(iipHandle, buffer_ptr, RxBytes, &BytesReceived);
				//printf("xxx\n");
			}

			remainder = remainder - BytesReceived;
			buffer_ptr = &buffer_ptr[BytesReceived];
		}
	}
	
	return &frame_ptr[HEADER_SIZE];
}

int IiP_shot(IiP_HANDLE iipHandle)
{
	int col, pix, line, address, append, charCount = 0;
	BYTE_2_WORD byte2word;
	BYTE_2_DWORD byte2dword;
	BYTE_2_LONG byte2long;
	byte2word.wordw = 0;
	byte2dword.dwordw = 0;
	byte2long.longw = 0;
	char filePath[128];
	FILE *file_handle;
	
	FT_STATUS ftStatus;
	DWORD TxBytes, RxBytes=1, EventDWord, BytesReceived;
	BYTE_2_WORD header;
	int width, height;
	unsigned char RxBuffer[MAX_FRAME_SIZE];
	unsigned char *RxBuffer_ptr, *shotMem;
	
	printf("Start of Screenshot!");
	
	IiP_register(iipHandle, INTREG_SHOT, 1);
	//IiP_register(iipHandle, INTREG_SHOT, 0);
	
	Sleep(1000);
	
	RxBuffer_ptr = &RxBuffer[0];
	
	while (RxBytes != 0)
	{
		FT_GetStatus(iipHandle, &RxBytes, &TxBytes, &EventDWord);
		printf("Status: RxBytes = %d, TxBytes = %d, EventDWord = %d.\n", RxBytes, TxBytes, EventDWord);
		if (RxBytes > 0)
		{
			ftStatus = FT_Read(iipHandle, RxBuffer_ptr, RxBytes, &BytesReceived);
			if (ftStatus != FT_OK)
				printf("ERROR: Unable to read from FTDI-device!\n");
			else
				printf("Read %d Bytes from FTDI-device!\n", BytesReceived);
		}
		RxBuffer_ptr = &RxBuffer[RxBytes];
	}
	

	header.BYTE.b1 = RxBuffer[5];
	header.BYTE.b0 = RxBuffer[6];
	width = header.wordw;
	header.BYTE.b1 = RxBuffer[7];
	header.BYTE.b0 = RxBuffer[8];
	height = header.wordw;
	
	shotMem = &RxBuffer[22];
	


	sprintf(filePath,"screenshot.bmp");
	
	//printf("%s\n", filePath);
	file_handle = fopen (filePath, "wb");
	
	if (file_handle != NULL)
	{
		// HEADER
		fprintf(file_handle, "BM");
		// file size, offset = 2
		byte2dword.dwordw = 54 + 3*(width*height);
		fprintf(file_handle, "%c%c%c%c", byte2dword.BYTE.b0, byte2dword.BYTE.b1, byte2dword.BYTE.b2, byte2dword.BYTE.b3);
		// reserved, offset = 6 
		byte2dword.dwordw = 0;
		fprintf(file_handle, "%c%c%c%c", byte2dword.BYTE.b0, byte2dword.BYTE.b1, byte2dword.BYTE.b2, byte2dword.BYTE.b3);
		// offset of picture data, offset = 10 
		byte2dword.dwordw = 54;
		fprintf(file_handle, "%c%c%c%c", byte2dword.BYTE.b0, byte2dword.BYTE.b1, byte2dword.BYTE.b2, byte2dword.BYTE.b3);
		
		// DATA INFORMATION BLOCK
		// size of info block, offset = 14
		byte2dword.dwordw = 40;
		fprintf(file_handle, "%c%c%c%c", byte2dword.BYTE.b0, byte2dword.BYTE.b1, byte2dword.BYTE.b2, byte2dword.BYTE.b3);
		// bitmap width, offset = 18
		byte2long.longw = width;
		fprintf(file_handle, "%c%c%c%c", byte2long.BYTE.b0, byte2long.BYTE.b1, byte2long.BYTE.b2, byte2long.BYTE.b3);
		// bitmap width, offset = 22
		byte2long.longw = height; // positive for bottom-up
		fprintf(file_handle, "%c%c%c%c", byte2long.BYTE.b0, byte2long.BYTE.b1, byte2long.BYTE.b2, byte2long.BYTE.b3);
		// planes, offset = 26
		byte2word.wordw = 1;
		fprintf(file_handle, "%c%c", byte2word.BYTE.b0, byte2word.BYTE.b1);
		// color depth, offset = 28
		byte2word.wordw = 24;
		fprintf(file_handle, "%c%c", byte2word.BYTE.b0, byte2word.BYTE.b1);
		// compression, offset = 30
		byte2dword.dwordw = 0;
		fprintf(file_handle, "%c%c%c%c", byte2dword.BYTE.b0, byte2dword.BYTE.b1, byte2dword.BYTE.b2, byte2dword.BYTE.b3);
		// image size, offset = 34
		byte2dword.dwordw = 0;
		fprintf(file_handle, "%c%c%c%c", byte2dword.BYTE.b0, byte2dword.BYTE.b1, byte2dword.BYTE.b2, byte2dword.BYTE.b3);
		// hor pixel per meter, offset = 38
		byte2long.longw = 0;
		fprintf(file_handle, "%c%c%c%c", byte2long.BYTE.b0, byte2long.BYTE.b1, byte2long.BYTE.b2, byte2long.BYTE.b3);
		// ver pixel per meter, offset = 42
		byte2long.longw = 0;
		fprintf(file_handle, "%c%c%c%c", byte2long.BYTE.b0, byte2long.BYTE.b1, byte2long.BYTE.b2, byte2long.BYTE.b3);
		// color table, offset = 46
		byte2dword.dwordw = 0;
		fprintf(file_handle, "%c%c%c%c", byte2dword.BYTE.b0, byte2dword.BYTE.b1, byte2dword.BYTE.b2, byte2dword.BYTE.b3);
		// number of colors, offset = 50
		byte2dword.dwordw = 0;
		fprintf(file_handle, "%c%c%c%c", byte2dword.BYTE.b0, byte2dword.BYTE.b1, byte2dword.BYTE.b2, byte2dword.BYTE.b3);
		charCount=54;
		
		// IMAGE DATA
		append = (width*3) % 4;
		for (line=height-1; line>=0; line--)
		{
			for (pix=0; pix<width; pix++)
			{
				address = line*width + pix;
				for (col=0; col<3; col++)
				{
					fprintf(file_handle, "%c", shotMem[address]);
					charCount++;
				}
			}
			//printf("%i\n", pix);
			for (pix=0; pix<append; pix++)
			{
				fprintf(file_handle, "%c", 0);
				charCount++;
			}
		}
		
		fclose (file_handle);
		IiP_register(iipHandle, INTREG_SHOT, 0);
		
		return 1;
		
	}
	
	return 0;
}

#endif
