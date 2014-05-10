/// Emil Hedemalm
/// 2014-04-09
/// A blob-of-points class for computer vision processing.

#ifndef CV_BLOB_H
#define CV_BLOB_H

#include "CVPoint.h"
#include <List/List.h>

/// A blob-of-points class for computer vision processing.
class CVBlob 
{
public:
	List<CVPoint*> points;
	/// Some coordinates about the blob as a whole.
	Vector2i min, max;
	Vector2f center;
};

#endif