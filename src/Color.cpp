/// Emil Hedemalm
/// 2014-08-31
/// An extension of Vector4f, pretty much, and will serve as a utility class for handling textures and colors in UI amongst other things.

#include "Color.h"

// o.o
Color::Color()
: Vector4f()
{}


/// Filled with 4 unsigned bytes.
Color::Color(uchar r, uchar g, uchar b, uchar a)
: Vector4f()
{
	x = r;
	y = g;
	z = b;
	w = a;
}

/// E.g. "0x115588AA"
Color Color::ColorByHexName(String byHexName)
{
	int hex = byHexName.ParseHex();

	Color newColor;

	/// Count amount of characters.
	int components = byHexName.Length() / 2 - 1;
	if (components == 4)
		newColor = ColorByHex32(hex);
	else if (components == 3)
		newColor = ColorByHex24(hex);
	else if (components == 2)
		newColor = ColorByHex16(hex);
	else
		newColor = ColorByHex8(hex);

	newColor.name = byHexName;
	return newColor;
}

/// Anticipates a hex-color in 0xRRGGBBAA format.
Color Color::ColorByHex32(int hex)
{
	unsigned char r, g, b, a;
	r = hex >> 24 % 256;
	g = hex >> 16 % 256;
	b = hex >> 8 % 256;
	a = hex >> 0 % 256;

	Color newColor(r,g,b,a);
	float inv255 = 1 / 255.f;
	newColor *= inv255;	
	return newColor;
}
/// Anticipates a hex-color in 0xRRGGBB format.
Color Color::ColorByHex24(int hex)
{
	unsigned char r, g, b;
	r = hex >> 16 % 256;
	g = hex >> 8 % 256;
	b = hex % 256;
	Color newColor(r, g, b, 255);
	float inv255 = 1 / 255.f;
	newColor *= inv255;	
	return newColor;
}

/// Anticipates a hex-color in 0xLLAA format, where L is luminosity or grey-scale.
Color Color::ColorByHex16(int hex)
{
	unsigned char l, a;
	l = hex >> 8 % 256;
	a = hex >> 256;
	Color newColor(l, l, l, a);
	float inv255 = 1 / 255.f;
	newColor *= inv255;	
	return newColor;
}
/// Anticipates a hex-color in 0xLL format, where L is luminosity or grey-scale.
Color Color::ColorByHex8(int hex)
{
	unsigned char l;
	l = hex >> 256;
	Color newColor(l, l, l, 255);
	float inv255 = 1 / 255.f;
	newColor *= inv255;	
	return newColor;
}





