#include "Material.h"

Material::Material(){
	ambient[0] = 0.5f;
	ambient[1] = 0.5f;
	ambient[2] = 0.5f;
	ambient[3] = 1;
	diffuse[0] = 0.5;
	diffuse[1] = 0.5f;
	diffuse[2] = 0.5f;
	diffuse[3] = 1;
	specular[0] = 1.0f;
	specular[1] = 1.0f;
	specular[2] = 1.0f;
	specular[3] = 1;
	shininess = 50.0f;
	emission[0] = 0.0f;
	emission[1] = 0.0f;
	emission[2] = 0.0f;
	emission[3] = 1.0f;
}

void Material::justAmbient(){
	ambient[0] = 1.0f;
	ambient[1] = 1.0f;
	ambient[2] = 1.0f;
	ambient[3] = 1;
	diffuse[0] = 0.0f;
	diffuse[1] = 0.0f;
	diffuse[2] = 0.0f;
	diffuse[3] = 1;
	specular[0] = 0.0f;
	specular[1] = 0.0f;
	specular[2] = 0.0f;
	specular[3] = 1;
	shininess = 0.0f;
	emission[0] = 0.0f;
	emission[1] = 0.0f;
	emission[2] = 0.0f;
	emission[3] = 1.0f;
}