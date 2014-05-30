#include "Graphics/GraphicsManager.h"
#include "GraphicsState.h"

int fps;
int currentFrame = 0;
time_t lastSecond;

void GraphicsManager::RenderFPS(){
	// Clear errors upon entering.
	GLuint error = glGetError();

	++currentFrame;
	time_t thisSecond = time(NULL);
	if (thisSecond > lastSecond){
		fps = currentFrame;
		currentFrame = 0;
		lastSecond = thisSecond;
	}

	// Draw awesome grid for debugging, yo.
	if (true){
	    if (Graphics.GL_VERSION_MAJOR >= 2)
            glUseProgram(0);
		// Set projection
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		Matrix4d mat;
		mat.InitOrthoProjectionMatrix();
		glLoadMatrixd(mat.getPointer());
		// Load identity to model matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Enable blending
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		graphicsState.currentTexture = NULL;
		// Disable lighting
		glDisable(GL_LIGHTING);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGLError in Render "<<error;
		}
		glDisable(GL_COLOR_MATERIAL);
		// Specifies how the red, green, blue and alpha source blending factors are computed
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGLError in Render "<<error;
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		float x, y, w, h, z, w2, h2;
		float baseHeight = -0.5f;
		float greenPerTick = 0.03f;
		float redDecrease = 0.03f;
		x = 0.9f;
		w = 0.03f;
		w2 = 0.05f;
		h = 1.0f / 60.0f;
		h2 = 1.0f / 80.0f;
		z = -9;

#define GETY		baseHeight + (1.0f / 60.0f) * i;
#define GETCOLOR	1.0f - i * redDecrease, -1.0f + greenPerTick * i, 0.0167f * i
#define GETCOLOR2	1.1f - i * redDecrease, -0.9f + greenPerTick * i, 0.0167f * i + 0.1f

		int i;
		for (i = 0; i < fps; ++i){
			glColor3f(GETCOLOR);
			y = GETY;
			glBegin(GL_QUADS);
				glVertex3f(x, y, z);
				glVertex3f(x+w, y, z);
				glVertex3f(x+w, y+h, z);
				glVertex3f(x, y+h, z);
			glEnd();
		}

		/// Draw marks at 0, 15, 30 and 60 FPS ^^
		i = 0;
		y = GETY;
		h /= 2.0f;
		glColor3f(1.0f, 0, 0);
		glBegin(GL_QUADS);
			glVertex3f(x, y, z);
			glVertex3f(x+w2, y, z);
			glVertex3f(x+w2, y+h2, z);
			glVertex3f(x, y+h2, z);
		glEnd();

		/// 15 fps
		i = 15;
		y = GETY;
		glColor3f(GETCOLOR2);
		glBegin(GL_QUADS);
			glVertex3f(x, y, z);
			glVertex3f(x+w2, y, z);
			glVertex3f(x+w2, y+h2, z);
			glVertex3f(x, y+h2, z);
		glEnd();

		/// 30 fps
		i = 30;
		y = GETY;
		glColor3f(GETCOLOR2);
		glBegin(GL_QUADS);
			glVertex3f(x, y, z);
			glVertex3f(x+w2, y, z);
			glVertex3f(x+w2, y+h2, z);
			glVertex3f(x, y+h2, z);
		glEnd();

		/// 45 fps
		i = 45;
		y = GETY;
		glColor3f(GETCOLOR2);
		glBegin(GL_QUADS);
		glVertex3f(x, y, z);
		glVertex3f(x+w2, y, z);
		glVertex3f(x+w2, y+h2, z);
		glVertex3f(x, y+h2, z);
		glEnd();

		/// 60 fps
		i = 60;
		y = GETY;
		glColor3f(GETCOLOR2);
		glBegin(GL_QUADS);
			glVertex3f(x, y, z);
			glVertex3f(x+w2, y, z);
			glVertex3f(x+w2, y+h2, z);
			glVertex3f(x, y+h2, z);
		glEnd();

		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGLError in Render "<<error;
		}
	}
}
