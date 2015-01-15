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
}

DataStream::~DataStream()
{
	if (data)
		delete[] data;
	data = NULL;
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
		int newLength = dataLength + numberOfBytes;
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
	assert(numberOfBytes < bytesUsed);
	bytesUsed -= numberOfBytes;
	/// Move back the bytes in the stream.
	memmove(data, data + numberOfBytes, bytesUsed);
}

/// Sets used bytes to 0.
void DataStream::PopAll()
{
	bytesUsed = 0;
}	

/// Returns the current data as a string. Do note that this will fail if there are binary 0s within the stream.
bool DataStream::GetDataAsString(String & string)
{
	string.Allocate(bytesUsed + 1, 0);
	memset(string.c_str_editable(), 0, bytesUsed + 1);
	memcpy(string.c_str_editable(), data, bytesUsed);  
	return true;
}

/// Number of free bytes within, before a re-allocation is necessary.
int DataStream::FreeBytes()
{
	return dataLength - bytesUsed;
}
