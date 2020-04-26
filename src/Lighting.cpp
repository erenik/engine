#include "Lighting.h"

#include <Graphics/OpenGL.h>

#include <iostream>
#include <ctime>
#include <cstring>
#include <fstream>

#include "Graphics/Shader.h"
#include "GraphicsState.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GraphicsMessages.h"

#include "Window/AppWindowManager.h"

#include "UI/UserInterface.h"
#include "UI/UIButtons.h"

#include "Message/MessageManager.h"
#include "Message/MathMessage.h"
#include "Message/FileEvent.h"

#include "Graphics/Camera/Camera.h"

AppWindow * Lighting::lightingEditor = NULL;

String lightList = "LightList";

#define UPDATE_LAST_UPDATE_TIME this->lastUpdate = Time::Now();


/// Default constructor, sets all light pointers and other variables to 0/NULL
Lighting::Lighting()
{
	global_ambient = Vector4f(1.0f, 1.0f, 1.0f, 0.1f);
	this->activeLight = NULL;
	this->activeLightIndex = 0;
	this->lightCounter = 0;
	lastPreparationFrame = -1;
	UPDATE_LAST_UPDATE_TIME
}
Lighting::~Lighting()
{
}

bool Lighting::SaveLighting(String toFileName)
{
	if (!toFileName.Contains(".lighting"))
		toFileName.Add(".lighting");
	std::fstream file;
	file.open(toFileName.c_str(), std::ios_base::out | std::ios_base::binary);
	if (!file.is_open())
		return false;
	this->WriteTo(file);
	file.close();
	return true;
}

bool Lighting::LoadLighting(String fromFileName)
{
	if (activeLight)
		activeLight->CloseEditorWindow();
	std::fstream file;
	file.open(fromFileName.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
		return false;
	this->ReadFrom(file);
	file.close();
	// Stuff.
	this->OpenEditorWindow();
	// Make it active.
//	Graphics.QueueMessage(new GMSetLighting(this));
	UPDATE_LAST_UPDATE_TIME
}

/// Returns true if the message had any meaning, adjusting values within the lighting.
bool Lighting::ProcessMessage(Message * message)
{
	String msg = message->msg;
	// See if it's a message that could apply to our lights.
	switch(message->type)
	{
		case MessageType::FILE_EVENT:
		{
			FileEvent * fileEvent = (FileEvent*) message;
			String file = fileEvent->files[0];
			if (msg == "LoadLighting")
			{
				LoadLighting(file);
			}
			else if (msg == "SaveLighting")
			{
				SaveLighting(file);
			}
			break;
		}
		case MessageType::VECTOR_MESSAGE:
		{
			VectorMessage * vm = (VectorMessage*) message;
			if (msg == "SetGlobalAmbient")
			{
				global_ambient = vm->GetVector4f();
			}
		}
		// General basic string messages sent by all UI buttons and internal messaging
		case MessageType::STRING: 
		{
			if (msg == "OnReloadUI")
			{
				if (lightingEditor)
				{	
					UserInterface * ui = lightingEditor->ui;
					Graphics.QueueMessage(new GMSetUIv3f("GlobalAmbient", GMUI::VECTOR_INPUT, GetAmbient(), ui));
					this->UpdateLightList();
				}
				if (activeLight)
					activeLight->OnPropertiesUpdated();
			}
			else if (msg == "SaveLighting")
			{
				MesMan.QueueMessages("OpenFileBrowser(Save Lighting,SaveLighting,.lighting)");
			}
			else if (msg == "LoadLighting")
			{
				// If arguments exist, load it.
				if (msg.Contains("("))
				{
					String arg = msg.Tokenize("()")[1];
					if (arg.Length() == 0)
						arg = "temp.lighting";
					LoadLighting(arg);
				}
				// Else open a file-browser for it.
				else 
				{
					MesMan.QueueMessages("OpenFileBrowser(Load Lighting,LoadLighting,.lighting)");
				}
			}
			else if (msg == "ClearLights")
			{
				if (activeLight)
					activeLight->CloseEditorWindow();
				this->DeleteAllLights();
				UpdateLightList();
			}
			else if (msg == "DisableLight")
			{
				if (activeLight)
				{
					activeLight->currentlyActive = !activeLight->currentlyActive;
				}
			}
			else if (msg.Contains("NewLight"))
			{
				String name = "Light";
				if (msg.Contains(":"))
					name = msg.Tokenize(":")[1];
				Light * light = NewLight(name);
				// Open up the editor for it.
				light->OpenEditorWindow();
			}
			else if (msg.Contains("SelectLightByIndex:"))
			{
				String indexStr = msg.Tokenize(":")[1];
				int index = indexStr.ParseInt();
				Light * selectedLight = SelectLightByIndex(index);
			}
			else if (msg.Contains("OpenLightingEditor"))
			{
				this->OpenEditorWindow();
			}
			else if (msg.Contains("OpenLightEditor"))
			{
				if (!activeLight)
					return false;
				activeLight->OpenEditorWindow();
			}
			else if (msg == "UpdateLightList")
				UpdateLightList();
			else if (msg == "DeleteActiveLight")
			{
				lights.Remove(activeLight);
				activeLight->CloseEditorWindow();
				activeLight = NULL;
				UpdateLightList();
				
			}
			
			
		}
	}
	return false;
}


/// Creates an editor-AppWindow for this lighting, assuming the existance of LightingMenu and LightEditor GUI files.
AppWindow * Lighting::OpenEditorWindow()
{
	// Look for an existing lighting-editor AppWindow.
	if (lightingEditor)
	{
	
	}
	// Create it if not existing.
	else {
		lightingEditor = WindowMan.NewWindow("LightingEditor", "Lighting editor");
		UserInterface * ui = lightingEditor->CreateUI();
		ui->Load("gui/LightingMenu.gui");
		lightingEditor->SetRequestedSize(Vector2i(400, 320));
		lightingEditor->DisableAllRenders();
		lightingEditor->renderUI = true;
		lightingEditor->CreateGlobalUI();
		lightingEditor->Create();
	}
	
	// Show it.
	lightingEditor->Show();
	// Bring it to the top if needed.
	lightingEditor->BringToTop();

	
	// Set default values as specified in this lighting.
	UserInterface * ui = lightingEditor->ui;

	Graphics.QueueMessage(new GMSetUIv3f("GlobalAmbient", GMUI::VECTOR_INPUT, GetAmbient(), ui));
	UpdateLightList(lightingEditor);
	return lightingEditor;
}

/// Updates UI for all lights in this lighting.
void Lighting::UpdateLightList(AppWindow * inWindow)
{
	AppWindow * window = inWindow;
	if (!window)
		window = lightingEditor;
	if (!window)
		return;
	// Set default values as specified in this lighting.
	UserInterface * ui = window->ui;
	Graphics.QueueMessage(new GMClearUI(lightList, ui));
	// Add all lights.
	for (int i = 0; i < this->lights.Size(); ++i)
	{
		Light * light = lights[i];
		// Add an entry for it in the list.
		UIButton * button = new UIButton(light->name);
		button->sizeRatioY = 0.2f;
		button->activationMessage = "SelectLightByIndex:"+String::ToString(i)+"&&OpenLightEditor";
		Graphics.QueueMessage(new GMAddUI(button, lightList, ui));
	}
}

/// Creates a new light to this setup.
Light * Lighting::NewLight(String name)
{
	Light * light = new Light(this);
	light->name = name;
	activeLight = light;
	lastUpdate = Timer::GetCurrentTimeMs();
	lights.Add(light);
	// Update UI if needed.
	UpdateLightList();
	return light;
}

/// Adds light to the lighting, return NULL upon falure. 
Light * Lighting::Add(Light * newLight)
{
	newLight->currentlyActive = true;
	lights.AddItem(newLight);
	UPDATE_LAST_UPDATE_TIME
	return newLight;
}

/// Removes target light. Returns false on failure.
bool Lighting::Remove(Light * toRemove)
{
	UPDATE_LAST_UPDATE_TIME
	return lights.Remove(toRemove);
}

/// Creates a default setup of 3-4 lights for testing purposes.
void Lighting::CreateDefaultSetup()
{
/*
	/// Ambient clear blue sky
	SetAmbient(64/500.0f, 156/500.0f, 255/500.0f);
	// First general sunlight
	CreateLight();
	SetType(LightType::DIRECTIONAL);
//	SetDiffuse(255/255.0f, 255/255.0f, 251/255.0f);
//	SetSpecular(255/255.0f, 255/255.0f, 251/255.0f);
	SetColor(255/255.0f, 255/255.0f, 251/255.0f);
	SetPosition(0,1.0f, 0.2f);
	SetName("Sun light");

	// Then.. a light bulb o-o something
	CreateLight();
	SetType(LightType::POSITIONAL);
	SetPosition(20.0f, 15.0f, 10.2f);
	SetColor(0.5f, 1.0f, 1.0f);
//	SetDiffuse(0.5f, 1.0f, 1.0f);
//	SetSpecular(0.5f, 1.0f, 1.0f);
	SetAttenuation(1.0f, 0.0005f, 0.0f);
	SetName("Light bulb.. something");

	// Then.. a light bulb o-o something
	CreateLight();
	SetType(LightType::POSITIONAL);
	SetPosition(-20.0f, 35.0f, 10.2f);
	SetColor(1.5f, 0.5f, 0.5f);
//	SetDiffuse(1.5f, 1.0f, 1.0f);
//	SetSpecular(1.5f, 1.0f, 1.0f);
	SetAttenuation(1.0f, 0.005f, 0.00001f);
	SetName("Light bulb.. something2");

	// Then.. a light bulb o-o something
	CreateLight();
	SetType(LightType::SPOTLIGHT);
	SetPosition(-10.0f, 5.0f, -50.2f);
	SetColor(0.3f, 0.3f, 3.0f);
//	SetDiffuse(1.5f, 1.0f, 1.0f);
//	SetSpecular(1.0f, 1.0f, 3.0f);
	SetSpotDirection(0.02f,-0.001f,1.0f);
	SetSpotCutoffExponent(45.0f, 5);
	SetAttenuation(1.0f, 0.000f, 0.000f);
	SetName("Spot light.. o-o");

	std::cout<<"\nDefault lights created.";
	*/
}
/// Returns all lights in their current state.
List<Light*> Lighting::GetLights() const
{
	List<Light*> constLightList;
	for (int i = 0; i < lights.Size(); ++i)
	{
		constLightList.Add(lights[i]);
	}
	return constLightList;
}

int Lighting::NumLights() const 
{
	return lights.Size();
}


/// Fills the whole light-array, attempting to  test the limits of the hardware with as many lights as possible.
void Lighting::Overload()
{
	/*
	for (int i = 0; i < MAX_LIGHTS; ++i){
		int index = CreateLight();
		if (index == -1){
			std::cout<<"\nERROR: Unable to create further lights. Maximum lights reached!";
			return;
		}
		float r, g, b;
		r = rand()%100 / 1.0f;
		g = rand()%100 / 1.0f;
		b = rand()%100 / 1.0f;
		SetDiffuse(r,g,b);
		SetSpecular(r,g,b);
		SetPosition((float)(rand()%2200 - 1100) ,(float)(rand()%2200 - 1100), (float)(rand()%2200 - 1100));
		SetAttenuation(1.0f, 0.1f, 0.001f);
		SetType(LightType::POSITIONAL);
	}
	*/
}

/// Sets ambient using 0-255 values
void Lighting::SetAmbient(int r, int g, int b, int a)
{
	this->global_ambient = Vector4f(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
	UPDATE_LAST_UPDATE_TIME
}
/// Sets ambient using floats.
void Lighting::SetAmbient(float r, float g, float b, float a)
{
	this->global_ambient = Vector4f(r,g,b,a);
	this->lastUpdate = Time::Now();
}
/// Sets ambient using doubles.
void Lighting::SetAmbient(double r, double g, double b, double a) {
	SetAmbient(float(r), float(g), float(b), float(a));
}


/// Sets ambient values. Alpha defaults to 0.
void Lighting::SetAmbient(const Vector3f & values)
{
	this->global_ambient = Vector4f(values);
	this->lastUpdate = Time::Now();
	if (debug == 13)
		std::cout<<"\nLighting::SetAmbient: "<<global_ambient<<" lastupdate: "<<lastUpdate.Milliseconds();
}

/// Creates a new light source, adding it to the array. Returns the newly created light's index and sets it as the currently editable light.
Light * Lighting::CreateLight()
{
	if (lights.Size() >= MAX_LIGHTS)
		return NULL;
	activeLight = new Light(this);
	/// Set default values
	activeLight->diffuse = Vector4f(1,1,1,1);
	activeLight->specular = Vector4f(1,1,1,1);
	activeLight->attenuation = Vector3f(1.0f,0.0f,0.0f);
	// Set name
	String name = "Light " + String::ToString(lightCounter);
	++this->lightCounter;
	activeLight->SetName(name);
	UPDATE_LAST_UPDATE_TIME
	lights.Add(activeLight);
	return activeLight;
}


/// Selects and makes active.
Light * Lighting::SelectLightByIndex(int index)
{
	if (index > lights.Size() || index < 0)
		return NULL;
	activeLight = lights[index];
	return activeLight;
}

/// Selects and makes active target light. May return NULL.
Light * Lighting::SelectLightByName(String byName)
{
	for (int i = 0; i < lights.Size(); ++i)
	{
		Light * light = lights[i];
		if (light->name == byName)
		{
			light->currentlyActive = true;
			activeLight = light;
			return light;
		}
	}
	return NULL;
}

/// Returns a pointer to selected light. USE WITH CAUTION.
Light * Lighting::GetLight(int index){
	assert(index >= 0 && "Invalid light-index!");
	return lights[index];
}

/// Deletes active light source or target light if argument is provided. Returns false if no light is selected or hte light did not belong to this lighting.
bool Lighting::DeleteLight(Light * light)
{	
	return lights.Remove(light);
}

/// Deletes all light sources contained within.
int Lighting::DeleteAllLights()
{
	int lightsDeleted = lights.Size();
	lights.ClearAndDelete();
	UPDATE_LAST_UPDATE_TIME
	return lightsDeleted;
}


/// Loads from target file, calling ReadFrom once a valid stream has been opened.
bool Lighting::LoadFrom(String fileName)
{
	if (!fileName.Contains(".lighting"))
		fileName += ".lighting";
	std::fstream file;
	file.open(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
		return false;
	ReadFrom(file);
	file.close();
	// Update UI if needed.
	UpdateLightList();
	UPDATE_LAST_UPDATE_TIME
	return true;
}


// Versions
#define LIGHTING_VERSION_0 0// Initial version.
int lightingVersion = LIGHTING_VERSION_0;

/// Writes to file stream.
void Lighting::WriteTo(std::fstream & file){
	// Write version
	file.write((char*)&lightingVersion, sizeof(int));
	// Write ambient
	global_ambient.WriteTo(file);
	// Write number of lights
	int numLights = lights.Size();
	file.write((char*)&numLights, sizeof(int));
	for (int i = 0; i < lights.Size(); ++i){
		lights[i]->WriteTo(file);
	}
}
/// Reads from file stream.
void Lighting::ReadFrom(std::fstream & file)
{
	// Delete old
	lights.ClearAndDelete();
	activeLight = NULL;
	// Write version
	int version;
	file.read((char*)&version, sizeof(int));
	assert(version == lightingVersion);
	// Write ambient
	global_ambient.ReadFrom(file);
	// Write number of lights
	int numLights = 0;
	file.read((char*)&numLights, sizeof(int));
	for (int i = 0; i < numLights; ++i){
		Light * light = new Light(this);
		light->ReadFrom(file);
		lights.Add(light);
	}
}

/// Fills the big arrays with data from the individual lights.
void Lighting::PrepareForLoading()
{
	activeLights = 0;
	ambient[0] = global_ambient[0];
	ambient[1] = global_ambient[1];
	ambient[2] = global_ambient[2];
	ambient[3] = global_ambient[3];
	assert(GraphicsThreadGraphicsState->camera);
	Vector3f camPos = GraphicsThreadGraphicsState->camera->position;
	for (int i = 0; i < lights.Size() && activeLights < MAX_LIGHTS; ++i)
	{
		Light * light = lights[i];
		if (!light->currentlyActive)
			continue;
		if (light->attenuation.x <= 0)
			continue;

		float dist = (camPos - light->position).Length();
		if (dist > 50 && light->type != LightType::DIRECTIONAL)
			continue;

		// Only take those lights which are visible in the frustum, or will reach it with their light?
		// ... 
		int interval4 = activeLights * 4;
		int interval3 = activeLights * 3;
		int interval = 4;
		diffuse[interval4] = light->diffuse[0];
		diffuse[interval4+1] = light->diffuse[1];
		diffuse[interval4+2] = light->diffuse[2];
		diffuse[interval4+3] = light->diffuse[3];

		specular[interval4] = light->specular[0];
		specular[interval4+1] = light->specular[1];
		specular[interval4+2] = light->specular[2];
		specular[interval4+3] = light->specular[3];

		interval = 3;
		position[interval3] = light->position[0];
		position[interval3+1] = light->position[1];
		position[interval3+2] = light->position[2];

		attenuation[interval3] = light->attenuation[0];
		attenuation[interval3+1] = light->attenuation[1];
		attenuation[interval3+2] = light->attenuation[2];
		if (debug == 15)
			std::cout<<"\nLight attenuation "<<activeLights<<" :"<<light->attenuation;

		castsShadows[activeLights] = light->shadowMapIndex;

		type[activeLights] = light->type;

		spotDirection[interval3] = light->spotDirection[0];
		spotDirection[interval3+1] = light->spotDirection[1];
		spotDirection[interval3+2] = light->spotDirection[2];
		/// Calcualte cutoff as a cosine value of the degrees converted to radians before we throw it in ^^
		spotCutoff[activeLights] = light->spotCutoff;
		spotExponent[activeLights] = light->spotExponent;

		activeLights++;
	}
	if (debug == 4)
		std::cout<<"\nActive lights: "<<activeLights;
}

/// Loads selected lighting into the active shader program
void Lighting::LoadIntoShader(Shader * shader)
{
	GLuint loc = -1, error;
	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::cout<<"\nError before LoadLighting";
	}
	if (!shader)
		return;
	if (lastUpdate == shader->lastLightUpdate)
		return;
	shader->lastLightUpdate = lastUpdate;
//#define PRINT_DEBUG

	// Return. If no ambient is present no other light will be either.
	if (shader->uniformLight.ambientVec4 == -1)
		return;

	CheckGLError("LoadLighting before gl calls.");

	/// Only update once per frame, as needed.
	if (graphicsFrameNumber != lastPreparationFrame)
	{
		PrepareForLoading();
		/// Update.
		lastPreparationFrame = graphicsFrameNumber;
	}

	glUniform4fv(shader->uniformLight.ambientVec4, 1, ambient);
	CheckGLError("Error setting global ambient luminosity");
	
	// If no diffuse uniform, abort.
	if (shader->uniformLight.diffuseVec4 == -1)
		return;
	/// Set Diffuse
	if (shader->uniformLight.diffuseVec4 != -1)
	{
		glUniform4fv(shader->uniformLight.diffuseVec4, activeLights, diffuse);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting light diffuse properties";
		}
	}/// Set Specular
	if (shader->uniformLight.specularVec4 != -1){
		glUniform4fv(shader->uniformLight.specularVec4, activeLights, specular);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting light specular properties";
		}
	}
	/// Set Position
	if (shader->uniformLight.positionVec3 != -1){
		glUniform3fv(shader->uniformLight.positionVec3, activeLights, position);
		error = glGetError();
		if (error != GL_NO_ERROR){
			// std::cout<<"\nGL_ERROR: Error setting light position properties";
		}
	}
	/// Set attenuation
	if (shader->uniformLight.attenuationVec3 != -1){
		glUniform3fv(shader->uniformLight.attenuationVec3, activeLights, attenuation);
		CheckGLError("Attenuation");
	}
	if (shader->uniformLight.castsShadowsBool != -1)
	{
		glUniform1iv(shader->uniformLight.castsShadowsBool, activeLights, castsShadows);
		CheckGLError("CastsShadows");
	}
	/// Set Type
	if (shader->uniformLight.typeInt != -1){
		glUniform1iv(shader->uniformLight.typeInt, activeLights, type);
		error = glGetError();
		if (error != GL_NO_ERROR){
			//std::cout<<"\nGL_ERROR: Error setting light type";
		}
	}
	/// Set spotlight attributes
	if (shader->uniformLight.spotDirectionVec3 != -1){
		glUniform3fv(shader->uniformLight.spotDirectionVec3, activeLights, spotDirection);
	}
	if (shader->uniformLight.spotCutoffFloat != -1){
		glUniform1fv(shader->uniformLight.spotCutoffFloat, activeLights, spotCutoff);
	}
	if (shader->uniformLight.spotExponentInt != -1){
		glUniform1iv(shader->uniformLight.spotExponentInt, activeLights, spotExponent);
	}
	error = glGetError();
	if (error != GL_NO_ERROR){
		// std::cout<<"\nGL_ERROR: Error setting spotlight attributes";
	}
	/// Set amount of lights
	loc = glGetUniformLocation(shader->shaderProgram, "activeLights");
	if (loc != -1) {
		error = glGetError();
		glUniform1i(loc, activeLights);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting amount of active lights";
		}
	}
	else
		std::cout<<"\nUnable to find uniform for activeLights.";
}
