/// Emil Hedemalm
/// 2014-12-05
/// Creates/maintains (parts of) a Sphere

#include "Sphere.h"


Sphere::Sphere()
{
#define DEFAULT_SECTIONS 8
	sections = DEFAULT_SECTIONS;
	segments = Vector2i(DEFAULT_SECTIONS,DEFAULT_SECTIONS);
	size = Vector2i(TwoPI, PI);
	radius = 1.0f; 

	offsetX = 0;
	invertTexUCoords = false;
}

/// Sphere Initializer
Sphere::Sphere(float radius, Vector3f position /* = Vector3f()*/ )
: radius(radius), position(position), sections(DEFAULT_SECTIONS)
{
	offsetX = 0;
	invertTexUCoords = false;
}

Sphere::~Sphere()
{
}


/** Creates a "quadratic" sphere-segment, 
	using given width and height (both in radians) from the middle of X and Y respectively.
	Assigns UV-coordinates automatically in a quadratic projection sense.
*/
Sphere * Sphere::CreateSegmentFromEquator(Vector2f size, Vector2i segments, float offsetX, bool invertTexUCoords)
{

	Sphere * sphere = new Sphere();
	sphere->size = size;
	sphere->segments = segments;
	sphere->offsetX = offsetX;
	sphere->invertTexUCoords = true;
	sphere->Generate();
	// Give it an automized name based on the sections
	sphere->SetName("Sphere");
	sphere->SetSource("Generated.");

	return sphere;
}

void Sphere::Generate()
{
	// Check that the mesh isn't already allocated and being used.
	if (numVertices)
		throw 1;

	//if (width > TwoPI)
	//	width = TwoPI;
	//if (height > PI)
	//	height = PI;

	int vertexCount = (segments.x + 1) * (segments.y + 1);

	// Default to 0
	numUVs = numNormals = numVertices = 0;

	// Allocate arrays
	vertices.Allocate(vertexCount);
	uvs.Allocate(vertexCount);
	normals.Allocate(vertexCount);

	Vector2f dSegment = size / segments;

	// Will only depend on where you want it centered?
	// Since from center,...
	float offsetY = PI * 0.5 - size.y * 0.5; 

	// For each row
	for (int y = 0; y < segments.y + 1; ++y){
		// For each vertices in the row
		for (int x = 0; x < segments.x + 1; ++x){


			int cIndex = y * (segments.x + 1) + x;
			//						Regular sinus for x				multiply with sine of row to get the relative size.
			vertices[cIndex].x = 1 * sin((x) * dSegment.x + offsetX) * sin((y) * dSegment.y + offsetY);
			vertices[cIndex].y = 1 * cos((y) * dSegment.y + offsetY);
			vertices[cIndex].z = 1 * cos((x) * dSegment.x + offsetX) * sin((y) * dSegment.y + offsetY);

			uvs[cIndex].x = (x / (float) segments.x);
			uvs[cIndex].y = (1 - y / (float) segments.y);

			if (invertTexUCoords)
				uvs[cIndex].x = 1.f - uvs[cIndex].x;

			normals[cIndex] = Vector3f(vertices[cIndex].x, vertices[cIndex].y, vertices[cIndex].z).NormalizedCopy();

			++numVertices;
			++numUVs;
			++numNormals;
		}
	}


	// Triangulate straight away.
	faces.Allocate(segments.x * segments.y * 2);
	// Default to 0.
	numFaces = 0;
	// Index to start looking at for each row.
	int index = 0;
	// Create numFaces
	for (int i = 0; i < segments.y; ++i)
	{
		for (int j = 0; j < segments.x; ++j){
			// 3 numVertices
			MeshFace * face = &faces[numFaces];
			face->numVertices = 3;
			// Allocate
			face->AllocateArrays();
			int v;
#define SET_VERTEX(vi,val) {v = vi; face->vertices[v] = face->normals[v] = face->uvs[v] = (val);}
			SET_VERTEX(0, index + j + 1 + segments.x);
			SET_VERTEX(1, index + j + 1 + segments.x + 1);
			SET_VERTEX(2, index + j + 1);
			/*v = 0; face->vertices[v] = face->normals[v] = face->uvs[v] = index + j + 1 + segments.x;
			v = 1; face->vertices[v] = faces->normals[v] = face->uvs[v] = index + j + 1 + segments.x + 1;
			v = 2; face->vertices[v] = faces->normals[v] = face->uvs[v] = index + j + 1;
			*/
			++numFaces;

			// 3 numVertices
			face = &faces[numFaces];
			face->numVertices = 3;
			// Allocate
			face->AllocateArrays();
			SET_VERTEX(0, index + j + 1);
			SET_VERTEX(1, index + j);
			SET_VERTEX(2, index + j + 1 + segments.x);
			//v = 0; faces->vertices[v] = faces->normals[v] = faces->uvs[v] = index + j + 1;
			//v = 1; faces->vertices[v] = faces->normals[v] = faces->uvs[v] = index + j;
			//v = 2; faces->vertices[v] = faces->normals[v] = faces->uvs[v] = index + j + 1 + segments.x;
			++numFaces;
		}
		// Increment sections + 1 since we're using extra numVertices for simplicities sake.
		index += segments.x + 1;
	}	
}

