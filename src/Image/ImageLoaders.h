/// Emil Hedemalm
/// 2015-05-29
/// o.o Loads pngggggg irrespective of library.

#ifndef AE_PNG_H
#define AE_PNG_H

#include "Texture.h"

/// Loads into texture.
bool LoadPNG(String fromFile, Texture * intoTexture);
bool LoadLodePNG(String source, Texture * texture);
bool LoadOpenCV(String source, Texture * texture);

/// Saving function.
bool SavePNG(String toFile, Texture * fromTexture);
bool SaveBMP(String toFile, Texture * fromTexture);

#endif // AE_PNG_H
