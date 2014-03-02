// Emil Hedemalm
// 2013-06-23

#include "Graphics/GraphicsManager.h"
#include "Lighting.h"
#include "ModelManager.h"
#include "GraphicsState.h"

/// Do render lights plz

/// bölöööö

/// Renders 3D-representations of all light-sources.
void GraphicsManager::RenderLights(){

	// Optionally render the global-light as a small... tile?
	// TODO: if so.

	SetShaderProgram(0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(graphicsState->projectionMatrixF.getPointer());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(graphicsState->viewMatrixF.getPointer());

	Lighting * lighting = graphicsState->lighting;

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
//	glBindTexture(GL_TEXTURE_2D, 0);
//	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);

	/// Render all lights provided in the shiberiwa!
	for (int i = 0; i < lighting->ActiveLights(); ++i){
		// Reset view/proj matrices
		glLoadIdentity();
		glLoadMatrixf(graphicsState->viewMatrixF.getPointer());

		Light * light = lighting->GetLight(i);
		Vector3f position = light->position;
		Vector3f destination;
		glColor4f(light->diffuse.x,light->diffuse.y,light->diffuse.z,1);
		switch(light->type){
			case LightType::POINT: {
				// Render a sphere, maybe a max/min thingy?
				glBegin(GL_POINTS);
					glVertex3f(position.x, position.y, position.z);
				glEnd();
				glTranslatef(position.x, position.y, position.z);
			//	graphicsState->modelMatrixF = Matrix4f::Translation(position);
				Model * model = ModelMan.GetModel("Sphere6");
				if (model)
					model->mesh->Render(*graphicsState);
				else
					std::cout<<"\nUnable to render PointLight!";
				break;
			}
			case LightType::DIRECTIONAL:
				/// Render a sphere as well as some parallell lines?
				glBegin(GL_POINTS);
					glVertex3f(0,0,0);
				glEnd();
				destination -= position * 100000;
				glBegin(GL_LINES);
					glVertex3f(-destination.x, -destination.y, -destination.z);
					glVertex3f(0,0,0);
					glVertex3f(0,0,0);
					glVertex3f(destination.x, destination.y, destination.z);
				glEnd();
				break;
			case LightType::SPOTLIGHT:
				/// Render a sphere as well as a cone?
				glBegin(GL_POINTS);
					glVertex3f(position.x, position.y, position.z);
				glEnd();
				destination = position + light->spotDirection * 100000;
				glBegin(GL_LINES);
					glVertex3f(position.x, position.y, position.z);
					glVertex3f(destination.x, destination.y, destination.z);
				glEnd();
				break;
			default:
				assert(false && "Unsupported light-type in GraphicsManager::RenderLights!");
				break;
		}
	}
}
