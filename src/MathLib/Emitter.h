/// Emil Hedemalm
/// 2016-04-30
/// Emitter for getting randomly - or structured randomly - selected points/vectors within some desired area.

#ifndef EMITTER_H
#define EMITTER_H

#include "MathLib.h"
#include "PhysicsLib/Shapes/Triangle.h"
#include "Random/Random.h"

namespace EmitterType {
	enum emitterTypes
	{
		DEFAULT,
		CONTOUR,
		POINT_DIRECTIONAL,
		POINT_CIRCLE,


		/// New types using the Emitter sub-system.
		SPHERE,
		TRIANGLES,
		POINT, // Using vec-vector for position.
		LINE, // Using the vec-vector.
		LINE_BOX, // Uses vec- and vec2-vectors to create a box or parralellogram distribution. If both vectors are equal in size a rhomb or quad is formed. Rectangle or parralellogram if one is longer than the other.
		LINE_Y, // Using up-vector
		PLANE_XY,
		PLANE_XZ, // Plane in XZ (left-right, forward-backward) axes.
		CIRCLE_XY,
		VECTOR, // Constant vector.

		NONE, // Default constructor.
	}; 
};

/// Based on types above, produces a given result.
class Emitter 
{
public:
	Emitter();
	// Sets default up/left/forward vectors.
	void DefaultVectors();
	// Scales all 3 base vectors.
	void Scale(float scale);
	// Sets scale of all 3 base vectors.
	void SetScale(float scale);
	/// Randomzies acordingly.
	void Position(Vector3f & vec);
	void Velocity(Vector3f & vec);
	/// Surface area in square-meters (or square-units, 1 unit in-game defaults to 1 meter, though).
	float SurfaceArea();
	int type;
	/// For later, Linear (pure random), weighted (constant and random part).
	int distribution;
	/// For configuring it.
	Vector3f up, left, forward;
	// Constant vector.
	Vector3f vec, vec2;
	/// Added after randomization.
	Vector3f offset;
	List<Triangle> * triangles;
	Random r;
};

#endif
