/// Emil Hedemalm
/// 2015-02-20
/// Structure for containing a group of entities to be rendered instanced 
/// (one draw command), after the necessary buffers have been updated.

#include "Entity/Entity.h"
#include "List/List.h"
#include "Graphics/OpenGL.h"

/// Below more instancing options. The first entity registerd with a set of options will define the options used for that specific entity group.
namespace InstancingOption
{
	enum 
	{
		UPDATE_MATRICES = 1, // Use this if using the instancing system for non-static entities.
		UPDATE_EMISSIVE_MAP_OPTIONS = (1 << 1), // If true, will update emissive properties, such as sprite index (if many in one texture), or a basic multiplier, or both. 

	};
};

#define RIG RenderInstancingGroup

class RenderInstancingGroup : public List< std::shared_ptr<Entity> > 
{
	friend class GraphicsState;
public:
	RenderInstancingGroup();
	RenderInstancingGroup(EntitySharedPtr reference);
	virtual ~RenderInstancingGroup();
	void Nullify();
	
	/// o.o
	void AddEntity(EntitySharedPtr entity);
	/// Removes all occurences of any items in the sublist in this list. Also re-points the reference pointer if needed.
	void RemoveEntity(EntitySharedPtr entity);

	// Called once per frame. Called with force = true when adding an entity or several to the group.
	void UpdateBuffers(bool force = false);
	// Called once per viewport that is rendered.
	void Render(GraphicsState& graphicsState);

	/// Based on reference.
	String name;
	/// See enum above. Default 0.
	int options;

	/// o.o based on initial reference entity.
	bool isShadowCasting;
	bool isSolid;
private:
	/// Base model and texture on the reference.
	EntitySharedPtr reference;
	/// Necessary buffers?
	float * matrixData, 
		* normalMatrixData;
	/// Number of entities that fit in the current buffers.
	int bufferEntityLength;
	/// For instanced particle rendering. Some buffers.
	GLuint matrixBuffer,
		normalMatrixBuffer;
};
