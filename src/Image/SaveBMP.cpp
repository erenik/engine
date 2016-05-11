/// Emil Hedemalm
/// 2016-04-22
/// Lazy saver.

#include "ImageLoaders.h"

#include "File/File.h"
#include "DataStream/DataStream.h"
#include "Texture.h"

bool SaveBMP(String toFile, Texture * texture)
{
	toFile.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (!toFile.Contains(".bmp"))
	{
		toFile = toFile + ".bmp";
	}
	DataStream ds;
	File file(toFile);
	file.OpenForWriting();
	bool isOpen = file.IsOpen();
	if (!isOpen)
		return false;

	// Make stream fit size beforehand?
	ds.Allocate(texture->height * texture->width * 3 + 80);

	int bitsPerPixel = 24;
	// Calc it.. 14 for header
	int dataStartOffset = 14 + 12;
	// Image data.
	int bytesPerRowWithPadding = ((bitsPerPixel * texture->width + 31) / 32) * 4;
	int bytesPerRow = texture->width * 3;
	int paddingBytesPerRow = bytesPerRowWithPadding - bytesPerRow;
	int dataSize = bytesPerRowWithPadding * texture->height;
	int fileSize = dataStartOffset + dataSize;

	// Write data to stream,
	// BMP header, 14 bytes. https://en.wikipedia.org/wiki/BMP_file_format#Device-independent_bitmaps_and_the_BMP_file_format
	ds.PushRawText("BM");
	ds.PushInt(fileSize);
	ds.PushInt(fileSize); // 4 bytes reserved, filesize dummy data.
	ds.PushInt(dataStartOffset);
	// DIB header, (bitmap information header)
	ds.PushInt(12); // Smallest header.
	ds.PushInt16(texture->width);
	ds.PushInt16(texture->height);
	ds.PushInt16(1); // color-panes, must be 1
	ds.PushInt16(bitsPerPixel); // bits per pixel, default 32, no alpha support.
	assert(paddingBytesPerRow >= 0);
	for (int y = 0; y < texture->height; ++y)
	{
		for (int x = 0; x < texture->width; ++x)
		{
			Vector4i vec = texture->GetPixelVec4i(x,y); // BGR format
			ds.PushInt8(vec.z);
			ds.PushInt8(vec.y);
			ds.PushInt8(vec.x);
//			ds.PushInt8(vec.w);
		}
		ds.PadBytes(0, paddingBytesPerRow);
	}

	// Dump stream unto file.
	file.GetStream().write((char*)ds.GetData(), ds.Bytes());
	file.Close();
	return true;
}

