/// Emil Hedemalm
/// 2014-09-23
/// "Declares a point of interest in a scene." - COLLADA spec.
/// http://www.khronos.org/files/collada_spec_1_5.pdf

#ifndef COLLADA_NODE_H
#define COLLADA_NODE_H

#include "String/AEString.h"
#include "MathLib.h"

class XMLElement;
class SkeletalAnimationNode;

namespace ColladaNodeType
{
	enum colladaNodeTypes 
	{
		NODE,
		JOINT,
	};
};

class ColladaNode 
{
public:
	ColladaNode();

	/// Parses this bone's data from the given element, creating children as necessary and parsing recursively until all has been parsed.
	virtual bool ParseCollada(XMLElement * element);
	/// Parses attributes, without touching any child-nodes. Usually called from within ParseCollada
	virtual void ParseAttributes(XMLElement * element = NULL);
	/// Parses data from a non-node child.
	void ParseTranslation(XMLElement * element);
	/// Parses rotations. Overload/subclass to allow more rotations to be parsed. Remember to call the base-class version too!
	virtual void ParseRotation(XMLElement * element);

	/// Returns bone by given index.
	SkeletalAnimationNode * GetBoneByIndex(int index);
	/// Assigns bone index based on where the bone finds itself in the provided list.
	void AssignBoneIndices(List<String> & byNames);
	/// Fetches bones to target list. Works recursively, adding all nodes of JOINT type.
	void FetchBones(List<SkeletalAnimationNode*> & list);


	/// Updates world-model matrices accordingly.
	virtual void UpdateMatrices(Matrix4f & parentTransformationMatrix);

	String name;
	String id;
	// Resource/Source id. Used to either declare a new resource or refere to an existing one if it starts with a hash #
	String sid;
	// See enum above.
	int type; 
	// Parsed type string.
	String typeString;

	/// Translation relative to parent or model to which the node is attached to.
	Vector3f translation;
	/// Rotation along their respective axes.
	Vector4f rotateX, rotateY, rotateZ;

	/// Model matrix for this specific joint. All parent matrices have been pre-multiplied into it.
	Matrix4f nodeModelMatrix;

	/// Child nodes
	ColladaNode * parent;
	List<ColladaNode*> children;

	/// Reference when parsing or writing?
	XMLElement * colladaRef;

protected:


};


#endif
