/// Emil Hedemalm
/// 2015-05-29
/// o.o Loads pngggggg irrespective of library.

#include "Image.h"
#ifdef LIBPNG
	#include <png.h>
#endif
#include <zlib.h>
#include "Texture.h"

#include "File/File.h"
#include "DataStream/DataStream.h"

#define MEGABYTE 1048576 // 1024 * 1024

/// Loads into texture.
bool LoadPNG(String fromFile, Texture * intoTexture)
{
	DataStream stream, outputStream;
	/// Should move allocation to after image size has been discovered... TODO
	stream.Allocate(MEGABYTE);
	outputStream.Allocate(MEGABYTE);
	stream.SetMaxBytes(MEGABYTE * 20); // 20 MB in max.
	outputStream.SetMaxBytes(MEGABYTE * 100);
	File file;
	file.SetPath(fromFile);
	file.Open();
	bool readOK = file.ReadBytes(stream, 8);
	if (!readOK)
		return false;
	/// Check first 8 bytes.
	// http://en.wikipedia.org/wiki/Portable_Network_Graphics#Technical_details
	uchar * data = stream.GetData();
	if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47)
		// good
		;
	else {
		return false;
	}
	enum // http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html#C.IHDR
	{
		PNG_GRAYSCALE, // 1,2,4,8,16 bit depths allowed
		PNG_RGB = 2, // RGB, 8 or 16 bit depths allowed
		PNG_PALETTE_INDEX = 3, // requires PLTE chunk
		PNG_GRAYSCALE_ALPHA = 4, // 8 or 16 bit depths allowed
		PNG_RGBA = 6, // RGB + alpha sample, 8 or 16 bit depths allowed
	};
	enum 
	{
		PNG_NO_INTERLACING,
		PNG_ADAM7_INTERLACING,
	};
	int width, height, bitDepth, colorType, compressionMethod, filterMethod, interlaceMethod;

	/// Initialize inflater
    z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
#define CHUNK 16384
	int ret, flush;
    unsigned have;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return false;
	
	// Read chunks.
	while(true)
	{
		file.ReadBytes(stream, 8);
#define GetInt(index) ((data[index+0] << 24) + (data[index+1] << 16) + (data[index+2] << 8) + data[index+3])
		int chunkLength = GetInt(0);
		String chunkType((char*)data+4, (char*)data+8);
		std::cout<<"\nChunk type: "<<chunkType;
		/// Read the actual data.
//		assert(chunkLength < stream.Bytes());
		file.ReadBytes(stream, chunkLength);
		if (chunkType == "IHDR")
		{
			// Image height, width, depth, etc., http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html#C.IHDR
			width = GetInt(0);
			height = GetInt(4);
			bitDepth = data[8];
			colorType = data[9];
			compressionMethod = data[10]; // Default deflate/inflate with 'sliding window' at most 32768 bytes.
			assert(compressionMethod == 0);
			filterMethod = data[11];
			assert(filterMethod == 0);
			interlaceMethod = data[12];

		}
		else if (chunkType == "pHYs")
		{
			// 'Intended pixel size' and aspect ratio..
		}
		else if (chunkType == "iCCP")
		{
			// ICC color profile, http://en.wikipedia.org/wiki/ICC_profile

		}
		else if (chunkType == "PLTE")
		{
			// Palette of colors.
			std::cout<<"\nplaataaaaaaa";

		}
		else if (chunkType == "IDAT")
		{
			// Image data
			std::cout<<"\nDataaaaaaa";
			// Inflate it,
			// decompress until deflate stream ends or end of file 
			do {
				int bytesPopped = stream.PopBytesToBuffer(in, CHUNK);
				strm.avail_in = bytesPopped; // fread(in, 1, CHUNK, source);
				if (strm.avail_in == 0)
					break;
				strm.next_in = in;
				// run inflate() on input until output buffer not full
				do {
					strm.avail_out = CHUNK;
					strm.next_out = out;

					ret = inflate(&strm, Z_NO_FLUSH);
					assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
					switch (ret) {
						case Z_NEED_DICT:
							ret = Z_DATA_ERROR;     /* and fall through */
						case Z_DATA_ERROR:
						case Z_MEM_ERROR:
							(void)inflateEnd(&strm);
							return ret;
					}
					have = CHUNK - strm.avail_out;
					/// Push decoded/inflated image data to output-stream.
					outputStream.PushBytes(out, have);
				} while (strm.avail_out == 0);
			 /* done when inflate() says it's done */
		    } while (ret != Z_STREAM_END);
			/* clean up and return */
			(void)inflateEnd(&strm);
			if (ret == Z_STREAM_END)
				break;
			/// Error, broken stream.?
			else {
				std::cout<<"\nBroken stream when inflating png data. D:";
				return false;
			}
		}
		else if (chunkType == "IEND")
		{
			// End.
			break;
		}
		// Read CRC ending the chunk.
		file.ReadBytes(stream, 4);
	}
	/// Alright, data should be there, sizes too, now create texture from it? perhaps copy over the pre-allocated array already? D:
	Texture * texture = intoTexture;
	texture->Resize(Vector2i(width, height));
	uchar * texData = texture->data;

	std::cout<<"\nSuccess o.o";
	uchar * idata = outputStream.GetData();
	List<int> encountered;
	int channelsPerPixel = (colorType == PNG_RGB? 3 : colorType == PNG_RGBA? 4 : 1);
	int bytesPerPixel = channelsPerPixel * bitDepth / 8;
	int pixelsLoaded = 0;
	int filter = 0;
	
	List<Vector3i> filters;

	/// Grab the scanline thingies first.
	/*
	if (i < width)
	{
		for (int j = i; j < i + bytesPerPixel; ++j)
		{
			filters.AddItem(idata[i]);
			filters.AddItem(idata[i]);
			filters.AddItem(idata[i]);			
		}
		continue;
	}*/

	int outStreamBytes = outputStream.Bytes()
	for (int i = 0; i < outStreamBytes; i += bytesPerPixel)
	{



		if (pixelsLoaded % width == 0)
		{
			// check scanline byte first.
			filter = idata[i];
//			std::cout<<"\nFilter: "<<filter;
	//		++i;
		}

		uchar r, g, b, a;
		int value = idata[i];
		r = idata[i];
		g = idata[i+1];
		b = idata[i+2];
		a = channelsPerPixel == 4? idata[i+3] : 255;


		/// pixel start index. 
		int psi = pixelsLoaded * 4;
		/// Alright, try ship it into the texture? RGBA style.
		texData[psi+0] = r;
		texData[psi+1] = g;
		texData[psi+2] = b;
		texData[psi+3] = a;

		++pixelsLoaded;

		// Some debug couts.
		/*
		if (encountered.Exists(value))
			continue;
		std::cout<<"\ndata "<<i<<": "<<(int) idata[i];
		encountered.AddItem(value);*/
	}
	/// Set texture stats?
	texture->bytesPerChannel = 1;
	return true;
}
