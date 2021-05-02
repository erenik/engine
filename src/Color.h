/// Emil Hedemalm
/// 2014-08-31
/// An extension of Vector4f, pretty much, and will serve as a utility class for handling textures and colors in UI amongst other things.

#ifndef COLOR_H
#define COLOR_H

#include "MathLib/Vector4f.h"
#include "System/DataTypes.h"

#include "String/AEString.h"

class 
#ifdef USE_SSE
	alignas(16)
#endif
	Color : public Vector4f
{
public:
	// o.o
	Color();
	/// Full alpha
	Color(ConstVec3fr fromVector);
	Color(const Vector4f & fromVector);
	/// Filled with 4 unsigned bytes.
	Color(uchar r, uchar g, uchar b, uchar a);

	//const Vector4f& Vector() const { return vec; };

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

	static Color White() { return Color::ColorByHexName("0xFFFFFFFF"); }
	static Color Gray() { return Color::ColorByHexName("0x888888FF"); }


	String GetName();

	void SetAlpha(float value) { w = value; }
	void SetRGB(float r, float g, float b) { x = r; y = g; z = b; }

	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);

	Color WithAlpha(float alpha) const;

	/// E.g. RGBA r g b a where r g b a are numbers between 0 and 255, or 0 and 1.0
	bool ParseFrom(String str);

	static Color defaultTextColor;

private:

	//Vector4f vec;
	String name;
	void AssignName();
};

#endif
