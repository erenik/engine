/// Emil Hedemalm
/// 2016-04-30
/// Emitter for getting randomly - or structured randomly - selected points/vectors within some desired area.

#include "Emitter.h"
#include "MathLib/Angle3.h"

Emitter::Emitter()
{
	DefaultVectors();
	type = EmitterType::SPHERE;
	triangles = 0;
}

Emitter::Emitter(EmitterType type)
	: type(type)
{
	DefaultVectors();
	triangles = 0;
}

// Sets default up/left/forward vectors.
void Emitter::DefaultVectors()
{
	up = Vector3f(0,1,0);
	left = Vector3f(-1,0,0);
	forward = Vector3f(0,0,-1);
}

// Scales all 3 base vectors.
void Emitter::Scale(float scale)
{
	up *= scale;
	left *= scale;
	forward *= scale;
}

// Sets scale of all 3 base vectors.
void Emitter::SetScale(float scale)
{
	DefaultVectors();
	up *= scale;
	left *= scale;
	forward *= scale;
}

/// Randomizes acordingly.
void Emitter::Position(Vector3f & positionVec)
{
	switch(type)
	{
		case EmitterType::SPHERE:
		{
			// Randomize yaw and pitch.
			float pitch = rand() * oneDivRandMaxFloat * PI * 0.5;
			float yaw = rand() * oneDivRandMaxFloat * TwoPI;
			Vector3d dir = Angle3::VectorFromPitchYawForwardZMinus(pitch,yaw);
			positionVec = -dir.x * left + dir.y * up + dir.z * forward;
			break;	
		}
		case EmitterType::POINT:
			positionVec = vec;
			break;
		case EmitterType::LINE:
		{
			positionVec = (rand() * oneDivRandMaxFloat - 0.5f) * vec;
			break;
		}
		case EmitterType::LINE_BOX:
		{
			positionVec = (rand() * oneDivRandMaxFloat - 0.5f) * vec +
				(rand() * oneDivRandMaxFloat - 0.5f) * vec2;
			break;
		}
		case EmitterType::LINE_Y:
			positionVec = (rand() * oneDivRandMaxFloat - 0.5f) * up;
			break;
		case EmitterType::PLANE_XY:
			positionVec = (rand() * oneDivRandMaxFloat - 0.5f) * left +
				(rand() * oneDivRandMaxFloat - 0.5f) * up;
			break;
		case EmitterType::PLANE_XZ:
			positionVec = (rand() * oneDivRandMaxFloat - 0.5f) * left +
				(rand() * oneDivRandMaxFloat - 0.5f) * forward;
			break;
		case EmitterType::CIRCLE_XY:
		{
			// Random angle.
			float angle = (rand() * oneDivRandMaxFloat * 2 * PI);
			positionVec = Vector3f(cos(angle), sin(angle), 0) * (rand() * oneDivRandMaxFloat);
			break;
		}
		case EmitterType::CIRCLE_ARC_XY: {
			// Random angle.
			float angle = arcOffset + (rand() * oneDivRandMaxFloat * arcLength) - arcLength * 0.5f;
			positionVec = Vector3f(cos(angle), sin(angle), 0);
			break;
		}
		case EmitterType::WEIGHTED_CIRCLE_ARC_XY: {
			// Random angle.
			float randomWeight = pow(rand() * oneDivRandMaxFloat, weight); // Generates a value between 0 and 1.0, closer to 0 the bigger the weight.
			float angle = arcOffset + (rand() * oneDivRandMaxFloat * arcLength * randomWeight) - arcLength * 0.5f * randomWeight;
			positionVec = Vector3f(cos(angle), sin(angle), 0);
			break;
		}
		case EmitterType::VECTOR:
			positionVec = vec;
			break;
		case EmitterType::TRIANGLES: 
		{
			/// Grab triangle.
			Triangle & tri = (*triangles)[rand() % triangles->Size()];
			/// Get random within.
			float x = r.Randf();
			float y = r.Randf(1-x);
			float z = 1-x-y;
			Vector3f parts(x, y, z);
//			std::cout<<"\nparts "<<parts;
			positionVec = tri.point1 * parts.x + tri.point2 * parts.y + tri.point3 * parts.z;
//			std::cout<<" pos "<<positionVec;
	//		std::cout<<"\ntri1 "<<tri.point1<<" 2: "<<tri.point2<<" 3: "<<tri.point3;
			break;
		}
		default:
			assert(false);
	}
	positionVec += offset;
}

void Emitter::Velocity(Vector3f & vec)
{
	 return Position(vec);
}

/// Surface area in square-meters (or square-units, 1 unit in-game defaults to 1 meter, though).
float Emitter::SurfaceArea()
{
	switch(type)
	{
		case EmitterType::PLANE_XZ:
		{
			return left.Length() * forward.Length();
		}
		default:
			assert(false);
	}
}

