/// Emil Hedemalm
/// 2014-09-23
/** A node used for animation and rendering of a model which incorporates skeletal animation. 
	Developed in conjunction with COLLADA testing. Another common term would be just "Bone".
	See http://en.wikipedia.org/wiki/Skeletal_animation
*/

#include "SkeletalAnimationNode.h"

#include "String/StringUtil.h"

#include "XML/XMLElement.h"

#include "GraphicsState.h"
#include "Graphics/OpenGL.h"
#include "Graphics/Fonts/TextFont.h"

#include "PhysicsLib/EstimatorFloat.h"
#include "PhysicsLib/EstimatorVec3f.h"

List<Bone*> Bone::bones;

SkeletalAnimationNode::SkeletalAnimationNode()
	: ColladaNode()
{
	jointOrientZ = jointOrientY = jointOrientX = Vector4f(0,0,0,0);
	boneIndex = -1;

	bones.Add(this);
}

SkeletalAnimationNode::~SkeletalAnimationNode()
{
	bones.Remove(this);
}

void Bone::FreeAll()
{
	bones.ClearAndDelete();
}

/// Parses rotations. Overload/subclass to allow more rotations to be parsed. Remember to call the base-class version too!
void SkeletalAnimationNode::ParseRotation(XMLElement * element)
{
	// Parse base rotations first.
	ColladaNode::ParseRotation(element);
	// Then bone-specific rotations.
	Xarg * arg = element->GetArgument("sid");
	String rotation = arg->value;
	if (rotation == "jointOrientZ")
		jointOrientZ.ReadFrom(element->data,  " ,");
	else if (rotation == "jointOrientY")
		jointOrientY.ReadFrom(element->data, " ,");
	else if (rotation == "jointOrientX")
		jointOrientX.ReadFrom(element->data,  " ,");
}

// Loads animations for this node, by comparing the source ID of this node with animation channel targets in the animation library.
void SkeletalAnimationNode::LoadAnimations(List<XMLElement*> & animationList)
{
	for (int i = 0; i < animationList.Size(); ++i)
	{
		Xelement * anim = animationList[i];	
		// Check if the target of the channel corresponds to us?
		Xelement * channel = anim->GetElement("channel");
		if (!channel)
			continue;

		Xarg * arg = channel->GetArgument("target");
		List<String> path = arg->value.Tokenize("/");
		for (int i = 0; i < path.Size(); ++i)
		{
			String pathToken = path[i];
			// Found one! \owo/
			if (pathToken == id)
			{
				LoadAnimation(anim);
			}
		}
	}
	std::cout<<"\n"<<animationsFloat.Size()<<" animations found for joint "<<id;

	// Load animations recursively.
	for (int i = 0; i < children.Size(); ++i)
	{
		ColladaNode * child = children[i];
		if (child->type != ColladaNodeType::JOINT)
			continue;
		Bone * bone =  (Bone*)child;
		bone->LoadAnimations(animationList);
	}
}	

// Loads a single animation.
bool SkeletalAnimationNode::LoadAnimation(XMLElement * fromAnimationElement)
{
	Estimator * estimator = NULL;
	EstimatorVec3f * vec3fAnim = NULL;
	EstimatorFloat * floatAnim = NULL;
	Xelement * animElement = fromAnimationElement;
		
	// Parse channel for output target
	Xelement * channel = fromAnimationElement->GetElement("channel");
	String target = channel->GetArgument("target")->value;
	
	String targetVar = target.Tokenize("/").Last();

	// Maya-style.
	if (targetVar.Contains("translate"))
	{
		// Vector based estimator required.
		vec3fAnim = new EstimatorVec3f();
		animationsVec3f.Add(vec3fAnim);
		estimator = vec3fAnim;
		// Set output location
		vec3fAnim->variableToPutResultTo = &translation;
	}
	else if (targetVar.Contains("rotate"))
	{
		floatAnim = new EstimatorFloat();
		estimator = floatAnim;
		animationsFloat.Add(floatAnim);

		/*
		if (targetVar.Contains("X"))
			floatAnim->variableToPutResultTo = &jointOrientX[3];
		if (targetVar.Contains("Y"))
			floatAnim->variableToPutResultTo = &jointOrientY[3];
		if (targetVar.Contains("Z"))
			floatAnim->variableToPutResultTo = &jointOrientZ[3];
			*/

		// Old one,
		if (targetVar.Contains("X"))
			floatAnim->variableToPutResultTo = &rotateX[3];
		if (targetVar.Contains("Y"))
			floatAnim->variableToPutResultTo = &rotateY[3];
		if (targetVar.Contains("Z"))
			floatAnim->variableToPutResultTo = &rotateZ[3];
			
	}
	// Bad target.
	else {
	
		std::cout<<"\nBad target detected \'"<<targetVar<<"\' Skipping this animation.";
		return false;
	}

	// Check the sampler to see which source array data is to be used for what.
	Xelement * sampler = fromAnimationElement->GetElement("sampler");
	List<Xelement*> inputsElements = sampler->GetElements("input");

	String inputSource, outputSource;
	for (int i = 0; i < inputsElements.Size(); ++i)
	{
		Xelement * inputElement = inputsElements[i];
		Xarg * semantic = inputElement->GetArgument("semantic");
		Xarg * sourceElement = inputElement->GetArgument("source");
		String source = sourceElement->value;
		source.Remove("#");
		if (semantic->value == "INPUT")
		{
			inputSource = source;
		}
		else if (semantic->value == "OUTPUT")
		{
			outputSource = source;
		}
	}

	// Fetch input!
	Xelement * sourceInputElement = animElement->GetElement("source", "id", inputSource);
	// Fetch time-stamps.
	Xelement * floatArrayElement = sourceInputElement->GetElement("float_array");
	List<String> timeStampStrings = floatArrayElement->data.Tokenize(" \t\n");
	List<float> timeStamps = StringListToFloatList(timeStampStrings);
	/// Add new states with the time-stamps to the estimator!.. ?
	// Fetch moar data first. o.o
	
	// Fetch output! owo
	Xelement * sourceOutputElement = animElement->GetElement("source", "id", outputSource);
	// Fetch time-stamps.
	floatArrayElement = sourceOutputElement->GetElement("float_array");
	List<String> dataStrings = floatArrayElement->data.Tokenize(" \t\n");
	/// Check in the accessor what the stride is, as it defines the data type needed to store the values properly.
	int stride = sourceOutputElement->GetElement("accessor")->GetArgument("stride")->value.ParseInt();

	List<float> floatValues = StringListToFloatList(dataStrings);
	/// Add new states as specified by stride!
	for (int i = 0; i < timeStamps.Size(); ++i)
	{
		float timeStamp = timeStamps[i];
		float timeInMs = timeStamp * 1000;
		switch(stride)
		{
			case 1:
			{
				float fValue = floatValues[i];
			//	std::cout<<"\nfValue: "<<fValue;
				floatAnim->AddStateMs(fValue, timeInMs);
				break;
			}
			case 2:
			{
				Vector3f vector(floatValues[i * stride], floatValues[i * stride + 1], 0);
				vec3fAnim->AddState(vector, timeInMs);
				break;
			}
			case 3:
			{
				Vector3f vector(floatValues[i * stride], floatValues[i * stride + 1], floatValues[i * stride + 2]);
				vec3fAnim->AddState(vector, timeInMs);
				break;
			}
		}
	}
	

	// Parse all data!
	List<Xelement*> sources = animElement->GetElements("source");
	for (int i = 0; i < sources.Size(); ++i)
	{

		Xelement * source = sources[i];
	}

	// Save it in the aggregate list.
	animations.Add(estimator);
	return true;
}

/** Animates the skeleton for given time. If loop is true, the time will be modulated to be within the interval of available time-stamps.
	Recommended to call UpdateMatrices afterwards in order to actually use the animated data when rendering.
*/
void SkeletalAnimationNode::AnimateForTime(int64 timeInMs, bool loop /*= false*/)
{
	// Apply animations
	for (int i = 0; i < animations.Size(); ++i)
	{
		Estimator * animation = animations[i];
		animation->Estimate(AETime(TimeType::MILLISECONDS_NO_CALENDER, timeInMs), loop);
	}
	
	// Update matrices.


	// Update children.
	for (int i = 0; i < children.Size(); ++i)
	{
		ColladaNode * node = children[i];
		if (node->type == ColladaNodeType::JOINT)
		{
			Bone * bone = (Bone*) node;
			bone->AnimateForTime(timeInMs, loop);
		}
	}
}

/// Updates world matrices accordingly.
void SkeletalAnimationNode::UpdateMatrices(Matrix4f & parentTransformationMatrix)
{
	// Translate the matrix according to the specification?
	Matrix4f newBoneModelMatrix = parentTransformationMatrix;
	newBoneModelMatrix *= Matrix4f::InitTranslationMatrix(translation);
	newBoneModelMatrix *= Matrix4f::GetRotationMatrixZ(DEGREES_TO_RADIANS(this->jointOrientZ[3]));
	newBoneModelMatrix *= Matrix4f::GetRotationMatrixY(DEGREES_TO_RADIANS(this->jointOrientY[3]));
	newBoneModelMatrix *= Matrix4f::InitRotationMatrixX(DEGREES_TO_RADIANS(this->jointOrientX[3]));

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

/// Renders all bones! ovo Uses old GL because reasons.
void SkeletalAnimationNode::RenderBones(GraphicsState & graphicsState)
{
	// Push the matrix.
	Matrix4f modelMatrix = graphicsState.modelMatrixF;
//	graphicsState.modelMatrixF.LoadIdentity();

	// Fetch absolute positions, or render relatively...?

	// Multiply the pre-multiplied joint matrix onto the given matrix.
	Matrix4f mergedModelMatrix = modelMatrix * nodeModelMatrix;

	// Recalculate model-view matrix for rendering the initial bone..?
	Matrix4f modelView = graphicsState.viewMatrixF * mergedModelMatrix;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelView.getPointer());
	

	// Assert that this is a bone?
	Vector3f position = this->translation;
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set color based on joint-type.
	if (type == ColladaNodeType::JOINT)
	{
		if (parent == NULL)
			glColor4f(0,1,0,1);
		else if (children.Size() == 0)
			glColor4f(1,0,0,1);
		else 
			glColor4f(1,1,1,1);
	}
	else 
		glColor4f(1,1,1,0.5f);

	glPointSize(3.f);

	// Draw some lines.
	glBegin(GL_POINTS);		
		glVertex3f(0, 0, 0);
	glEnd();

	// Set a special matrix for rendering the text orthogonally towards the camera..?
	Vector3f pointInWorldSpace = mergedModelMatrix.Product(Vector3f(0,0,0));

	// Translate the matrix according to the specification?
	/* // Multiplicatoin code for the matrix has moved to UpdatedMatrices()
	graphicsState.modelMatrixF *= Matrix4f::InitTranslationMatrix(translation);
	graphicsState.modelMatrixF *= Matrix4f::GetRotationMatrixZ(DEGREES_TO_RADIANS(this->jointOrientZ[3]));
	graphicsState.modelMatrixF *= Matrix4f::GetRotationMatrixY(DEGREES_TO_RADIANS(this->jointOrientY[3]));
	graphicsState.modelMatrixF *= Matrix4f::InitRotationMatrixX(DEGREES_TO_RADIANS(this->jointOrientX[3]));

	graphicsState.modelMatrixF *= Matrix4f::InitRotationMatrixZ(DEGREES_TO_RADIANS(this->rotateZ[3]));
	graphicsState.modelMatrixF *= Matrix4f::InitRotationMatrixY(DEGREES_TO_RADIANS(this->rotateY[3]));
	graphicsState.modelMatrixF *= Matrix4f::InitRotationMatrixX(DEGREES_TO_RADIANS(this->rotateX[3]));
	*/


	/*
	graphicsState.modelMatrixF *= Matrix4f::GetRotationMatrixZ(DEGREES_TO_RADIANS(this->rotateZ[3]));
	graphicsState.modelMatrixF *= Matrix4f::GetRotationMatrixY(DEGREES_TO_RADIANS(this->rotateY[3]));
	graphicsState.modelMatrixF *= Matrix4f::InitRotationMatrixX(DEGREES_TO_RADIANS(this->rotateX[3]));
	*/

	bool renderText = true;
	if (renderText)
	{
		Matrix4f tmp = graphicsState.modelMatrixF;
		
		Matrix4f translationMatrix = Matrix4f::InitTranslationMatrix(pointInWorldSpace);
		// Scale up the text!
		float textScale = 20.f;
		Matrix4f scaleMatrix = Matrix4f::InitScalingMatrix(Vector3f(textScale, textScale, textScale));
		graphicsState.modelMatrixF = translationMatrix * scaleMatrix;

		TextFont * font = graphicsState.currentFont;
		assert(font);
		Text nameText = name;
		font->RenderText(nameText, graphicsState);

		graphicsState.modelMatrixF = tmp;
	}

	// Render children!
	for (int i = 0; i < children.Size(); ++i)
	{
		SkeletalAnimationNode * child = (SkeletalAnimationNode *) children[i];
		child->RenderBones(graphicsState);
	}

	// Pop the matrix.
	graphicsState.modelMatrixF = modelMatrix;
}

