/// Emil Hedemalm
/// 2014-02-13
/// Square mesh generated and meant to be used by UI rendering.

#ifndef SQUARE_H
#define SQUARE_H

#include "Globals.h"
#include "Mesh.h"

/// A simple 2D square mesh used mainly for UI
class Square : public Mesh {
public:
	Square();
	/// Virtual destructor so that base class destructor is called correctly.
	virtual ~Square();

	/// Sets the dimensions of the square using provided arguments
	void SetDimensions(float left, float right, float bottom, float top, float z = 0);
	/// Sets UV coordinates of the square using provided arguments
	void SetUVs(float left, float right, float bottom, float top);
};

#endif
