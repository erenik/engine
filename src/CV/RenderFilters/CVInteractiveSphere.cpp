/// Emil Hedemalm
/// 2014-11-28
/// o.o

#include "CVRenderFilters.h"
#include "CV/CVPipeline.h"
#include "CV/Data/CVSwipe.h"

#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "Model/Model.h"
#include "TextureManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMLight.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "PhysicsLib/EstimatorFloat.h"

#include "Network/NetworkManager.h"
#include "Network/Session/SessionTypes.h"
#include "Network/SIP/SIPSession.h"

#include "Window/AppWindowManager.h"

#include "Message/Message.h"
#include "Message/MessageManager.h"

#include "File/FileUtil.h"

#include "Sphere.h"

bool quaternions = false;
bool useSIP = false;

int interactiveSpherePort = 33003;

int numSlides = 9;

CVInteractiveSphere::CVInteractiveSphere()
	: CVRenderFilter(CVFilterID::INTERACTIVE_SPHERE)
{
//		CVFilterSetting * sphereScale, * boardScale, * boardDistance,
	//	* rotationSpeed;
	sphereScale = new CVFilterSetting("Sphere scale", 30.f);
	boardScale = new CVFilterSetting("Board scale", Vector3f(0.8f,0.5f,1));
	boardDistance = new CVFilterSetting("Board distance", 1.4f);
	rotationSpeed = new CVFilterSetting("Rotation speed", 0.02f);
	focusItem = new CVFilterSetting("Focus item", 0);
	instantSwitches = new CVFilterSetting("Instant switches", false);
	boardAlpha = new CVFilterSetting("Board alpha", 0.8f);
	textureDir = new CVFilterSetting("Texture dir", CVSettingType::STRING, "img/erenik");
	boardRotation = new CVFilterSetting("Board rotation", Vector3f(0,0,PI*0.5));
	sphereRotation = new CVFilterSetting("Sphere rotation", Vector3f(0,0,0));
	connectToPeer = new CVFilterSetting("Connect to peer", CVSettingType::STRING, "127.0.0.1");
	changeHost = new CVFilterSetting("Change host");
	projectedContent = new CVFilterSetting("Projected content", 0);
	syncWithClient = new CVFilterSetting("Sync with client");
	projectionScreenScale = new CVFilterSetting("Projection screen scale", 50.f);
	projectionScreenSphereSegmentSize = new CVFilterSetting("Projection screen curvature", Vector2f(1.f, 1.f));
	projectionScreenSphereOffset = new CVFilterSetting("Projection screen sphere offset", 0.0f);
	whiteBackground = new CVFilterSetting("White Background", false);
	projectionModel = new CVFilterSetting("Projection model", 0);
	sphereColor = new CVFilterSetting("Sphere color", Vector4f(1,1,1,1));
	port = new CVFilterSetting("Network/Host Port", interactiveSpherePort);
	switchDuration = new CVFilterSetting("Switch duration (sphere)", 1.f);

	settings.Add(22, 
		sphereScale, boardScale, boardDistance, 
		rotationSpeed, focusItem, instantSwitches,
		boardAlpha, textureDir, boardRotation,
		sphereRotation, connectToPeer, changeHost,
		projectedContent, syncWithClient, projectionScreenScale, 
		projectionScreenSphereSegmentSize, projectionScreenSphereOffset, whiteBackground,
		projectionModel, sphereColor, port,
		switchDuration);
	
	sphereEntity = NULL;
	projectionScreenEntity = NULL;
	projectionTexture = NULL;
	session = NULL;
}

CVInteractiveSphere::~CVInteractiveSphere()
{
	MapMan.DeleteEntities(GetEntities());
}

int CVInteractiveSphere::Process(CVPipeline * pipe)
{
	if (port->HasChanged())
	{
		interactiveSpherePort = port->GetInt();
		// Delete session and re-create it then, as needed.
		SAFE_DELETE(session);
	}

	// Set integrator to be a simple one?
	if (!sphereEntity)
	{
		// Remove gravity..
		Physics.QueueMessage(new PMSet(PT_GRAVITY, Vector3f()));

		sphereEntity = MapMan.CreateEntity("Sphere", ModelMan.GetModel("Sphere.obj"), TexMan.GetTexture("0xAAFF"));
//		boardReferenceEntity = MapMan.CreateEntity("Board reference/pivot entity", NULL, NULL);
	//	Physics.QueueMessage(new PMSetEntity(boardReferenceEntity, PT_SET_PARENT, sphereEntity));
		if (!quaternions)
			Physics.QueueMessage(new PMSetEntity(sphereEntity, PT_USE_QUATERNIONS, false));
		Physics.QueueMessage(new PMSetEntity(sphereEntity, PT_SET_ROTATION, Vector3f(0, 0, PI * 0.5)));

		projectionScreenEntity = MapMan.CreateEntity("ProjectionScreen", ModelMan.GetModel("Sprite.obj"), TexMan.GetTexture("0xFF"));
	}

	if (sphereColor->HasChanged())
	{
		// Assign color as a texture.
		Graphics.QueueMessage(new GMSetEntityTexture(sphereEntity, TexMan.GenerateTexture(sphereColor->GetVec4f()))); 
	}

	if (projectionModel->HasChanged())
	{
		switch(projectionModel->GetInt())
		{
			case 0:
			default:
				// Default: The weird sphere?
				AssignProjectionSphere();
				break;
			case 1:
				// A regular 1x1 quad sprite.
				GraphicsMan.QueueMessage(new GMSetEntity(projectionScreenEntity, GT_MODEL, ModelMan.GetModel("Sprite.obj")));
				break;

		}
	}

	if (projectionScreenSphereSegmentSize->HasChanged() || projectionScreenSphereOffset->HasChanged())
	{
		// Default: The weird sphere?
		AssignProjectionSphere();
	}

	if (sphereScale->HasChanged())
	{
		PhysicsMan.QueueMessage(new PMSetEntity(sphereEntity, PT_SET_SCALE, sphereScale->GetFloat()));
		PhysicsMan.QueueMessage(new PMSetEntity(projectionScreenEntity, PT_SET_SCALE, sphereScale->GetFloat()));
	}

	if (sphereRotation->HasChanged())
	{
		PhysicsMan.QueueMessage(new PMSetEntity(sphereEntity, PT_SET_ROTATION, sphereRotation->GetVec3f()));
	}
	if (pipe->swipeState == SwipeState::SWIPE_ENDED)
	{
		Vector2f swipeDir = pipe->swipeGestureDirection;
		// 
#define Y_LIMIT 0.77f
		// Turn the wheel.
		if (swipeDir.y < -Y_LIMIT)
		{
			focusItem->SetInt(focusItem->GetInt() + 1);
		}
		else if (swipeDir.y > Y_LIMIT)
		{
			focusItem->SetInt(focusItem->GetInt() - 1);
		}
		// Sync
		if (swipeDir.x > 0.65f)
		{
			syncWithClient->SetBool(true);
		}
	}

	// Load lighting?
	static bool lightingLoaded = false;
	if (!lightingLoaded)
	{
		Lighting lighting;
		lighting.LoadFrom("lighting/InteractiveSphere");
		GraphicsMan.QueueMessage(new GMSetLighting(lighting));
		lightingLoaded = true;
	}
	// Create the small screens?
	if (boardEntities.Size() == 0)
	{
		int numToCreate = numSlides;
		rotationBetween = 2 * PI / numToCreate;
		for (int i = 0; i < numToCreate; ++i)
		{
			String color = ("0xFFFFFFFF");
			Entity * board = MapMan.CreateEntity("Board " + String(i), ModelMan.GetModel("Sprite"), TexMan.GetTexture(color));
			boardEntities.Add(board);
			// Rotate 'em.
			Physics.QueueMessage(new PMSetEntity(board, PT_SET_PRE_TRANSLATE_ROTATION, Quaternion(Vector3f(0,1,0), rotationBetween * i)));
		}
		Physics.QueueMessage(new PMSetEntity(boardEntities, PT_SET_PARENT, sphereEntity));
		// Move 'em out.
	}
	if (boardRotation->HasChanged())
		Physics.QueueMessage(new PMSetEntity(boardEntities, PT_SET_ROTATION, boardRotation->GetVec3f()));
	if (textureDir->HasChanged())
	{
		GrabTextures();
		SetTexturesForBoards();
	}

	if (boardAlpha->HasChanged())
		Graphics.QueueMessage(new GMSetEntityf(boardEntities, GT_ALPHA, boardAlpha->GetFloat()));
	if (boardScale->HasChanged())
	{
		Physics.QueueMessage(new PMSetEntity(boardEntities, PT_SET_SCALE, boardScale->GetVec3f()));
	}
	if (boardDistance->HasChanged())
	{
		Physics.QueueMessage(new PMSetEntity(boardEntities, PT_POSITION_Z, boardDistance->GetFloat()));
	}
	if (rotationSpeed->HasChanged())
	{
		if (quaternions)
			Physics.QueueMessage(new PMSetEntity(sphereEntity, PT_CONSTANT_ROTATION_SPEED, Vector3f(0, rotationSpeed->GetFloat(), 0)));
	}

	if (focusItem->HasChanged())
	{
		GoToItem(focusItem->GetInt());
	}
	if (projectionScreenScale->HasChanged())
		UpdateProjectionScale();

	if (whiteBackground->HasChanged())
	{
		WindowMan.ListWindows();
		AppWindow * AppWindow = WindowMan.GetWindowByName("ProjectionWindow");
		if (!AppWindow)
			return CVReturnType::RENDER;
		bool white = whiteBackground->GetBool();
		// Grab background-color
		if (white)
		{
			// Set background color
			window->SetBackgroundColor(Vector4f(1,1,1,1), true);
			// Set particle stuffs.
			MesMan.QueueMessages("SetParticleBlendEquation:Subtractive");			
		}
		else {
			// Set background color
			window->SetBackgroundColor(Vector4f(0,0,0,1), true);
			// Set particle stuffs.
			MesMan.QueueMessages("SetParticleBlendEquation:Additive");			
		}
	}

	// Setup network.
	if (useSIP)
	{
		if (!NetworkMan.GetSIPSession())
		{
			bool ok = NetworkMan.StartSIPServer();
			assert(ok);
		}
		else 
		{
			SIPSession * sip = NetworkMan.GetSIPSession();
			if (!sip->IsHost())
			{
				bool ok = sip->Host();
				// Not ok?
				assert(ok);
			}
			else 
			{
				// SIP OK. Do messages.
				if (connectToPeer->HasChanged())
				{
					SIPSession * sip = NetworkMan.GetSIPSession();
					sip->ConnectTo(connectToPeer->GetString(), 33000, 33001);
				}
				if (changeHost->HasChanged())
				{
					if (host == 0)
					{
						host = 1;
						sip->SendInfo("ChangeHost:2");
					}
					else 
					{
						host = (host == 1? 2 : 1);
						String toSend = "ChangeHost:"+String(host == 1? 2 : 1);
						sip->SendInfo(toSend);
					}
					OnHostChanged();
				}
				if (projectedContent->HasChanged())
				{
					String cmd = "ProjectContent:"+String(projectedContent->GetInt()); 
					sip->SendInfo(cmd);
				}
				if (syncWithClient->HasChanged())
				{
					String cmd = "ProjectContent:"+String(focusItem->GetInt());
					sip->SendInfo(cmd);
				}
			}
		}
	}
	/// Non-SIP.
	else {
		if (!session)
		{
			session = new Session("CVInteractiveSphere", "CVInteractiveSphere", SessionType::NULL_TYPE);
			// Add it to the network manager so that it actually checks stuff.
			NetworkMan.AddSession(session);
			bool hosted = session->Host(interactiveSpherePort);
			assert(hosted && "Could not host on target port!");
			if (hosted)
				std::cout<<"\nHosted on port "<<interactiveSpherePort;
		}
		else 
		{
			if (connectToPeer->HasChanged())
			{
	//			if (!session->IsHost())
	//			{
				if (connectToPeer->GetString() != "127.0.0.1")
				{
					std::cout<<"\nConnecting to "<<connectToPeer->GetString()<<" on port: "<<interactiveSpherePort;
					bool connected = session->ConnectTo(connectToPeer->GetString(), interactiveSpherePort);
					std::cout<<"\nConnected to peer? "<<connected;
				}
	//			}
			}
			if (changeHost->HasChanged())
			{
				if (host == 0)
				{
					host = 1;
					session->SendText("ChangeHost:2");
				}
				else 
				{
					host = (host == 1? 2 : 1);
					String toSend = "ChangeHost:"+String(host == 1? 2 : 1);
					session->SendText(toSend);
				}
				OnHostChanged();
			}
			if (projectedContent->HasChanged())
			{
				String cmd = "ProjectContent:"+String(projectedContent->GetInt()); 
				session->SendText(cmd);
			}
			if (syncWithClient->HasChanged())
			{
				String cmd = "ProjectContent:"+String(focusItem->GetInt());
				session->SendText(cmd);
			}
		}
	}

	return CVReturnType::RENDER;
}

void CVInteractiveSphere::ProcessMessage(Message * message)
{
	switch(message->type)
	{
		case MessageType::STRING:
		case MessageType::SIP:
		{
			String msg = message->msg;
			if (msg.Contains("ProjectContent"))
			{
				List<String> tokens = msg.Tokenize(":");
				if (tokens.Size() < 2)
				{
					std::cout<<"\n DERP::::";
					break;
				}
				int index = msg.Tokenize(":")[1].ParseInt();
				focusItem->SetInt(index);
				GoToItem(focusItem->GetInt());
			}
			else if (msg.Contains("ChangeHost:"))
			{
				String arg = msg.Tokenize(":")[1];
				host = arg.ParseInt();
				OnHostChanged();
			}
			break;	
		}
	}
}



List<Entity*> CVInteractiveSphere::GetEntities()
{
	return boardEntities + sphereEntity + projectionScreenEntity; 
}

void CVInteractiveSphere::GoToItem(int index)
{
	// Rotate it?
	bool instant = instantSwitches->GetBool();
	float targetRotY = index * rotationBetween;
	if (instant)
	{
		if (quaternions)
			Physics.QueueMessage(new PMSetEntity(sphereEntity, PT_SET_ROTATION, Quaternion(Vector3f(0,1,0), targetRotY))); 
		else 
			Physics.QueueMessage(new PMSetEntity(sphereEntity, PT_SET_ROTATION, Vector3f(0, targetRotY, PI * 0.5)));
	}
	// Slide it! o.o
	else 
	{
		/// Clear previous estimators first to avoid lag or over-spam calculations.
		PhysicsMan.QueueMessage(new PMClearEstimators(sphereEntity));
		float currentRotY = sphereEntity->rotation.y;
		EstimatorFloat * estimator = new EstimatorFloat();
		estimator->inheritFirstValue = true;
		estimator->AddStateMs(targetRotY, switchDuration->GetFloat() * 1000);
		PhysicsMan.QueueMessage(new PMSlideEntity(sphereEntity, PT_ROTATION_Y, estimator));

		/// So.. we got an index.
		while (index < 0)
			index += numSlides;
		index = index % numSlides;
		index = index % textures.Size();
		
		Texture * tex = TexMan.GetTexture(textures[index]);
		assert(tex);
		projectionTexture = tex;
		// Got a good index now.
		GraphicsMan.QueueMessage(new GMSetEntityTexture(projectionScreenEntity, DIFFUSE_MAP | SPECULAR_MAP, tex));
		UpdateProjectionScale();
	}
}

void CVInteractiveSphere::UpdateProjectionScale()
{
	if (!projectionTexture)
		return;
	// Check size.
	float widthRatio = projectionTexture->width / (float)projectionTexture->height;
	float scale = projectionScreenScale->GetFloat();
	Vector2f scale2f(widthRatio * scale, scale);
	// Re-scale the entity to given scale.
	PhysicsMan.QueueMessage(new PMSetEntity(projectionScreenEntity, PT_SET_SCALE, scale2f)); 
}

void CVInteractiveSphere::GrabTextures()
{
	textures.Clear();
	GetFilesInDirectory(textureDir->GetString(), textures);
	for (int i = 0; i < textures.Size(); ++i)
	{
		String & tex = textures[i];
		tex = textureDir->GetString() + "/" + tex;
	}
}

void CVInteractiveSphere::SetTexturesForBoards()
{
	for (int i = 0; i < boardEntities.Size(); ++i)
	{
		Entity * board = boardEntities[i];
		// Set texture
		String tex;
		if (textures.Size())
			tex = textures[i % textures.Size()];
		else 
			tex = "White";
		Graphics.QueueMessage(new GMSetEntityTexture(board, DIFFUSE_MAP | SPECULAR_MAP, tex));
	}
}

void CVInteractiveSphere::OnHostChanged()
{
	bool isHost;
	switch(host)
	{
		// Me host.
		case 0: case 1:
			// Make sphere-entity visible.
			isHost = true;
			break;
		// Me 'client'
		case 2:
			isHost = false;
			break;
		default:
			assert(false);
	}
//	isHost = false;

	// Change lighting for the both modes.
	if (isHost)
	{
		Lighting lighting;
		lighting.LoadFrom("lighting/InteractiveSphere");
		GraphicsMan.QueueMessage(new GMSetLighting(lighting));
	}
	else 
	{
		Lighting lighting;
		lighting.LoadFrom("lighting/InteractiveProjection");
		GraphicsMan.QueueMessage(new GMSetLighting(lighting));		
	}

	GraphicsMan.QueueMessage(new GMSetEntityb(sphereEntity, GT_VISIBILITY, isHost, true));
	GraphicsMan.QueueMessage(new GMSetEntityb(projectionScreenEntity, GT_VISIBILITY, !isHost, true));

	// Unregister particle effects?
	// Since it's in another filter, send a message to toggle in?
	MesMan.QueueMessages((isHost? "EnableFilter" : "DisableFilter") + String(":Optical flow"));
}

void CVInteractiveSphere::AssignProjectionSphere()
{
	// Just create a new model.. for now.
	Model * model = ModelMan.NewDynamic();
	// Define new curvature parameters.
	model->mesh = Sphere::CreateSegmentFromEquator(projectionScreenSphereSegmentSize->GetVec2f(), Vector2i(10,10), projectionScreenSphereOffset->GetFloat(), true);
	model->SetName(model->mesh->name);
	GraphicsMan.QueueMessage(new GMSetEntity(projectionScreenEntity, GT_MODEL, model)); 
}

