/// Emil Hedemalm
/// 2014-08-31
/// An extension of Vector4f, pretty much, and will serve as a utility class for handling textures and colors in UI amongst other things.

#ifndef COLOR_H
#define COLOR_H

#include "MathLib/Vector4f.h"
#include "System/DataTypes.h"

#include "String/AEString.h"

class Color : public Vector4f 
{
public:
	// o.o
	Color();
	/// Filled with 4 unsigned bytes.
	Color(uchar r, uchar g, uchar b, uchar a);
	/// E.g. "0x115588AA"
	static Color ColorByHexName(String byHexName);
	/// Anticipates a hex-color in 0xRRGGBBAA format.
	static Color ColorByHex32(uint32 hex);
	/// Anticipates a hex-color in 0xRRGGBB format.
	static Color ColorByHex24(uint32 hex);
	/// Anticipates a hex-color in 0xLLAA format, where L is luminosity or grey-scale.
	static Color ColorByHex16(uint32 hex);
	/// Anticipates a hex-color in 0xLL format, where L is luminosity or grey-scale.
	static Color ColorByHex8(uint32 hex);
	String name;

};

#endif
