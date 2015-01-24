/// Emil Hedemalm
/// 2014-08-31
/// An extension of Vector4f, pretty much, and will serve as a utility class for handling textures and colors in UI amongst other things.

#include "Color.h"

// o.o
Color::Color()
: Vector4f()
{
}


Color::Color(const Vector4f & fromVector)
: Vector4f(fromVector)
{
}


/// Filled with 4 unsigned bytes.
Color::Color(uchar r, uchar g, uchar b, uchar a)
: Vector4f()
{
	float inv255 = 1 / 255.f;
	x = r;
	y = g;
	z = b;
	w = a;
	(*this) *= inv255;	
}

/// E.g. "0x115588AA"
Color Color::ColorByHexName(String byHexName)
{
	uint32 hex = byHexName.ParseHex();

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
Color Color::ColorByHex32(uint32 hex)
{
	uint32 r, g, b, a;
	r = (hex >> 24) % 256;
	g = (hex >> 16) % 256;
	b = (hex >> 8) % 256;
	a = (hex >> 0) % 256;

	Color newColor(r,g,b,a);
	return newColor;
}
/// Anticipates a hex-color in 0xRRGGBB format.
Color Color::ColorByHex24(uint32 hex)
{
	unsigned char r, g, b;
	r = hex >> 16 % 256;
	g = hex >> 8 % 256;
	b = hex % 256;
	Color newColor(r, g, b, 255);
	return newColor;
}

/// Anticipates a hex-color in 0xLLAA format, where L is luminosity or grey-scale.
Color Color::ColorByHex16(uint32 hex)
{
	uint32 l, a;
	l = (hex >> 8) % 256;
	a = hex % 256;
	Color newColor(l, l, l, a);
	return newColor;
}
/// Anticipates a hex-color in 0xLL format, where L is luminosity or grey-scale.
Color Color::ColorByHex8(uint32 hex)
{
	unsigned char l;
	l = hex % 256;
	Color newColor(l, l, l, 255);
	return newColor;
}

bool Color::WriteTo(std::fstream & file)
{
	name.WriteTo(file);
	Vector4f::WriteTo(file);
	return true;
}
bool Color::ReadFrom(std::fstream & file)
{
	name.ReadFrom(file);
	Vector4f::ReadFrom(file);
	return true;
}

String Color::GetName()
{
	if (name.Length() == 0)
		AssignName();
	return name;
}


void Color::AssignName()
{
	name = "R"+String(x)+"G"+String(y)+"B"+String(z)+"Z"+String(w);
}
