/// Emil Hedemalm
/// 2014-10-14
/// Conversion-specific filters.

#include "CVImageFilters.h"
#include "CV/CVPipeline.h"
#include <bitset>

CVShortToUnsignedByte::CVShortToUnsignedByte()
	: CVImageFilter(CVFilterID::SHORT_TO_UNSIGNED_BYTE)
{
	mostSignificantBit = new CVFilterSetting("Most significant bit", 0);
	settings.Add(mostSignificantBit);
	about = "Specifies where the first bit should be read from the 2 bytes.\nThe rest 7 bits will be chosen automatically beased on the first one.\nBit 0 refers to the bit of largest value\n while bit 15 refers to the least significant bit.";
}

int CVShortToUnsignedByte::Process(CVPipeline * pipe)
{
	cv::Mat & input = pipe->input;
	if (input.type() != CV_16UC1)
	{
		errorString = "Image not single-channel 16 bits per channel/per pixel.";
		input.copyTo(pipe->output);
		return CVReturnType::CV_IMAGE;
	}
	cv::Mat newImage(input.rows, input.cols, CV_8UC1);
	uchar * newImageData = newImage.data;
	int channels = input.channels();
	int msb = mostSignificantBit->GetInt();
	// Takes the first 8 bits.
	int filter = 0xFF00;
	if (msb > 0)
		filter = filter >> msb;
	else if (msb < 0)
		filter = filter << (-msb);
	for( int y = 0; y < input.rows; y++ )
	{
		for( int x = 0; x < input.cols; x++ )
		{ 
			int pixelIndex = y * input.cols + x;
				
			ushort  * pixelData = (ushort*) &input.data[pixelIndex];
			ushort shortValue = *pixelData;
			std::cout<<"\nBits pre "<<std::bitset<16>(shortValue);		

			int value = *pixelData;
			int newValue = value & filter;
			int shift = (8 - msb);
			int newValueShifted = shift > 0 ? (newValue >> shift) : newValue;
			uchar charValue = newValueShifted;
			std::cout<<" Bits post "<<std::bitset<8>(charValue);		


			/// Fetch rgb values.
			uchar r,g,b;
			newImageData[pixelIndex] = charValue;
		}
	}
	pipe->output = newImage;
	return CVReturnType::CV_IMAGE;
}




