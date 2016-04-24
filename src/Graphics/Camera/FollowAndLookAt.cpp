/// Emil Hedemalm
/// 2016-04-24
/// Follow- and LookAt- camera functionalities, for the preset of FollowAndLookAt (MMORPG-style or cinematic camera).

#include "Camera.h"
#include "Entity/Entity.h"

/// MMORPG style tracking and following.
void Camera::FollowAndLookAt()
{
	Follow();
	LookAt(false);
	/// Copy over so calculation works...
	positionWithOffsets = position;
}

void Camera::Follow()
{
	/// Copied from third-person Distance
	{
		// Get data
		Vector3f entityPosition = entityToTrack->worldPosition;
	
		/// The actual position of the camera at the moment.
		Vector3f currentCamPosition = position;
		// Look at it from our new position!
		// Find rotation yaw needed.
		Vector3f toEntity = entityPosition - currentCamPosition;
		Vector2f toEntityXZ(toEntity[0], toEntity[2]);
		Vector2f toEntityXZNormalized = toEntityXZ.NormalizedCopy();

		float xzDistance = Vector2f(toEntity[0],toEntity[2]).Length();
		Vector2f toEntityXY(xzDistance, toEntity[1]);
		toEntityXY.Normalize();
	
		// Far away? get closer?
		// First update toEntity since it will be using the offsetted position from earlier, we want the flat XZ diff.
		float dist = toEntityXZ.Length();
		float relevantTrackingDistance = dist;
		minTrackingDistance = trackingDistanceBase - trackingDistanceLeeway;
		maxTrackingDistance = trackingDistanceBase + trackingDistanceLeeway;
		minTrackingDistance *= trackingDistanceMultiplier;
		maxTrackingDistance *= trackingDistanceMultiplier;
		bool hardSet = false;
		if (relevantTrackingDistance > maxTrackingDistance || 
			relevantTrackingDistance < minTrackingDistance)
		{
			hardSet = true;
		}
		ClampFloat(relevantTrackingDistance, minTrackingDistance, maxTrackingDistance);
		float currentDiff = dist - relevantTrackingDistance;
		float newDist = relevantTrackingDistance;
		if (toEntityXZNormalized.LengthSquared() == 0)
			toEntityXZNormalized = Vector2f(0,1);
		// XZ to XYZ
		Vector3f toMove(toEntityXZNormalized.x, 0, toEntityXZNormalized.y);
		toMove *= -newDist;
		// Reduce the diff.
		Vector3f newPosition = entityPosition + toMove;
		/// Save old Y.
		newPosition.y = position.y;
		/// Adjust position to go to minimum if below it.
		if (minimumDiff.y != 0 && toEntity.y > minimumDiff.y)
		{
			newPosition.y += minimumDiff.y * 0.1f; // Doesn't have to be hard minimum cap.
		}
		float maxDiff = 10.f;
		if (toEntity.y < -maxDiff)
			newPosition.y -= maxDiff * 0.1f;

		/// Smooth it.
		if (hardSet) // lolwat
			position = newPosition;
		else
			position = newPosition;
	}
}

void Camera::LookAt(bool ignoreSmoothing)
{
	/// Copied from ThirdPersonLookAt
	{
		// Get data
		Vector3f entityPosition = entityToTrack->worldPosition;
	
		/// The actual position of the camera at the moment.
		Vector3f currentCamPosition = camPos;
		// Look at it from our new position!
		// Find rotation yaw needed.
		Vector3f toEntity = entityPosition - currentCamPosition;
		Vector2f toEntityXZ(toEntity[0], toEntity[2]);
		Vector2f toEntityXZNormalized = toEntityXZ.NormalizedCopy();
		float yawNeeded = atan2(toEntityXZNormalized.y, toEntityXZNormalized.x);
		yawNeeded += PI * 0.5f;

		float xzDistance = Vector2f(toEntity[0],toEntity[2]).Length();
		Vector2f toEntityXY(xzDistance, toEntity[1]);
		toEntityXY.Normalize();
		float pitchNeeded = -atan2(toEntityXY[1], toEntityXY[0]);

		Angle currentPitch = rotation.x;
		Angle currentYaw = rotation.y;
		Angle toYaw = Angle(pitchNeeded) - currentPitch; // Already smoothed later.
//		toYaw *= 1 - trackingRotationalSmoothness;
		Angle toPitch = Angle(yawNeeded) - currentYaw;
//		toPitch *= 1 - trackingRotationalSmoothness;
		Angle smoothedPitch(currentPitch + toYaw);
		Angle smoothedYaw(currentYaw + toPitch);
		// Set rotations accordingly.
		rotation.x = smoothedPitch.Radians();
		rotation.y = smoothedYaw.Radians();
		if (ignoreSmoothing)
			smoothedRotation = rotation;
	}
}


// Processes input (rotation, zoom, etc) for FollowAndLookAt cameras.
void Camera::ProcessFollowAndLookAtMovement(float timeInSeconds)
{
	Vector3f radiansToTurn = rotationalVelocityEuler * timeInSeconds;
	if (radiansToTurn.y != 0)
	{
		/// Smooth MMORPG-style. Check if we have a target entity. If so, rotate around it.
		Vector3f entityPos = entityToTrack->worldPosition;
		Vector3f toEntity = position - entityToTrack->worldPosition;
		// Sideways rotation. Radians per second.
		Vector3f radiansToTurn = rotationalVelocityEuler * timeInSeconds;
		Vector3f toEntityXZ(toEntity.x, 0, toEntity.z);
		float distance = toEntity.Length();
		float distXZ = toEntityXZ.Length();
		/// Get current radians.
		Angle currentYaw(Vector2f(toEntity.x, toEntity.z).NormalizedCopy());
		currentYaw -= radiansToTurn.y; // Add some radians.
		// Make new vector, same distance.
		Vector2f newVec = currentYaw.ToVector2f();
		Vector3f newDiff = Vector3f(newVec.x, 0, newVec.y) * distXZ;
		newDiff.y = toEntity.y;
		Vector3f newPosition = entityPos + newDiff;
//		std::cout<<"\nOld position: "<<position<<" new position: "<<newPosition;
		// New position.
		position = newPosition;
		// Reset smoothed position too?
//		smoothedPosition = position;

		/// Update looking.
		LookAt(false);
	}
	// Rotate up/down.
	if (radiansToTurn.x != 0)
	{
		/// Smooth MMORPG-style. Check if we have a target entity. If so, rotate around it.
		Vector3f entityPos = entityToTrack->worldPosition;
		Vector3f toEntity = position - entityToTrack->worldPosition;
		// Sideways rotation. Radians per second.
		Vector3f toEntityXZ(toEntity.x, 0, toEntity.z);
		float distance = toEntity.Length();
		float distXZ = toEntityXZ.Length();
		/// Get current radians.
		Vector2f oldPitchVec = Vector2f(distXZ, toEntity.y);
		Vector2f opvNorm = oldPitchVec.NormalizedCopy();
		Angle currentPitch(opvNorm);
		currentPitch -= radiansToTurn.x; // Add some radians.
		// Make new vector, same distance.
		Vector2f newVec = currentPitch.ToVector2f();
		float newXZ = newVec.x * distance;
		float newY = newVec.y * distance;
		// Further multiply again X and Z components to get their position.
		float newX = newXZ * toEntityXZ.NormalizedCopy().x;
		float newZ = newXZ * toEntityXZ.NormalizedCopy().z;
		Vector3f newDiff(newX, newY, newZ);
		Vector3f newPosition = entityPos + newDiff;
//					std::cout<<"\nOld position: "<<position<<" new position: "<<newPosition;
		// New position.
		position = newPosition;
		/// Adjust rotation too so it doesn't lag too much.
//		rotationEuler.y -= radiansToTurn.y * 20;
	}

}