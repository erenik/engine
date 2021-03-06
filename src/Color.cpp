/// Emil Hedemalm
/// 2014-08-31
/// An extension of Vector4f, pretty much, and will serve as a utility class for handling textures and colors in UI amongst other things.

#include "Color.h"

Color Color::defaultTextColor = Color(Vector4f(1, 1, 1, 1));

// o.o
Color::Color()
: Vector4f()
{
}

/// Full alpha
Color::Color(ConstVec3fr fromVector)
	: Vector4f(fromVector, 1)
{
}

Color::Color(const Vector4f & fromVector)
: Vector4f(fromVector)
{
}


/// Filled with 4 unsigned bytes.
Color::Color(uchar r, uchar g, uchar b, uchar a)
: Vector4f(r,g,b,a)
{
	*this /= 255.f;
}

/// E.g. "0x115588AA"
Color Color::ColorByHexName(String byHexName)
{
	uint32 hex = (uint32) byHexName.ParseHex();
	int hexChars = byHexName.HexNumbers();

	Color newColor;

	/// Count amount of characters.
	int components = hexChars;
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

Color Color::WithAlpha(float alpha) const {
	Color newShared = Color(*this);
	newShared.w = alpha;
	return newShared;
}

/// E.g. RGBA r g b a where r g b a are numbers between 0 and 255, or 0 and 1.0
bool Color::ParseFrom(String str)
{
	// Set default values.
	*this = Vector4f(0, 0, 0, 1.0f);
	String string = str;
	List<float*> order;
	for (int i = 0; i < string.Length(); ++i)
	{
		char c = string.c_str()[i];
		if (c == 'R' || c == 'r')
			order.Add(&this->x);
		else if (c == 'G' || c == 'g')
			order.Add(&this->y);
		else if (c == 'B' || c == 'b')
			order.Add(&this->z);
		else if (c == 'A' || c == 'a')
			order.Add(&this->w);
	}
	List<String> parts = string.Tokenize(" ");
	int numbersParsed = 0;
	for (int i = 0; i < parts.Size(); ++i)
	{
		String part = parts[i];
		if (!part.IsNumber())
			continue;
		float number = part.ParseFloat();
		float * ptr = order[numbersParsed];
		*ptr = number;
		++numbersParsed;
		if (numbersParsed >= order.Size())
			break;
	}	
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
	name = "R"+String((*this)[0])+"G"+String((*this)[1])+"B"+String((*this)[2])+"Z"+String((*this)[3]);
}
