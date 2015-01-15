/// Emil Hedemalm
/// 2014-09-23
/** A node used for animation and rendering of a model which incorporates skeletal animation. 
	Developed in conjunction with COLLADA testing. Another common term would be just "Bone".
	See http://en.wikipedia.org/wiki/Skeletal_animation
*/

#include "ColladaNode.h"

#define Bone SkeletalAnimationNode 

class GraphicsState;
class EstimatorFloat;
class EstimatorVec3f;
class Estimator;

class SkeletalAnimationNode : public ColladaNode
{
public:
	SkeletalAnimationNode();
	virtual ~SkeletalAnimationNode();

	static void FreeAll();

	/*
	/// Parses this bone's data from the given element, creating children as necessary and parsing recursively until all has been parsed.
	virtual bool ParseCollada(XMLElement * element);
	/// Parses attributes, without touching any child-nodes. Usually called from within ParseCollada
	virtual void ParseAttributes(XMLElement * element);
	*/
	/// Parses rotations. Overload/subclass to allow more rotations to be parsed. Remember to call the base-class version too!
	virtual void ParseRotation(XMLElement * element);

	// Loads animations for this node, by comparing the source ID of this node with animation channel targets in the animation library.
	virtual void LoadAnimations(List<XMLElement*> & animationList);
	// Loads a single animation. Returns false if it fails.
	virtual bool LoadAnimation(XMLElement * fromAnimationElement);

	/** Animates the skeleton for given time. If loop is true, the time will be modulated to be within the interval of available time-stamps.
		Recommended to call UpdateMatrices afterwards in order to actually use the animated data when rendering.
	*/
	void AnimateForTime(int64 timeInMs, bool loop = false);
	/// Updates world-model matrices accordingly.
	void UpdateMatrices(Matrix4f & parentTransformationMatrix);

	/// Renders all bones! ovo Uses old GL because reasons.
	void RenderBones(GraphicsState & graphicsState);
	
	/// Orientation for the joint at this node?
	Vector4f jointOrientX, jointOrientY, jointOrientZ;

	/// Index as specified when loading from the document, starts at 0 for each skeleton's first bone.
	int boneIndex;

protected:
	/// Animation functions.
	List<EstimatorFloat*> animationsFloat;
	List<EstimatorVec3f*> animationsVec3f;
	List<Estimator*> animations;

	/// For static de-allocation and memory control.
	static List<Bone*> bones;

};




