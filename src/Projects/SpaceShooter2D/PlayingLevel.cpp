/// Emil Hedemalm
/// 2015-02-07
/// All code pertaining to playing of the actual level.

#include "SpaceShooter2D/SpaceShooter2D.h"

void SpaceShooter2D::Cleanup()
{
	/// Remove projectiles which have been passed by.
	for (int i = 0; i < projectileEntities.Size(); ++i)
	{
		Entity * proj = projectileEntities[i];
		ProjectileProperty * pp = (ProjectileProperty*) proj->GetProperty(ProjectileProperty::ID());
		if (pp->sleeping || 
				(proj->position[0] < despawnPositionLeft ||
				proj->position[0] > spawnPositionRight ||
				proj->position[1] < -1.f ||
				proj->position[1] > playingFieldSize[1] + 2.f
				)
			)
		{
			MapMan.DeleteEntity(proj);
			projectileEntities.Remove(proj);
			--i;
		}
	}

	/// Clean ships.
	for (int i = 0; i < level.ships.Size(); ++i)
	{
		Ship * ship = level.ships[i];
		if (ship->destroyed)
			continue;
		if (!ship->spawned)
			continue;
		if (!ship->entity)
			continue;
		// Check if it should de-spawn.
		if (ship->entity->position[0] < despawnPositionLeft)
		{
			ship->Despawn();
		}
	}
}

void SpaceShooter2D::RenderInLevel(GraphicsState * graphicsState)
{
	// Load default shader?
	ShadeMan.SetActiveShader(NULL);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(graphicsState->projectionMatrixD.getPointer());
	glMatrixMode(GL_MODELVIEW);
	Matrix4d modelView = graphicsState->viewMatrixD * graphicsState->modelMatrixD;
	glLoadMatrixd(modelView.getPointer());
	// Enable blending
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float z = -4;
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	graphicsState->currentTexture = NULL;
	// Disable lighting
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	// Ignore previous stuff there.
	glDisable(GL_DEPTH_TEST);
	// Specifies how the red, green, blue and alpha source blending factors are computed
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	Vector2f minField = levelEntity->position - playingFieldHalfSize - Vector2f(1,1);
	Vector2f maxField = levelEntity->position + playingFieldHalfSize + Vector2f(1,1);

	/// o.o
	for (int i = 0; i < shipEntities.Size(); ++i)
	{	
		// Grab the position
		Entity * e = shipEntities[i];
		Vector2f pos = e->position;
		// Check if outside boundary.
		if (pos > minField && pos < maxField)
		{
			continue; // Skip already visible ships.
		}
		// Clamp the position.
		pos.Clamp(minField, maxField);

		// Check direction from this position to the entity's actual position.
		Vector3f to = (e->position - pos);
		float dist = to.Length();
		Vector3f dir = to.NormalizedCopy();
		Vector3f a,b,c;
		// Move the position a bit out...?
		Vector3f center = pos;
		center.z = 7.f;

		// Center.
		a = b = c = center;
		// Move A away from the dir.
		a += dir * 0.7f;
		// Get side-dirs.
		Vector3f side = dir.CrossProduct(Vector3f(0,0,1)).NormalizedCopy();
		side *= 0.5f;
		b += side;
		c -= side;

		// Set color based on distance.
		float alpha = (1.f / dist) + 0.5f;
		glColor4f(1,1,1,alpha);

		// Draw stuff
		glBegin(GL_TRIANGLES);
	#define DRAW(a) glVertex3f(a.x,a.y,a.z)
			DRAW(a);
			DRAW(b);
			DRAW(c);
		glEnd();
	}

	glEnable(GL_DEPTH_TEST);
	CheckGLError("SpaceShooter2D::Render");
}




