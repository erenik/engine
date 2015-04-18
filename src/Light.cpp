/// Emil Hedemalm
/// 2014-01-19
/// Simple structure for a light (any kind)

#include "Light.h"
#include <fstream>

#include "Window/AppWindowManager.h"

#include "UI/UserInterface.h"
#include "UI/UIButtons.h"

#include "Message/MathMessage.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GraphicsMessages.h"

List<Light*> Light::lights;

Light::Light(String name)
: name(name)
{
	Nullify();
	lights.Add(this);
}

Light::Light(Lighting * lighting)
{
	Nullify();
	this->lighting = lighting;
	lights.Add(this);
}

void Light::Nullify()
{
	isStar = false;
	attenuation[0] = 1.0f; 
	type = 1;
	lastUpdate = 0;
	spotExponent = 5;
	spotCutoff = 30;
	data = NULL;
	owner = NULL;
	registeredForRendering = false;
	lighting = NULL;
	currentlyActive = true;
	castsShadow = false;
	shadowMap = NULL;
	shadowMapIndex = -1;
}

Light::Light(const Light & otherLight)
{
	Nullify();
	type = otherLight.type;
	ambient = otherLight.ambient;
	diffuse = otherLight.diffuse;
	specular = otherLight.specular;
	position = otherLight.position;
	attenuation = otherLight.attenuation;
	currentlyActive = otherLight.currentlyActive;
	lastUpdate = otherLight.lastUpdate;
	spotDirection = otherLight.spotDirection;
	spotExponent = otherLight.spotExponent;
	spotCutoff = otherLight.spotCutoff;
	name = otherLight.name;
	data = otherLight.data;
	owner = NULL;
}

Light::~Light()
{
	lights.Remove(this);
}

void Light::FreeAll()
{
	while(lights.Size())
		delete lights[0];
}

	

String lightEditorName = "LightEditor";
	

/// Opens a dedicated editor AppWindow for this light. Assumes a valid LightEditor.gui is available in the UI directory.
void Light::OpenEditorWindow()
{
	// Look for an existing lighting-editor AppWindow.
	AppWindow * window = WindowMan.GetWindowByName(lightEditorName);
	if (window)
	{
	
	}
	// Create it if not existing.
	else {
		window = WindowMan.NewWindow(lightEditorName, "Light editor");
		UserInterface * ui = window->CreateUI();
		ui->Load("gui/LightEditor.gui");
		window->DisableAllRenders();
		window->renderUI = true;
		window->SetRequestedSize(Vector2i(400, 320));
		window->CreateGlobalUI();
		window->Create();
	}
	
	// Show it.
	window->Show();
	// Bring it to the top if needed.
	window->BringToTop();
	// Update stats within the AppWindow.
	OnPropertiesUpdated();
}

void Light::CloseEditorWindow()
{
	// Look for an existing lighting-editor AppWindow.
	AppWindow * window = WindowMan.GetWindowByName(lightEditorName);
	if (!window)
		return;
	window->Hide();
}

// For interaction with UI as well as scripting.
void Light::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			List<String> tokens = msg.Tokenize("(),");
			String param = tokens[2];
			String arg = tokens[3];
			if (param.Contains("shadowMapFarplane"))
			{
				shadowMapFarplane = arg.ParseFloat();
			}
			else if (param.Contains("shadowMapZoom"))
				shadowMapZoom = arg.ParseFloat();
			break;
		}
		case MessageType::SET_STRING:
		{
			SetStringMessage * ssm = (SetStringMessage*)message;
			if (msg == "SetLightName")
			{
				name = ssm->value;
				OnPropertiesUpdated();
				assert(lighting);
				this->lighting->UpdateLightList();
			}
			break;
		}
		case MessageType::VECTOR_MESSAGE:
		{
			VectorMessage * vm = (VectorMessage*) message;
			if (msg == "SetLightColor")
			{
				this->diffuse = this->specular = vm->GetVector4f();
			}
			else if (msg == "SetLightDiffuse")
			{
				this->diffuse = vm->GetVector4f();
			}
			else if (msg == "SetLightSpecular")
			{
				this->specular = vm->GetVector4f();
			}
			else if (msg == "SetLightAttenuation")
			{
				this->attenuation = vm->GetVector4f();
			}
			else if (msg == "SetLightPosition")
			{
				this->position = vm->GetVector4f();
			}
			else if (msg == "SetLightSpotDirection")
			{
				this->spotDirection = vm->GetVector4f();
			}
			break;
		}
		case MessageType::INTEGER_MESSAGE: 
		{
			IntegerMessage * im = (IntegerMessage*) message;
			if (msg == "SetLightType")
			{
				this->type = im->value;
			}
			else if (msg == "SetLightSpotCutoff")
			{
				spotCutoff = im->value;
			}
			else if (msg == "SetLightSpotExponent")
				spotExponent = im->value;
			break;	
		}
	}
}

// For interaction with UI as well as scripting.
void Light::ProcessMessageStatic(Message * message)
{
	String msg = message->msg;
	List<String> tokens = msg.Tokenize("(,)");
	String name = tokens[1];
	String param = tokens[2];
	param.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	String arg = tokens[3];
	Light * light = GetLight(name);
	if (light)
		light->ProcessMessage(message);
}


// Updates UI as necessary
void Light::OnPropertiesUpdated()
{
	AppWindow * window = WindowMan.GetWindowByName(lightEditorName);
	if (!window)
		return;
	UserInterface * ui = window->ui;
	if (!ui)
		return;
	Graphics.QueueMessage(new GMSetUIs("LightName", GMUI::STRING_INPUT_TEXT, name, ui));
	Graphics.QueueMessage(new GMSetUIv3f("LightPosition", GMUI::VECTOR_INPUT, position, ui));
	Graphics.QueueMessage(new GMSetUIv3f("LightDiffuse", GMUI::VECTOR_INPUT, diffuse, ui));
	Graphics.QueueMessage(new GMSetUIv3f("LightColor", GMUI::VECTOR_INPUT, diffuse, ui));
	Graphics.QueueMessage(new GMSetUIv3f("LightSpecular", GMUI::VECTOR_INPUT, specular, ui));
	Graphics.QueueMessage(new GMSetUIv3f("LightAttenuation", GMUI::VECTOR_INPUT, attenuation, ui));
	Graphics.QueueMessage(new GMSetUIi("LightType", GMUI::INTEGER_INPUT, type, ui));
	Graphics.QueueMessage(new GMSetUIv3f("LightSpotDirection", GMUI::VECTOR_INPUT, spotDirection, ui));
	Graphics.QueueMessage(new GMSetUIi("LightSpotCutoff", GMUI::INTEGER_INPUT, spotCutoff, ui));
	Graphics.QueueMessage(new GMSetUIi("LightSpotExponent", GMUI::INTEGER_INPUT, spotExponent, ui));
}


// Versions
#define LIGHT_VERSION_0 0// Initial version.
int lightVersion = LIGHT_VERSION_0;

/// Writes to file stream.
void Light::WriteTo(std::fstream & file){
	// Write version
	file.write((char*)&lightVersion, sizeof(int));
	// Write name
	name.WriteTo(file);
	// Write type
	file.write((char*)&type, sizeof(int));

	// Write spotlight stats
	file.write((char*)&spotCutoff, sizeof(float));
	file.write((char*)&spotExponent, sizeof(int));
	
	// Write all them vectors.
	ambient.WriteTo(file);
	diffuse.WriteTo(file);
	specular.WriteTo(file);
	position.WriteTo(file);
	attenuation.WriteTo(file);
	spotDirection.WriteTo(file);
	
}
/// Reads from file stream.
void Light::ReadFrom(std::fstream & file){
	// Read version
	int version;
	file.read((char*)&version, sizeof(int));
	assert(version == LIGHT_VERSION_0);
	// Read name
	name.ReadFrom(file);
	// Read type
	file.read((char*)&type, sizeof(int));
	// Read spotlight stats
	file.read((char*)&spotCutoff, sizeof(float));
	file.read((char*)&spotExponent, sizeof(int));

	// Read all them vectors.
	ambient.ReadFrom(file);
	diffuse.ReadFrom(file);
	specular.ReadFrom(file);
	position.ReadFrom(file);
	attenuation.ReadFrom(file);
	spotDirection.ReadFrom(file);
}


void Light::SetName(const String newName){ 
	assert(abs(newName.Length()) < 5000);
	name = newName; 
}

Light * Light::GetLight(String byName)
{
	for (int i = 0; i < lights.Size(); ++i)
	{
		Light * light = lights[i];
		if (light->name == byName)
			return light;
	}
	return NULL;
}



