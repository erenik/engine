// Emil Hedemalm
// 2013-07-03 Linuxification.

#include "Graphics/GraphicsManager.h"

#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/Springs/Spring.h"
#include "Physics/Collision/CollisionShapeOctree.h"

#include "PhysicsLib/PhysicsMesh.h"
#include "PhysicsLib/Shapes/AABB.h"
#include "PhysicsLib/Shapes/Quad.h"
#include "PhysicsLib/Shapes/Cube.h"

#include "GraphicsState.h"
#include "Viewport.h"

#include "Model/ModelManager.h"
#include "Model/Model.h"

#define PRINT_ERROR std::cout<<"\nGLError in Render "<<error;

/// Wosh!
void RenderLine(const Vector3f & origin, const Vector3f & end, const Vector4f & baseColor);
void RenderFadingLine(const Vector3f & origin, const Vector3f & end, const Vector4f & baseColor, const Vector4f & highlightColor);

Entities physicalEntities;
long lastFetch = 0;
/// Renders physical bounds for all entities within the frustum. Does a local frustum culling that will slow down the system more ^^
void GraphicsManager::RenderPhysics()
{
    // Fetch new entities only if the camera has been updated and max once per sec to not lagg too much.
	if (clock() > lastFetch + CLOCKS_PER_SEC * 0.5f){
		physicalEntities = Physics.GetEntities();
		for (int i = physicalEntities.Size()-1; i >= 0; --i){
			if (physicalEntities[i]->registeredForPhysics == false)
				physicalEntities.Remove(physicalEntities[i]);
		}
		if (physicalEntities.Size() <= 0)
			return;
		// Cull the shiat
		physicalEntities.CullByCamera(graphicsState.camera);
		lastFetch = clock();
	}

	// Disable depth-test so that all selections are rendered to screen no matter how we've done stuff earlier.
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1.0f);

	// Set default shading program
	Shader * shader = ShadeMan.SetActiveShader(&graphicsState, "Wireframe");

	/// Set rainbow factor for XYZ ^w^
	float rainbowXYZFactor = 0.5f;
	// Set color of the wireframes of all selected objects
	glUniform4f(shader->uniformPrimaryColorVec4, 0.0f, 0.8f, 1.0f, 0.2f);
	// Set projection and view matrices just in-case too.
	glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, graphicsState.viewMatrixF.getPointer());
	GLuint error = glGetError();
	glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, graphicsState.projectionMatrixF.getPointer());
	error = glGetError();

	/// Disable crap
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float z = -4;
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	graphicsState.currentTexture = NULL;
	// Disable lighting
	glDisable(GL_LIGHTING);
	error = glGetError();
	if (error != GL_NO_ERROR){
		PRINT_ERROR
	}
	glDisable(GL_COLOR_MATERIAL);

	// Render the entities now also, muppet
	// Set to wireframe
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//	glPolygonMode(GL_FRONT, GL_FILL);
	for (int i = 0; i < physicalEntities.Size(); ++i){
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		Entity* entity = physicalEntities[i];
		if (entity == NULL){
			std::cout<<"\nWARNING: NULL pointer in physical entities selection when rendering physical wireframes.";
			continue;
		}
		/// Set model matrix
		/// Apply transformation
		Matrix4d transformationMatrix = entity->transformationMatrix;

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /// Render AABB
        const GLuint & uniformPrimaryColorVec4 = shader->uniformPrimaryColorVec4;
       // assert(shader->uniformPrimaryColorVec4 != -1);
    //    std::cout<<"\n";
    //    std::cout<<"Shader: "<<shader->name;
    //    std::cout<<" UPC4: "<<uniformPrimaryColorVec4;
        PhysicsProperty * physics = entity->physics;
		if (!physics)
			continue;
		Shader * shader = ActiveShader();
        switch(physics->collissionState){
            case AABB_IDLE:
                glUniform4f(shader->uniformPrimaryColorVec4, 0.3f, 0.3f, 0.3f, 1.0f);
                break;
            case AABB_INTERSECTING:
          //      std::cout<<"\nAABB_INTERSECTING!";
                glUniform4f(shader->uniformPrimaryColorVec4, 0.8f, 0.7f, 0.3f, 1.0f);
                break;
            case COLLIDING:
                glUniform4f(shader->uniformPrimaryColorVec4, 1.0f, 0.2f, 0.2f, 1.0f);
                break;
            default:
                glUniform4f(shader->uniformPrimaryColorVec4, 0.5f, 0.5f, 0.5f, 1.0f);
        }

      //  if (i == 0){
           // glColor4f(0.5f,0.5f,0.5f,1.0f);
            glUniform1f(glGetUniformLocation(shader->shaderProgram, "rainbowXYZFactor"), 0.2f);
            Matrix4f aabbMatrix;
            AABB * aabb = entity->aabb;
        //    std::cout<<"\nAABB: Position: "<<aabb.position<<" scale: "<<aabb.scale<<" min: "<<aabb.min<<" max: "<<aabb.max;
			aabbMatrix = Matrix4f::InitTranslationMatrix(aabb->position);
            aabbMatrix.Scale(aabb->scale);

            glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, aabbMatrix.getPointer());
            Model * cube = ModelMan.GetModel("cube.obj");
            cube->Render(&graphicsState);
      //  }

        glUniform1f(glGetUniformLocation(shader->shaderProgram, "rainbowXYZFactor"), rainbowXYZFactor);

        // Should only need to translate to physical position straight away since that's what we're rendering...!
	//	transformationMatrix.multiply((Matrix4d().translate(Vector3d(entity->physics->physicalPosition))));
		Model * model = NULL;
		GLuint error  = glGetError();
		switch(entity->physics->shapeType){
	//		PLANE,
	//CYLINDER,
	//SPHERE,		// Uses the Entity's internal [radius] and [position]
	//MESH,
			case ShapeType::TRIANGLE: {
				Plane * plane = (Plane*)entity->physics->shape;
				model = ModelMan.GetModel("plane.obj");
				break;
			}
			case ShapeType::QUAD: {
				Plane * plane = (Plane*)entity->physics->shape;
				model = ModelMan.GetModel("plane.obj");
				break;
			}
			case ShapeType::PLANE: {
				Plane * plane = (Plane*)entity->physics->shape;
				model = ModelMan.GetModel("plane.obj");
				break;
			}
			case ShapeType::CUBE: {
			//	model = ModelMan.GetModel("cube.obj");

				// First render sphere!, then normal mesh.
				// Multiply by entity

				Cube * sphere = (Cube*)entity->physics->shape;
				ShadeMan.SetActiveShader(&graphicsState, nullptr);

				/// Set matrices
				glMatrixMode(GL_PROJECTION);
				glLoadMatrixf(graphicsState.projectionMatrixF.getPointer());
				glMatrixMode(GL_MODELVIEW);
				Matrix4f modelView = graphicsState.viewMatrixF;
				glLoadMatrixf(modelView.getPointer());
				// Disable lighting, enabling pure color-rendering
				glDisable(GL_LIGHTING);
				// Disable Depth-writing
				glDepthMask(GL_FALSE);
			//	glDisable(GL_DEPTH_TEST);
				glBlendFunc(GL_ONE, GL_ONE);

				glColor4f(1.1f, 0.1f, 0.1f, 5.0f);
				glBegin(GL_QUADS);
					glVertex3f(sphere->hitherBottomLeft[0], sphere->hitherBottomLeft[1], sphere->hitherBottomLeft[2]);
					glVertex3f(sphere->hitherBottomRight[0], sphere->hitherBottomRight[1], sphere->hitherBottomRight[2]);
					glVertex3f(sphere->hitherTopRight[0], sphere->hitherTopRight[1], sphere->hitherTopRight[2]);
					glVertex3f(sphere->hitherTopLeft[0], sphere->hitherTopLeft[1], sphere->hitherTopLeft[2]);

				glColor4f(0.1f, 1.1f, 1.1f, 5.0f);
					glVertex3f(sphere->fartherBottomRight[0], sphere->fartherBottomRight[1], sphere->fartherBottomRight[2]);
					glVertex3f(sphere->fartherBottomLeft[0], sphere->fartherBottomLeft[1], sphere->fartherBottomLeft[2]);
					glVertex3f(sphere->fartherTopLeft[0], sphere->fartherTopLeft[1], sphere->fartherTopLeft[2]);
					glVertex3f(sphere->fartherTopRight[0], sphere->fartherTopRight[1], sphere->fartherTopRight[2]);
					glEnd();

				/// Re-enable depth-writing
				glDepthMask(GL_TRUE);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_DEPTH_TEST);

				// Set default shading program
				ShadeMan.SetActiveShader(&graphicsState, "Wireframe");
				continue;

				break;
			}
								  /*
			case ShapeType::CYLINDER: {
				Cylinder * cylinder = (Cylinder*)entity->physics->shape;
				model = ModelMan.GetModel("sphere.obj");
				break;
			}*/
			case ShapeType::SPHERE: {
				// Multiply by entity
				transformationMatrix.Multiply((Matrix4d().Scale(entity->Radius())));
				Sphere * sphere = (Sphere*)entity->physics->shape;
				model = ModelMan.GetModel("Sphere6.obj");
				break;
			}
			case ShapeType::MESH: {// Render the model
				// First render sphere!, then normal mesh.
				// Multiply by entity
				Matrix4f oldTransform = transformationMatrix;
				/*
				transformationMatrix = Matrix4f::Translation(entity->position);
				transformationMatrix.Multiply((Matrix4d().Scale(entity->physics->physicalRadius)));
				transformationMatrix.Multiply(entity->rotationMatrix);
				*/
#ifdef RENDER_MESH_BOUNDING_SPHERE

rerer
				Sphere * sphere = (Sphere*)entity->physics->shape;
				model = ModelMan.GetModel("sphere.obj");
				Matrix4f transform = Matrix4f(transformationMatrix);
				/// Set uniform matrix in shader to point to the AppState modelView matrix.
				glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, transform.getPointer());
				// Render if we got a model ^^
				if (model)
                    model->triangulatedMesh->Render();
#endif

				// Revert transform
				transformationMatrix = oldTransform;

				// Regular mesh rendering:
				PhysicsMesh * pm = entity->physics->physicsMesh;
				ShadeMan.SetActiveShader(&graphicsState, nullptr);

				/// Set matrices
				glMatrixMode(GL_PROJECTION);
				glLoadMatrixf(graphicsState.projectionMatrixF.getPointer());
				glMatrixMode(GL_MODELVIEW);
				Matrix4f modelView = graphicsState.viewMatrixF * transformationMatrix;
				glLoadMatrixf(modelView.getPointer());
				// Disable lighting, enabling pure color-rendering
				glDisable(GL_LIGHTING);

				// Disable Depth-writing
				glDepthMask(GL_FALSE);
#define RENDER_MESH_OCTREES 1
				if (RENDER_MESH_OCTREES && pm->collisionShapeOctree)
				{
					pm->collisionShapeOctree->Render(&graphicsState);
					// Set default shading program
					ShadeMan.SetActiveShader(&graphicsState, "Wireframe");
					// Disable Depth-writing
					glDepthMask(GL_TRUE);
					continue;
				}

				glColor4f(0.1f, 5.1f, 8.1f, 5.0f);
				glBegin(GL_TRIANGLES);
				for (int q = 0; q < pm->triangles.Size(); ++q){
					Triangle * tri = pm->triangles[q];
					glVertex3f(tri->point1[0], tri->point1[1], tri->point1[2]);
					glVertex3f(tri->point2[0], tri->point2[1], tri->point2[2]);
					glVertex3f(tri->point3[0], tri->point3[1], tri->point3[2]);
				}
				glEnd();
				glColor4f(4.1f, 5.1f, 0.1f, 5.0f);
				glBegin(GL_QUADS);
				for (int q = 0; q < pm->quads.Size(); ++q){
					Quad * quad = pm->quads[q];
					glVertex3f(quad->point1[0], quad->point1[1], quad->point1[2]);
					glVertex3f(quad->point2[0], quad->point2[1], quad->point2[2]);
					glVertex3f(quad->point3[0], quad->point3[1], quad->point3[2]);
					glVertex3f(quad->point4[0], quad->point4[1], quad->point4[2]);
				}
				glEnd();

				/// Re-enable depth-writing
				glDepthMask(GL_TRUE);

				// Set default shading program
				ShadeMan.SetActiveShader(&graphicsState, "Wireframe");
				continue;
				break;
			}
		}
		// Always multiply by entity scale, since this is adjustable everywhere in the editor pretty much..!
	//	transformationMatrix.multiply((Matrix4d().scale(entity->scale)));

	//	transformationMatrix.multiply(Matrix4d::GetRotationMatrixX(entity->rotation[0]));
	//	transformationMatrix.multiply(Matrix4d::GetRotationMatrixY(entity->rotation[1]));
	//	transformationMatrix.multiply(Matrix4d::GetRotationMatrixZ(entity->rotation[2]));
		Matrix4f transform = Matrix4f(transformationMatrix);
		/// Set uniform matrix in shader to point to the AppState modelView matrix.
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, transform.getPointer());
		// Render if we got a model ^^
		if (model)
			model->Render(&graphicsState);

	}

	// Render active triangles if wanted toooo
	ShadeMan.SetActiveShader(&graphicsState, nullptr);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(graphicsState.projectionMatrixF.getPointer());
	glMatrixMode(GL_MODELVIEW);
	Matrix4f modelView = graphicsState.viewMatrixF;
	glLoadMatrixf(modelView.getPointer());
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4f(0.1f, 0.1f, 0.1f, 0.1f);
	glBlendFunc(GL_ONE, GL_ONE);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < Physics.activeTriangles.Size(); ++i){
		Triangle triangle = Physics.activeTriangles[i];
		Triangle * tri = &triangle;
		glVertex3f(tri->point1[0], tri->point1[1], tri->point1[2]);
		glVertex3f(tri->point2[0], tri->point2[1], tri->point2[2]);
		glVertex3f(tri->point3[0], tri->point3[1], tri->point3[2]);
	}
	glEnd();


	/// Render OBBs too.
	/// Set default shader program
    ShadeMan.SetActiveShader(&graphicsState, nullptr);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(graphicsState.viewMatrixF.getPointer());
    for (int i = 0; i < physicalEntities.Size(); ++i){
        Entity* e = physicalEntities[i];
        /// Set optional color first.
        // e->physics->obb.Render();
    }

	/// Render collission data. Only render the latest collission if we're paused?
	Collision & c = Physics.lastCollision;
	CollisionResolution & cr = c.cr;
	if (graphicsState.activeViewport->renderCollisions
		&& Physics.IsPaused()
		&& c.one && c.two
		&& c.one->physics && c.two->physics
		)
	{

		glBlendFunc(GL_ALPHA, GL_ALPHA); // GL_ONE_MINUS_SRC_ALPHA
			

		ShadeMan.SetActiveShader(&graphicsState, nullptr);
		Vector3f pos1 = c.one->worldPosition, pos2 = c.two->worldPosition;

		// Render collission point.
		Vector3f origin = c.collissionPoint;
		Vector3f end = origin + c.collisionNormal;
		Vector3f baseColor(0.2f,0.2f,0.2f);
		Vector3f highlightColor(1,1,1);
		RenderFadingLine(origin, end, baseColor, highlightColor);

		/// Render the active lines, yaow.
		glPointSize(4.0f);
		glBegin(GL_POINTS);
		glColor4f(1,0,0,1);
		for (int i = 0; i < c.pointsOne.Size(); ++i){
			Vector3f p = c.pointsOne[i];
			glVertex3f(p[0], p[1], p[2]);
		}
		glColor4f(0,1,0,1);
		for (int i = 0; i < c.pointsTwo.Size(); ++i){
			Vector3f p = c.pointsTwo[i];
			glVertex3f(p[0], p[1], p[2]);
		}
		glEnd();
		glPointSize(1.0f);

		/// Render SATs
		if (ActiveViewport->renderSeparatingAxes){
			glBegin(GL_LINES);
			glColor4f(1,0,1,0.2f);
			for (int i = 0; i < c.separatingAxes.Size(); ++i){
				Vector3f axis = c.separatingAxes[i];
	#define VERTEX(v) glVertex3f(v[0],v[1],v[2]);
				VERTEX(c.collissionPoint);
				VERTEX((c.collissionPoint+axis*5.0f));
			}
			glEnd();
		}
		
		/// Render collission-point velocities.
		Vector4f firstVel(1,1,0,0.7f), secondVel(0,1,1,0.5f), highlight(1,1,1,0.2f);
		RenderFadingLine(c.collissionPoint, c.collissionPoint + c.collissionPointVelocity[0], firstVel, highlight);
		RenderFadingLine(c.collissionPoint, c.collissionPoint + c.collissionPointVelocity[1], secondVel, highlight);

		/// Render previous velocities in a slightly.. darker/alpha'd shade?
		glLineWidth(4.0f);
#define PREVIOUS_VEL		Vector4f(0.45f, 0.3f, 0.1f, 0.5f)
#define PREVIOUS_ANG_VEL	Vector4f(0.2f, 0.1f, 0.4f, 0.5f)
#define PREV_VEL_HIGHLIGHT	Vector4f(0.3f, 0.0f, 0.3f, 0.5f)
#define PREV_ANG_VEL_HIGHLIGHT	Vector4f(0.2f, 0.2f, 0.2f, 0.5f)
		/// Linear
			RenderFadingLine(c.one->worldPosition, c.one->worldPosition + c.one->physics->previousVelocity, PREVIOUS_VEL, PREV_VEL_HIGHLIGHT);
			RenderFadingLine(c.two->worldPosition, c.two->worldPosition + c.two->physics->previousVelocity, PREVIOUS_VEL, PREV_VEL_HIGHLIGHT);
		/// Angular
			RenderFadingLine(c.one->worldPosition, c.one->worldPosition + c.one->physics->previousAngularVelocity, PREVIOUS_ANG_VEL, PREV_ANG_VEL_HIGHLIGHT);
			RenderFadingLine(c.two->worldPosition, c.two->worldPosition + c.two->physics->previousAngularVelocity, PREVIOUS_ANG_VEL, PREV_ANG_VEL_HIGHLIGHT);
			
		glLineWidth(1.0f);

		/// Display further resolvation data.
		if (c.results & RESOLVED){
			Vector4f linearMomentumColorBase(0,0.65f,0,1.0f);
			Vector4f linearMomentumColorHighlight(0,1,0,1.0f);
			RenderFadingLine(pos1, pos1 + cr.deltaLinearMomentum[0]*0.1f, linearMomentumColorBase, linearMomentumColorHighlight);
			RenderFadingLine(pos2, pos2 + cr.deltaLinearMomentum[1]*0.1f, linearMomentumColorBase, linearMomentumColorHighlight);

			Vector3f angularMomentumColorBase(0,0.3f,0.65f);
			Vector3f angularMomentumColorHighlight(0,0.2f,1.0f);
			RenderFadingLine(pos1, pos1 + cr.deltaAngularMomentum[0]*10.f, angularMomentumColorBase, angularMomentumColorHighlight);
			RenderFadingLine(pos2, pos2 + cr.deltaAngularMomentum[1]*10.f, angularMomentumColorBase, angularMomentumColorHighlight);

			/// Render one and two with pre-collission matrices
			Shader * shader = ShadeMan.SetActiveShader(&graphicsState, "Flat");
			glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA); // GL_ONE_MINUS_SRC_ALPHA
			glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, graphicsState.viewMatrixF.getPointer());
			glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, graphicsState.projectionMatrixF.getPointer());
			// Hope matrices are still same.
			Model * model = c.one->model;
			glUniform4f(shader->uniformPrimaryColorVec4, 15.0f, 0.0f, 0.0f, 0.1f);
			glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, cr.onePreResolution.getPointer());
			model->Render(&graphicsState);
			
			model = c.two->model;
			glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, cr.twoPreResolution.getPointer());
			model->Render(&graphicsState);
		}
	}
	else {

	}

	/// Render Entity Velocities/Angular Velocities!
	ShadeMan.SetActiveShader(&graphicsState, nullptr);
	glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA); // GL_ONE_MINUS_SRC_ALPHA
	glLineWidth(4.0f);
	for (int i = 0; i < physicalEntities.Size(); ++i){
		Entity* e = physicalEntities[i];
		PhysicsProperty * p = e->physics;
		if (!p)
			continue;
		RenderFadingLine(e->worldPosition, e->worldPosition + p->velocity, Vector3f(0.2f,0.6f,0.2f), Vector3f(0,0.8f,0));
		RenderFadingLine(e->worldPosition, e->worldPosition + p->angularVelocity, Vector3f(0.2f,0.2f,0.6f), Vector3f(0,0,0.8f));
	}
	glLineWidth(1.0f);

	/// Render the springs plox!
	glLineWidth(2.0f);
	for (int i = 0; i < Physics.springs.Size(); ++i){
		Spring * spring = Physics.springs[i];
		RenderLine(spring->one->worldPosition, spring->two->worldPosition, Vector4f(1,1,1,0.2f));
	}
	glLineWidth(1.0f);

	// Set default shading program
	ShadeMan.SetActiveShader(&graphicsState, "Wireframe");

	// Render the entities now also, muppet
	// Set back to fill!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1.0f);
}


/// Woshie!
void RenderLine(const Vector3f & origin, const Vector3f & endPoint, const Vector4f & baseColor){
		glBegin(GL_LINES);
#define RenderVertex(v); glVertex3f(v[0],v[1],v[2]);
#define SetColor(c); glColor4f(c[0],c[1],c[2],c[3]);
		SetColor(baseColor);
		RenderVertex(origin);
		RenderVertex(endPoint);
		glEnd();
};

void RenderFadingLine(const Vector3f & origin, const Vector3f & endPoint, const Vector4f & baseColor, const Vector4f & highlightColor){

		glBegin(GL_LINES);

		int timeInMS = int(Timer::GetCurrentTimeMs());

		int state = AbsoluteValue(timeInMS % 3000);
	//	std::cout<<"\nState: "<<state;

#define RenderVertex(v); glVertex3f(v[0],v[1],v[2]);
#define SetColor(c); glColor4f(c[0],c[1],c[2],c[3]);

		Vector4f p;
		p = highlightColor;
		int start = 1000;
		float fadeTime = 300.0f;
		float lightTime = 300.0f;
		float fadeOutStart = start + fadeTime + lightTime;
		if (state < start)
			p *= 0;
		else if (state < start + fadeTime)
			p *= (state - start) / fadeTime;
		else if (state < start + fadeTime + lightTime)
			p *= 1.0f;
		else if (state < start + fadeTime*2 + lightTime)
			p *= 1.0f - (state - fadeOutStart) / fadeTime;
		else
			p *= 0;
		p += baseColor;
		SetColor(p);
		RenderVertex(origin);

		/// And then the normal.
		p = highlightColor;
		start = int(start + fadeTime);
		fadeOutStart = start + fadeTime + lightTime;
		if (state < start)
			p *= 0;
		else if (state < start + fadeTime)
			p *= (state - start) / fadeTime;
		else if (state < start + fadeTime + lightTime)
			p *= 1.0f;
		else if (state < start + fadeTime*2 + lightTime)
			p *= 1.0f - (state - fadeOutStart) / fadeTime;
		else
			p *= 0;
		p += baseColor;
		SetColor(p);

		RenderVertex(endPoint);

		glEnd();

		glBegin(GL_POINTS);
			SetColor(highlightColor);
			RenderVertex(origin);
		glEnd();
}
