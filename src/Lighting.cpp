#include "Lighting.h"

#include <Graphics/OpenGL.h>

#include <iostream>
#include <ctime>
#include <cstring>
#include <fstream>

#include "Shader.h"
#include "GraphicsState.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GraphicsMessages.h"

#include "Window/WindowManager.h"

#include "UI/UserInterface.h"
#include "UI/UIButtons.h"

#include "Message/MessageManager.h"
#include "Message/MathMessage.h"
#include "Message/FileEvent.h"

Window * Lighting::lightingEditor = NULL;

String lightList = "LightList";

/// Default constructor, sets all light pointers and other variables to 0/NULL
Lighting::Lighting()
{
	global_ambient = Vector4f(1.0f, 1.0f, 1.0f, 0.1f);
	this->lastUpdate = GetTime();
	this->activeLight = NULL;
	this->activeLightIndex = 0;
	this->lightCounter = 0;
}
Lighting::~Lighting()
{
	lights.ClearAndDelete();
}

/// Copy constructor ^^
Lighting::Lighting(const Lighting& otherLighting)
{
	Copy(&otherLighting);
}

/// Assignment operator...
const Lighting * Lighting::operator = (const Lighting & otherLighting)
{
	Copy(&otherLighting);
	return this;
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
}

/// Used for all copy-constructors.
void Lighting::Copy(const Lighting * fromThisLighting)
{
	lights.ClearAndDelete();
	for (int i = 0; i < fromThisLighting->lights.Size(); ++i)
	{
		Light * otherLight = fromThisLighting->lights[i];
		Light * newLight = new Light(*otherLight);
		lights.Add(newLight);
	}
	global_ambient = fromThisLighting->global_ambient;
	this->lastUpdate = fromThisLighting->lastUpdate;
	this->activeLight = NULL;	// Pointer, set to NULL!!!
	this->activeLightIndex = 0;	// Default to null
	this->lightCounter = fromThisLighting->lightCounter;
}

/// Returns true if the message had any meaning, adjusting values within the lighting.
bool Lighting::ProcessMessage(Message * message)
{
	String msg = message->msg;
	// See if it's a message that could apply to our lights.
	if (msg.StartsWith("SetLight"))
	{
		// Fetch active light.
		Light * light = activeLight;
		if (light)
		{
			light->lighting = this;
			light->ProcessMessage(message);
		}
		return true;
	}
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


/// Creates an editor-window for this lighting, assuming the existance of LightingMenu and LightEditor GUI files.
Window * Lighting::OpenEditorWindow()
{
	// Look for an existing lighting-editor window.
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
void Lighting::UpdateLightList(Window * inWindow)
{
	Window * window = inWindow;
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

/// DEPRECATE: bad arguments... Adds light to the lighting, return NULL upon falure. Note that the light is copied in this case! TODO: Remove this function.
Light * Lighting::Add(Light * newLight)
{
	assert(false);
	if (lights.Size() >= MAX_LIGHTS)
		return NULL;
	Light * light = new Light(*newLight);
	light->currentlyActive = true;
	activeLight = light;
	/// Set default values
	this->lastUpdate = GetTime();
	lights.Add(light);
	return light;
}

/// Removes target light. Returns false on failure.
bool Lighting::Remove(Light * toRemove)
{
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
void Lighting::SetAmbient(int r, int g, int b, int a){
	this->global_ambient = Vector4f(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
	this->lastUpdate = GetTime();
}
/// Sets ambient using floats.
void Lighting::SetAmbient(float r, float g, float b, float a){
	this->global_ambient = Vector4f(r,g,b,a);
	this->lastUpdate = GetTime();
}
/// Sets ambient values. Alpha defaults to 0.
void Lighting::SetAmbient(Vector3f values){
	this->global_ambient = Vector4f(values);
	this->lastUpdate = GetTime();
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
	this->lastUpdate = GetTime();
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
	lastUpdate = clock();
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


/// Loads selected lighting into the active shader program
void LoadLighting(Lighting * lighting, Shader * shader)
{
	
	GLuint loc = -1, error;
	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::cout<<"\nError before LoadLighting";
	}
	if (!lighting)
		return;
	if (!shader)
		return;
//#define PRINT_DEBUG

	// Return. If no ambient is present no other light will be either.
	if (shader->uniformLight.ambientVec4 == -1)
		return;

	CheckGLError("LoadLighting before gl calls.");

	/// Set ambient
	GLfloat ambient[4];
	ambient[0] = lighting->global_ambient.x;
	ambient[1] = lighting->global_ambient.y;
	ambient[2] = lighting->global_ambient.z;
	ambient[3] = lighting->global_ambient.w;
	glUniform4fv(shader->uniformLight.ambientVec4, 1, ambient);
	CheckGLError("Error setting global ambient luminosity");
	
	// If no diffuse uniform, abort.
	if (shader->uniformLight.diffuseVec4 == -1)
		return;

	/// Gather all data
	static GLfloat diffuse[MAX_LIGHTS * 4];
	static GLfloat specular[MAX_LIGHTS * 4];
	static GLfloat position[MAX_LIGHTS * 3];
	static GLfloat attenuation[MAX_LIGHTS * 3];
	static GLint type[MAX_LIGHTS];
	static GLfloat spotDirection[MAX_LIGHTS * 3];
	static GLfloat spotCutoff[MAX_LIGHTS];
	static GLint spotExponent[MAX_LIGHTS];
	int activeLights = 0;
	for (int i = 0; i < lighting->lights.Size(); ++i)
	{
		Light * light = lighting->lights[i];
		if (!light->currentlyActive)
			continue;
		// Only take those lights which are visible in the frustum, or will reach it with their light?
		// ... 

		int interval = 4;
		diffuse[activeLights*interval] = light->diffuse.x;
		diffuse[activeLights*interval+1] = light->diffuse.y;
		diffuse[activeLights*interval+2] = light->diffuse.z;
		diffuse[activeLights*interval+3] = light->diffuse.w;

		specular[activeLights*interval] = light->specular.x;
		specular[activeLights*interval+1] = light->specular.y;
		specular[activeLights*interval+2] = light->specular.z;
		specular[activeLights*interval+3] = light->specular.w;

		interval = 3;
		position[activeLights*interval] = light->position.x;
		position[activeLights*interval+1] = light->position.y;
		position[activeLights*interval+2] = light->position.z;

		attenuation[activeLights*interval] = light->attenuation.x;
		attenuation[activeLights*interval+1] = light->attenuation.y;
		attenuation[activeLights*interval+2] = light->attenuation.z;

		type[activeLights] = light->type;

		spotDirection[activeLights*interval] = light->spotDirection.x;
		spotDirection[activeLights*interval+1] = light->spotDirection.y;
		spotDirection[activeLights*interval+2] = light->spotDirection.z;
		/// Calcualte cutoff as a cosine value of the degrees converted to radians before we throw it in ^^
		spotCutoff[activeLights] = cos(light->spotCutoff / 180.0f * PI);
		spotExponent[activeLights] = light->spotExponent;

		activeLights++;
	}

//	std::cout<<"\nActive lights: "<<activeLights<<" of "<<lighting->lights.Size();


	/// Set all data
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
		error = glGetError();
		if (error != GL_NO_ERROR){
			//std::cout<<"\nGL_ERROR: Error setting light attenuation properties";
		}
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
