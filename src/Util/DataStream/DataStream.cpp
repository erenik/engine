/// Emil Hedemalm
/// 2014-10-08
/// Utility class for handling arbitrary data streams of any kind (kind of like a queue, but for data instead of pointer-based C++ objects as used in List and Queue classes)

#include "DataStream.h"

#include <cassert>
#include <cstring>
#include <iostream>

#include "String/AEString.h"

/// A stream for data, which re-allocates its internal array as necessary.
DataStream::DataStream()
{
	data = NULL;
	dataLength = 0;
	bytesUsed = 0;
	maxBytes = 1024 * 1024;
	automaticPushback = true;
	readOffset = 0;
}

DataStream::~DataStream()
{
	if (data)
		delete[] data;
	data = NULL;
}

/** If true, will automatically push back data, preparing for the loading of new data. 
	If false, will use an extra integer to keep track of current read location,
	reducing amount of movement of data. (e.g. when reading entire file into stream
	and then just analyzing the contents).
*/
void DataStream::SetPopAutomaticPushback(bool set)
{
	automaticPushback = set;
}

void DataStream::Allocate(int newMax)
{
	if (data)
		delete[] data;
	readPointer = data = new uchar[newMax];
	dataLength = newMax;
	bytesUsed = 0;
}

/// If manipulating contents from outside, set this to number of read/manipulated bytes?
void DataStream::SetBytesUsed(int newBytesUsed)
{
	bytesUsed = newBytesUsed;
}


/// Pushes the text into the stream, including ending NULL-sign.
void DataStream::Push(String text)
{
	PushBytes((uchar*)text.c_str(), text.Length() + 1);
}

/// Pushes raw text, without including NULL or ending '\n' signs.
void DataStream::PushRawText(String text)
{
	PushBytes((uchar*)text.c_str(), text.Length());
}

/// Pushes the text into the stream, replacing ending NULL-sign with '\n'.
void DataStream::PushLine(String text)
{
	PushBytes((uchar*)text.c_str(), text.Length());
	char end = '\n';
	PushBytes((uchar*) &end, 1);
}

/// Pushes an integer, or 4 bytes.
void DataStream::PushInt(int integer)
{
	PushBytes((uchar*)&integer, 4);
}

/// Pushes 2 bytes, 16 bits of data.
void DataStream::PushInt16(short shorterInteger)
{
	PushBytes((uchar*)&shorterInteger, 2);
}

/// Pushes 1 bytes, 8 bits of data.
void DataStream::PushInt8(char c)
{
	PushBytes((uchar*)&c, 1);	
}

/// o-o
void DataStream::PadBytes(char c, int num)
{
	for (int i = 0; i < num; ++i)
	{
		PushBytes((uchar*)&c, 1);
	}
}

/// Pushes bytes to the end of the array.
void DataStream::PushBytes(uchar * fromArray, int numberOfBytes)
{
//	std::cout<<"\nPushing bytes: "<<numberOfBytes;
	// Allocate as needed.
	if (!data)
	{
		data = new uchar[numberOfBytes];
		dataLength = numberOfBytes;
	}
	if (FreeBytes() < numberOfBytes)
	{
		/// If data length has been reached, just ignore this push? Or maybe discard older data?
		if (dataLength > maxBytes)
		{
			assert(numberOfBytes < dataLength);
			// Push back old data
			memmove(data, data + numberOfBytes, bytesUsed - numberOfBytes);
			// Add new data at the end.
			memcpy(data + bytesUsed - numberOfBytes, fromArray, numberOfBytes);
			return;
		}
		int newLength = bytesUsed + numberOfBytes;
		uchar * newData = new uchar[newLength];
		memcpy(newData, data, bytesUsed);
		dataLength = newLength;
		// Delete old array
		delete[] data;
		data = newData;
	}
	// Add the new data
	memcpy(data + bytesUsed, fromArray, numberOfBytes);
	bytesUsed += numberOfBytes;
}

// Pops all bytes up to the specified point in the array.
void DataStream::PopToPointer(uchar * point)
{
	int bytes = point - data;
	PopBytes(bytes);
}

/// Removes the X first number of bytes from the stream.
void DataStream::PopBytes(int numberOfBytes)
{
	assert(numberOfBytes <= bytesUsed);
	bytesUsed -= numberOfBytes;
	/// Move back the bytes in the stream.
	if (automaticPushback)
		memmove(data, data + numberOfBytes, bytesUsed);
	else { 
		readPointer += numberOfBytes;
		readOffset += numberOfBytes;
	}
}

/// Attempts to pop designated bytes from this stream, placing them into target buffer instead. Returns amount of bytes popped this way.
int DataStream::PopBytesToBuffer(uchar * buffer, int maxBytesToPop)
{
	int bytesMoved = maxBytesToPop;
	if (bytesMoved > bytesUsed)
		bytesMoved = bytesUsed;
	memmove(buffer, readPointer, bytesMoved);
	PopBytes(bytesMoved);
	return bytesMoved;
}

/// Sets used bytes to 0.
void DataStream::PopAll()
{
	bytesUsed = 0;
}	

/// Returns the read pointer for the data, which updates when popping data or when it is re-allocated (safer pointer than GetData())
uchar ** DataStream::GetReadPointer()
{
	return &readPointer;
}

/// Returns pointer to the data. May not be NULL-terminated.
uchar * DataStream::GetData()
{	
	return data; 
}

/// Returns the current data as a string. Do note that this will fail if there are binary 0s within the stream.
bool DataStream::GetDataAsString(String & string)
{
	string.Allocate(bytesUsed + 1, 0);
	memset(string.c_str_editable(), 0, bytesUsed + 1);
	memcpy(string.c_str_editable(), data, bytesUsed);  
	return true;
}

/// Returns the first line within the stream. Will return false if no '\n' or '\0' sequence was found.
bool FirstLine(String & string)
{
	assert(false);
	return false;
}


/// Number of bytes currently in the stream.
int DataStream::Bytes()
{
	return bytesUsed;
}

/// Number of free bytes within, before a re-allocation is necessary.
int DataStream::FreeBytes()
{
	return dataLength - bytesUsed;
}

/// Default 1 MB. Override here.
void DataStream::SetMaxBytes(int newMax)
{
	maxBytes = newMax;
}

