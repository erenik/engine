/// Emil Hedemalm
/// 2014-10-08
/// Utility class for handling arbitrary data streams of any kind (kind of like a queue, but for data instead of pointer-based C++ objects as used in List and Queue classes)

#ifndef DATA_STREAM_H
#define DATA_STREAM_H

#include "System/DataTypes.h"

class String;

/// A stream for data, which re-allocates its internal array as necessary.
class DataStream 
{
public:
	DataStream();
	virtual ~DataStream();

	/** If true, will automatically push back data, preparing for the loading of new data. 
		If false, will use an extra integer to keep track of current read location,
		reducing amount of movement of data. (e.g. when reading entire file into stream
		and then just analyzing the contents).
	*/
	void SetPopAutomaticPushback(bool set);

	/// Allocates, Deletes and creates new array. Use only upon initialization!
	void Allocate(int maxBytes);
	/// If manipulating contents from outside, set this to number of read/manipulated bytes?
	void SetBytesUsed(int bytesUsed);
	/// Pushes the text into the stream, including ending NULL-sign.
	void Push(String text);
	/// Pushes raw text, without including NULL or ending '\n' signs.
	void PushRawText(String text);
	/// Pushes the text into the stream, replacing ending NULL-sign with '\n'.
	void PushLine(String text);
	/// Pushes an integer, or 4 bytes.
	void PushInt(int integer);
	/// Pushes 2 bytes, 16 bits of data.
	void PushInt16(short shorterInteger);
	/// Pushes 1 bytes, 8 bits of data.
	void PushInt8(char c);
	/// Pushes bytes to the end of the array.
	void PushBytes(uchar * fromArray, int numberOfBytes);
	/// o-o
	void PadBytes(char c, int num);
	// Pops all bytes up to the specified point in the array.
	void PopToPointer(uchar * point);
	/// Removes the X first number of bytes from the stream.
	void PopBytes(int numberOfBytes);
	/// Attempts to pop designated bytes from this stream, placing them into target buffer instead. Returns amount of bytes popped this way.
	int PopBytesToBuffer(uchar * buffer, int maxBytesToPop);
	/// Sets used bytes to 0.
	void PopAll();
	/// Returns the read pointer for the data, which updates when popping data or when it is re-allocated (safer pointer than GetData())
	uchar ** GetReadPointer();
	/// Returns pointer to the data. The contents may or may not be NULL-terminated. Pointer may become invalid.
	uchar * GetData();
	/// Returns the current data as a string. Do note that this will fail if there are binary 0s within the stream. Returns false if something.. happens.
	bool GetDataAsString(String & string);
	/// Returns the first line within the stream. Will return false if no '\n' or '\0' sequence was found.
	bool FirstLine(String & string);
	/// Number of bytes currently in the stream.
	int Bytes();
	/// Number of free bytes within, before a re-allocation is necessary.
	int FreeBytes();
	/// Default 1 MB. Override here.
	void SetMaxBytes(int newMax);
protected:
	/// Default true.
	bool automaticPushback;
	/// Read offset if not using pushback.
	int readOffset;
	/// A pointer to read the data. Used and updated if using the read offset.
	uchar * readPointer;
	/// Max bytes. Pushing will do nothing if exceeding this limit. 1 MB may be a good limit.
	int maxBytes;
	/// The actual data.
	uchar * data;
	/// Length of allocated data array.
	int dataLength;
	/// Length of actual relevant data.
	int bytesUsed;
};

#endif


