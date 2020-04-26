//
//
//
//#include "LightSource.h"
//#include "GraphicsState.h"
//
//LightSource::LightSource() : Node() {
//	ambient[0] = 0.0;
//	ambient[1] = 0.0;
//	ambient[2] = 0.0;
//	ambient[3] = 1;
//	diffuse[0] = 1.0;
//	diffuse[1] = 1.0;
//	diffuse[2] = 1.0;
//	diffuse[3] = 1;
//	specular[0] = 1.0;
//	specular[1] = 1.0;
//	specular[2] = 1.0;
//	specular[3] = 1;
//
//	position[0] = position[2] = 0;
//	position[1] = 2.0;
//
//	type = 1;
//	
//	constantAttenuation = 1.0;
//	linearAttenuation = 0.0;
//	quadraticAttenuation = 0.0;
//}
//
//void LightSource::Render(){
//	checkGLError();
//	// Disabled for now
//	throw 3;
///*
//	// Enable lighting
//	glEnable(GL_LIGHTING);
//
//	// Enable lights lights
//	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
//	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
//	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
//	glLightfv(GL_LIGHT1, GL_POSITION, position);
//
//	glEnable(GL_LIGHT1);
//
//	// Render children with the light
//	Node::renderChildren(state);
//	
//	// Remove the lights and disable lighting
//	glDisable(GL_LIGHTING);
//
//	// If enabled, render a point cross at the lighting center	
//	if (GraphicsThreadGraphicsState.settings & RENDER_LIGHT_POSITION){
//		glLoadMatrixd(GraphicsThreadGraphicsState.modelMatrixD.getPointer());
//		glBegin(GL_POINTS);
//		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//			glVertex3f(position[0], position[1], position[2]);
//		glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
//			glVertex3f(position[0]+1.0f, position[1], position[2]);
//			glVertex3f(position[0]-1.0f, position[1], position[2]);
//			glVertex3f(position[0], position[1]+1.0f, position[2]);
//			glVertex3f(position[0], position[1]-1.0f, position[2]);
//			glVertex3f(position[0], position[1], position[2]+1.0f);
//			glVertex3f(position[0], position[1], position[2]-1.0f);
//		glEnd();
//	}
//	// Render siblings.
//	Node::renderSibling(state);
//	*/
//}
//
//void LightSource::renderPosition(GraphicsState &state){
//	checkGLError();
//	// Disabled for now
//	throw 3;
///*	// Render a point cross at the lighting center	
//	glLoadMatrixd(GraphicsThreadGraphicsState.modelMatrixD.getPointer());
//	glBegin(GL_POINTS);
//	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//		glVertex3f(position[0], position[1], position[2]);
//	glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
//		glVertex3f(position[0]+1.0f, position[1], position[2]);
//		glVertex3f(position[0]-1.0f, position[1], position[2]);
//		glVertex3f(position[0], position[1]+1.0f, position[2]);
//		glVertex3f(position[0], position[1]-1.0f, position[2]);
//		glVertex3f(position[0], position[1], position[2]+1.0f);
//		glVertex3f(position[0], position[1], position[2]-1.0f);
//	glEnd();
//	*/
//}