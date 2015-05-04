/// Emil Hedemalm
/// 2015-02-20
/// Separation into files, as it has started confusing me.
/// All functions related to setting up a shadow-mapping pre-pass.

#include "Render/RenderPass.h"
#include "Lighting.h"

#include "Physics/PhysicsManager.h"
#include "PhysicsLib/Shapes/AABB.h"

#include "Graphics/GraphicsProperty.h"

bool RenderPass::SetupLightPOVCamera()
{
	// Now it becomes tricky..
	static Camera * camera = NULL;
	bool anyShadows = false;
	if (camera == NULL)
		camera = CameraMan.NewCamera("LightPOVCamera");

	Lighting * lighting = graphicsState->lighting;
	List<Light*> lights = lighting->GetLights();
	for (int i = 0; i < lights.Size(); ++i)
	{
		Light * light = lights[i];
		if (!light->castsShadow)
			continue;
		if (light->type != LightType::DIRECTIONAL)
		{
			std::cout<<"\nLight types beside directional not supported for shadow mappinag at the moment.";
			continue;
		}
		anyShadows = true;
		
		Vector3f lightPosition = light->position;
		Vector3f lpn = lightPosition.NormalizedCopy();
		// Grab AABB of all relevant entities? Check the AABB-sweeper or other relevant handler?
		AABB allEntitiesAABB = PhysicsMan.GetAllEntitiesAABB();
		float mDist = allEntitiesAABB.max.Length();
		float m2Dist = allEntitiesAABB.min.Length();
		float maxDist = MaximumFloat(mDist, m2Dist);

		// Distance + some buffer.
		float farPlane = light->shadowMapFarplane;
		// Zoom will vary with how wide/long the area is which has to be visible/shadowed.
		float zoom = light->shadowMapZoom;
		// Adjust light position based on the gathered data above.
//		lightPosition = lpn * maxDist;
		
//		LogGraphics("Setting sun: farplane "+String(farPlane)+" zoom: "+String(zoom)+" Position: "+VectorString(lightPosition), INFO);
		
		camera->projectionType = Camera::ORTHOGONAL;
		camera->position = lightPosition;
		
		// Set rotation based on position?
		Vector2f xz(lpn.x, lpn.z);
		float xzLen = xz.Length();
		xz.Normalize();
		Angle yaw = Angle(xz);
		yaw -= Angle(PI/2);
//		std::cout<<"Yaw: "<<yaw.Degrees();
		Angle pitch = Angle(xzLen, lpn.y);
		camera->rotation.y = yaw.Radians();
		camera->rotation.x = pitch.Radians();
		camera->zoom = zoom;
		camera->SetRatioF(1,1);
		camera->farPlane = farPlane;
		camera->Update();
		Vector3f forward = camera->LookingAt();
		Vector3f up = camera->UpVector();
		Matrix4f viewProjection = camera->ViewProjectionF();
		Matrix4f viewProj2 = camera->ViewMatrix4f() * camera->ProjectionMatrix4f();
		Vector3f pos3 = viewProj2 * Vector4f(0,0,0,1);
		Vector3f position = viewProjection * Vector4f(0,0,0,1),
			position2 = viewProjection * Vector4f(10,0,0,1),
			position3 = viewProjection * Vector4f(0,10,0,1),
			position4 = viewProjection * Vector4f(0,0,10,1);
		graphicsState->SetCamera(camera);
		light->shadowMapIndex = 0;
		float elements [16] = {	0.5, 0, 0, 0,
								0, 0.5, 0, 0,
								0, 0, 0.5, 0,
								0.5, 0.5, 0.5, 1};
		Matrix4f biasMatrix(elements);
		Vector3f vec = biasMatrix * Vector3f(0,0,0);
		Vector3f vec2 = biasMatrix * Vector3f(0.5f, 0.5f, 0.5f);
		Matrix4f shadowMappingMatrix = biasMatrix * camera->ViewProjectionF();
		Vector3f shadowSpace = shadowMappingMatrix * Vector3f(0,0,0);
		Vector3f s2 = shadowMappingMatrix * Vector3f(10,0,0),
			s3 = shadowMappingMatrix * Vector3f(0,10,0),
			s4 = shadowMappingMatrix * Vector3f(0,0,10);

		/// Set up a viewport similar to the shadow-map texture we are going to use!
		glViewport(0, 0, shadowMapResolution, shadowMapResolution);
		glDisable(GL_SCISSOR_TEST);

		light->shadowMappingMatrix = shadowMappingMatrix;
		// Take current shadow map texture we created earlier and make sure the camera is bound to it for usage later.
		// Save matrix used to render shadows properly later on?
//			light->inverseTransposeMatrix = ;
		light->shadowMap = graphicsState->activeViewport->shadowMapDepthBuffer->renderBuffers[0]->texture;
		assert(light->shadowMap);
		return true;
	}
	return false;
}

/// Creates it as needed.
bool RenderPass::BindShadowMapFrameBuffer()
{
	CheckGLError("Before RenderPass::BindShadowMapFrameBuffer");
	if (!viewport->shadowMapDepthBuffer)
	{
		viewport->shadowMapDepthBuffer = new FrameBuffer("ShadowMapDepthBuffer");
	}
	if (!viewport->shadowMapDepthBuffer->IsGood() || viewport->shadowMapDepthBuffer->size != (Vector2i(1,1) * shadowMapResolution))
	{
		// Try and rebuild it..?
		if (!viewport->shadowMapDepthBuffer->CreateDepthBuffer(Vector2i(1, 1) * shadowMapResolution))
		{
			SAFE_DELETE(viewport->shadowMapDepthBuffer);
			return false;
		}
	}
	int error = glGetError();
	/// Make frame buffer active
	viewport->shadowMapDepthBuffer->Bind();
	// Set viewport size to clear?
	glViewport(0, 0, viewport->shadowMapDepthBuffer->size.x, viewport->shadowMapDepthBuffer->size.y);
	// Set buffers to render into (the textures ^^)
	viewport->shadowMapDepthBuffer->SetDrawBuffers();
	CheckGLError("RenderPass::BindShadowMapFrameBuffer");
	return true;
}


/// Used for e.g. shadow-mapping.
void RenderPass::RenderEntitiesOnlyVertices()
{
	bool optimized = true;
	if (!optimized)
	{
		RenderEntities();
		return;
	}
	Entity * entity;
	GraphicsProperty * gp;
	Timer timer;
	timer.Start();
	/// Instancing enabled?
	if (instancingEnabled)
	{	
		// Turn it off first, though.
		glUniform1i(shader->uniformInstancingEnabled, 0);
	}
	/// Bind default array.
	graphicsState->BindVertexArrayBuffer(0);
	// Render all entities listed in the graphicsState!
	for (int i = 0; i < entitiesToRender.Size(); ++i)
	{
		entity = entitiesToRender[i];
		// Just load transform as model matrix straight away.
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, entity->transformationMatrix.getPointer());	
		// Render the model
		entity->model->Render();
		++graphicsState->renderedObjects;		// increment rendered objects for debug info
	}
	if (instancingEnabled)
	{
		glUniform1i(shader->uniformInstancingEnabled, 1);
		for (int i = 0; i < entityGroupsToRender.Size(); ++i)
		{
			RenderInstancingGroup * group = entityGroupsToRender[i];
			group->Render();
		}
	}
	timer.Stop();
	FrameStats.renderEntities += timer.GetMs();	
}
