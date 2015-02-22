/// Emil Hedemalm
/// 2014-09-23
/// "Declares a point of interest in a scene." - COLLADA spec.
/// http://www.khronos.org/files/collada_spec_1_5.pdf

#include "ColladaNode.h"

#include "XML/XMLElement.h"
#include "SkeletalAnimationNode.h"

ColladaNode::ColladaNode()
{
	rotateX = rotateY = rotateZ = Vector4f(0,0,0,0);
	colladaRef = NULL;
	parent = NULL;
}

/// Parses this bone's data from the given element, creating children as necessary and parsing recursively until all has been parsed.
bool ColladaNode::ParseCollada(XMLElement * element)
{
	colladaRef = element;
	// Delete children if we had any?
	assert(children.Size() == 0);
	children.ClearAndDelete();

	// Fetch attributes again if needed?
	ParseAttributes(element);
	
	// Parse children. Expect only translations, positions and more bones!
	for (int i = 0; i < element->children.Size(); ++i)
	{
		Xelement * child = element->children[i];

		// Data-related to this node.
		if (child->name == "rotate")
		{
			ParseRotation(child);
		}
		else if (child->name == "translate")
		{
			ParseTranslation(child);
		}

		// Children in the node-hierarchy
		if (child->name == "node")
		{
			// Preliminary parsing, determining node-type.
			ColladaNode node;
			node.ParseAttributes(child);
			/// For the real children.
			ColladaNode * newChild = NULL;
			switch(node.type)
			{
				case ColladaNodeType::JOINT:
				{
					SkeletalAnimationNode * bone = new SkeletalAnimationNode();
					newChild = bone;
					break;
				}
				case ColladaNodeType::NODE:
					newChild = new ColladaNode();
					break;
			
			}
			if (newChild)
			{
				// Parse moar.
				newChild->ParseCollada(child);
				newChild->parent = this;
				children.Add(newChild);
			}
		}
	}
	return true;
}

/// Parses attributes, without touching any child-nodes. Usually called from within ParseCollada
void ColladaNode::ParseAttributes(XMLElement * element)
{
	if (element == NULL)
		element = colladaRef;
	for (int i = 0; i < element->args.Size(); ++i)
	{
		Xarg * arg = element->args[i];
		if (arg->name == "id")
			id = arg->value;
		else if (arg->name == "name")
			name = arg->value;
		else if (arg->name == "sid")
			this->sid = arg->value;
		else if (arg->name == "type")
		{
			typeString = arg->value;
			if (typeString == "NODE")
				type = ColladaNodeType::NODE;
			else if (typeString == "JOINT")
				type = ColladaNodeType::JOINT;
		}
	}
}


/// Parses data from a non-node child.
void ColladaNode::ParseTranslation(XMLElement * element)
{
	// check source id just to be sure
	translation.ReadFrom(element->data);
}
void ColladaNode::ParseRotation(XMLElement * element)
{
	Xarg * arg = element->GetArgument("sid");
	String rotation = arg->value;
	if (rotation == "rotateZ")
		rotateZ.ReadFrom(element->data, " ,");
	else if (rotation == "rotateY")
		rotateY.ReadFrom(element->data, " ,");
	else if (rotation == "rotateZ")
		rotateZ.ReadFrom(element->data, " ,");
}


/// Returns bone by given index.
SkeletalAnimationNode * ColladaNode::GetBoneByIndex(int index)
{
	if (type == ColladaNodeType::JOINT && ((Bone*)this)->boneIndex == index)
		return (Bone*)this;
	for (int i = 0; i < children.Size(); ++i)
	{
		ColladaNode * child = children[i];
		ColladaNode * result = child->GetBoneByIndex(index);
		if (result)
			return (Bone*)result;
	}
	return NULL;
}

/// Assigns bone index based on where the bone finds itself in the provided list.
void ColladaNode::AssignBoneIndices(List<String> & names)
{
	if (type == ColladaNodeType::JOINT)
	{
		Bone * bone = (Bone*) this;
		for (int i = 0; i < names.Size(); ++i)
		{
			String name = names[i];
			if (name == bone->sid)
			{
				bone->boneIndex = i;
				break;
			}
		}
	}
	for (int i = 0; i < children.Size(); ++i)
	{
		ColladaNode * child = children[i];
		child->AssignBoneIndices(names);
	}
}
	
/// Fetches bones to target list. Works recursively, adding all nodes of JOINT type.
void ColladaNode::FetchBones(List<SkeletalAnimationNode*> & list)
{
	// Add self if is a bone o.o
	if (this->type == ColladaNodeType::JOINT)
		list.Add((Bone*)this);

	// Iterate children recursively.
	for (int i = 0; i < children.Size(); ++i)
	{
		ColladaNode * child = children[i];
		child->FetchBones(list);
	}
}


/// Updates world-model matrices accordingly.
void ColladaNode::UpdateMatrices(Matrix4f & parentTransformationMatrix)
{
	// Translate the matrix according to the specification?
	Matrix4f newBoneModelMatrix = parentTransformationMatrix;
	newBoneModelMatrix *= Matrix4f::InitTranslationMatrix(translation);
	
	newBoneModelMatrix *= Matrix4f::InitRotationMatrixZ(DEGREES_TO_RADIANS(this->rotateZ[3]));
	newBoneModelMatrix *= Matrix4f::InitRotationMatrixY(DEGREES_TO_RADIANS(this->rotateY[3]));
	newBoneModelMatrix *= Matrix4f::InitRotationMatrixX(DEGREES_TO_RADIANS(this->rotateX[3]));
	// o.o
	nodeModelMatrix = newBoneModelMatrix;
	// Update kids.
	for (int i = 0; i < children.Size(); ++i)
	{
		ColladaNode * child = children[i];
		child->UpdateMatrices(nodeModelMatrix);
	}
}


