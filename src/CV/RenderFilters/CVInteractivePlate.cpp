/// Emil Hedemalm
/// 2014-11-10
/** For interactive kitchen demonstration, of a plate that will have content projected onto it, 
	and a selection of menus being browsable using swipe-gestures.
*/

#include "CVRenderFilters.h"
#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "CV/CVPipeline.h"
#include "CV/DataFilters/CVSwipeGesture.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "PhysicsLib/EstimatorFloat.h"

#include "Network/Http.h"
#include "Network/CURLManager.h"
#include "Message/Message.h"

#include "XML/XMLParser.h"
#include "XML/XMLElement.h"

#include "File/FileUtil.h"

CVInteractivePlate::CVInteractivePlate()
	: CVRenderFilter(CVFilterID::INTERACTIVE_PLATE)
{
	plateEntity = NULL;
	averagedPosition = Vector3f();
	averagedScale = 1.f;

	platePositionSmoothingFactor = new CVFilterSetting("Plate position smoothing", 0.2f);
	plateRotationSpeed = new CVFilterSetting("Plate Rotation speed", 0.2f);
	plateScale = new CVFilterSetting("Plate scale", 1.f);
	rotationOffset = new CVFilterSetting("Rotation offset", 0.f);
	// An integer. Swipes manipulate this value.
	currentMenu = new CVFilterSetting("Current menu", 0);
	sleepTime = new CVFilterSetting("Sleep time", 10);
	lockPlateInCenter = new CVFilterSetting("Lock plate in center", false);
	lockPlateScale = new CVFilterSetting("Lock plate scale", false);

	webPage = new CVFilterSetting("Web page");
	webPage->type = CVSettingType::STRING;
	webPage->SetString("http://10.42.0.1/");

	remoteBrowser = new CVFilterSetting("Remote browser");
	remoteBrowser->type = CVSettingType::STRING;
	remoteBrowser->SetString("remote_iceweasel");

	about = "Rotation offset in multiples of 90 degrees";

	settings.Add(10, 
		platePositionSmoothingFactor, plateRotationSpeed, plateScale,
		rotationOffset, currentMenu, sleepTime,
		lockPlateInCenter, lockPlateScale, webPage, 
		remoteBrowser);

	sleeping = false;
	lastInteraction = Time::Now();
}

CVInteractivePlate::~CVInteractivePlate()
{
}

int CVInteractivePlate::Process(CVPipeline * pipe)
{
	Time now = Time::Now();
	if ((now - lastInteraction).Seconds() > sleepTime->GetInt())
	{
		SetSleepThread(true);
	}

	// Paint optical flow!
	RenderOpticalFlow(pipe);

	static bool sent = false;
	if ((recipes.Size() == 0 && !sent) || 
		webPage->HasChanged() || 
		remoteBrowser->HasChanged())
	{
		FetchRecipes();
		sent = true;
	}
	// Create plate-entity.
	if (!plateEntity)
	{
		plateEntity = MapMan.CreateEntity("InteractivePlate PlateEntity", ModelMan.GetModel("Sprite.obj"), TexMan.GetTexture("White")); 
		Physics.QueueMessage(new PMSetEntity(plateEntity, PT_COLLISIONS_ENABLED, false));
	}

	if (lockPlateInCenter->HasChanged() && lockPlateInCenter->GetBool())
	{
		// Center it.
		Physics.QueueMessage(new PMSetEntity(plateEntity, PT_POSITION, Vector3f(0,0,2)));
	}
	if (lockPlateScale->HasChanged() && lockPlateScale->GetBool())
	{
		UpdateScale();
	}
	if (plateScale->HasChanged())
		UpdateScale();

	// Make it visible/invisible if a plate is visible.

	/// Check for swipes.. o.o
	if (pipe->swipeState == SwipeState::SWIPE_ENDED)
	{
		// Wake up if needed.
		SetSleepThread(false);

		lastInteraction = Time::Now();
		/// Check direction.
	//	std::cout<<"\nDetected swipe in direction: "<<pipe->swipeGestureDirection;
		
		// Rorate it.
		float rot = rotationOffset->GetFloat() * PI * 0.5f;
		Matrix4f zRot = Matrix4f::InitRotationMatrixZ(rot);
		Vector3f origDir = pipe->swipeGestureDirection;
		Vector3f rotated = zRot.Product(origDir);

		Vector2f dir = rotated;
		std::cout<<"\nRotated dir: "<<dir;
		if (dir.x < -0.7)
			// Do stuff.
			BringUpRecipe();
		else if (dir.x > 0.7)
			BringDownRecipe();
		else if (dir.y > 0.7)
			NextMenu();
		else if (dir.y < -0.7)
			PreviousMenu();
	}

	if (currentMenu->HasChanged())
		UpdatePlate();

	// Move it.
	if (pipe->circles.size() == 1)
	{
		// Paint circles
		RenderCircles(pipe);

		// Extract data.
		cv::Vec3f circle = pipe->circles[0];
		Vector2f circlePosition(circle[0], circle[1]);
		float radius = circle[2];
		float s0 = platePositionSmoothingFactor->GetFloat();
		float s1 = 1 - s0;
		averagedPosition = averagedPosition * s0 + circlePosition * s1;
		averagedScale = averagedScale * s0 + radius * s1;
		Vector3f position = averagedPosition;
		// Convert from input-analysis to projection-space.
		Vector3f worldPos = pipe->InputSpaceToWorldSpace(position);
		worldPos.z = 2;
		if (lockPlateInCenter->GetBool() == false)
		{
			PhysicsMan.QueueMessage(new PMSetEntity(plateEntity, PT_POSITION, worldPos));
		}
		recalculatedScale = averagedScale * plateScale->GetFloat();
		// Set scale too.
		if (!lockPlateScale->GetBool())
		{
			UpdateScale();
		}
	}

	// Re-scale it.
	if (plateScale->HasChanged())
	{
		recalculatedScale = averagedScale * plateScale->GetFloat();
		PhysicsMan.QueueMessage(new PMSetEntity(plateEntity, PT_SET_SCALE, recalculatedScale));
	}

	// Rotate it
	if (plateRotationSpeed->HasChanged())
	{
		PhysicsMan.QueueMessage(
			new PMSetEntity(
				plateEntity, 
				PT_CONSTANT_ROTATION_SPEED, 
				Vector3f(0, 0, plateRotationSpeed->GetFloat())
			)
		);
	}
	return CVReturnType::CV_IMAGE;
}

void CVInteractivePlate::ProcessMessage(Message * message)
{
	if (message->type != MessageType::CURL)
		return;

	CURLMessage * curlMessage = (CURLMessage*)message;
	if (!curlMessage->url.Contains("xml.php"))
		return;

	if (recipes.Size())
		return;

	if (curlMessage->contents.Length() <= 0)
		return;

	// Fetch recipies
	XMLParser parser;
	parser.data = curlMessage->contents.c_str_editable();
	parser.size = curlMessage->contents.Length();
	bool ok = parser.Parse();
	parser.data = NULL;

	List<String> parts = curlMessage->contents.Tokenize("<>");
	for (int i = 0; i < parts.Size(); ++i)
	{
		String part = parts[i];
		if (part.Contains("list "))
		{
			String length = part.Tokenize("=")[1];
			int lengthInt = length.ParseInt();
			std::cout<<"\nFound "<<lengthInt<<" items.";
		}
	}

	
	// All elements have been parsed. Look for stuff now.
	XMLElement * listElement = parser.GetElement("list");
	if (listElement)
	{
		int listLength = listElement->GetArgument("length")->value.ParseInt();
		std::cout<<"\nFound "<<listLength<<" items.";
		for (int i = 0; i < listElement->children.Size(); ++i)
		{
			Recipe newRecipe;
			XMLElement * recipe = listElement->children[i];
			String referenceID = recipe->GetElement("ref")->data;
			newRecipe.referenceID = referenceID;
			newRecipe.name = recipe->GetElement("title")->data;
			
			String imageLink = recipe->GetElement("weblink")->data;
			recipes.Add(newRecipe);

		}
	}
	// All recipes parsed.. hopefully? o.o
	if (recipes.Size())
	{
		this->PostToPi();
	}

	// String urlToProject = "http://10.42.0.100/InnoKitchen/xml.php?limit=1000";
	// Parse <code> elements for bar-codes
	// Parse <title> elements
	// Parse <list length=#> for 

	// Find images associated with the existing recipies.

	List<Recipe> oldRecipes = recipes;
	List<String> usedImages;

	// Clear old recipes
	recipes.Clear();

	// Create "new" recipes using available image data.
	for (int i = 0; i < oldRecipes.Size(); ++i)
	{
		Recipe recipe = oldRecipes[i];
		recipe.name.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		for (int j = 0; j < foodImages.Size(); ++j)
		{
			String image = foodImages[j];
			image.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (recipe.name.Contains("chili") && image.Contains("chili"))
				recipe.imageSource = image;
			if (recipe.name.Contains("Cake") && image.Contains("desertnice.png"))
				recipe.imageSource = image;
			if (recipe.name.Contains("Breakfast") && image.Contains("breakfast"))
				recipe.imageSource = image;
			if ((recipe.name.Contains("Pizza") && image.Contains("pizza")) ||
				(recipe.name.Contains("Onion Sallad") && image.Contains("OnionSallad")) ||
				(recipe.name.Contains("Some classy desert") && image.Contains("SomeClassyDesert")) ||
				(recipe.name.Contains("Cooked fish") && image.Contains("sparrisfish")) ||
				(recipe.name.Contains("schnitzel") && image.Contains("schnitzel")) ||
				(recipe.name.Contains("soup") && image.Contains("soup")) ||
				(recipe.name.Contains("some mix") && image.Contains("SomeMix")) ||
				(recipe.name.Contains("Fishy") && image.Contains("Fishy")) ||
				(recipe.name.Contains("Spaghetti") && image.Contains("Spaghetti")) ||
				(recipe.name.Contains("fruits") && image.Contains("fruits"))
//				(recipe.name.Contains("Fishy") && image.Contains("Fishy")) ||
				)
				recipe.imageSource = image;
		
		}
		if (recipe.referenceID.Length() && recipe.imageSource.Length())
		{
			// Add it
			recipes.Add(recipe);
			usedImages.Add(recipe.imageSource);
		}
		else 
		{
			std::cout<<"\nCould not find suitable image source or bad-code for recipe.";
		}
	}

	List<Recipe> recipesWithBarCodes = recipes;
	List<String> availableImages;
	for (int i = 0; i < foodImages.Size(); ++i)
	{
		String foodImage = foodImages[i];
		if (usedImages.Exists(foodImage))
		{
			std::cout<<"\nImage already in use: "<<foodImage;
			continue;
		}
		// Create a new recipe, using arbitrary recipe URL for it.
		Recipe newRecipe;
		newRecipe.imageSource = foodImage;
		if (recipesWithBarCodes.Size())
			newRecipe.referenceID = recipesWithBarCodes[i % recipesWithBarCodes.Size()].referenceID;
		recipes.Add(newRecipe);
	}
	std::cout<<"\nList of recipes: ";
	for (int i = 0; i < recipes.Size(); ++i)
	{
		std::wcout<<"\nRecipe "<<i<<": "<<recipes[i].referenceID.wc_str()<<" "<<recipes[i].imageSource.wc_str();
	}

	this->UpdatePlate();
}

List<Entity*> CVInteractivePlate::GetEntities()
{
	return plateEntity;
}

// Should be called when deleting a filter while the application is running. Removes things as necessary.
void CVInteractivePlate::OnDelete()
{
	if (plateEntity)
		MapMan.DeleteEntity(plateEntity);
	plateEntity = NULL;
}


void CVInteractivePlate::NextMenu()
{
	if (recipes.Size() == 0)
		FetchRecipes(true);
	assert(recipes.Size());
	currentMenu->SetInt((currentMenu->GetInt() + 1) % recipes.Size());
}

void CVInteractivePlate::PreviousMenu()
{
	int newInt = (currentMenu->GetInt() - 1);
	if (newInt < 0)
		newInt = recipes.Size() - 1;
	currentMenu->SetInt(newInt);
}

void CVInteractivePlate::UpdatePlate()
{
	String newTex = "Grey";
	if (recipes.Size() == 0)
	{
		std::cout<<"\nNo recipes D:";
		return;
	}
	// Fetch current recipe.
	Recipe & rec = recipes[currentMenu->GetInt()];
	newTex = rec.imageSource;
	if (newTex.Length() == 0)
	{
		// Grab file from image dir.
		newTex = foodImages[currentMenu->GetInt()];
	}
	Graphics.QueueMessage(new GMSetEntityTexture(plateEntity, DIFFUSE_MAP | SPECULAR_MAP, newTex));

	//std::cout<<"\nGot response. "<<siteContent.Length()<<" characters";
//	std::cout<<"\nSite content: "<<
}

void CVInteractivePlate::BringUpRecipe()
{
	if (recipes.Size() == 0)
		FetchRecipes();
	std::cout<<"\nBring up da recipeh!";
	// Get recipe.
	int menu = this->currentMenu->GetInt();
	std::cout<<"\nMenu #"<<menu;
	PostToPi();
}

void CVInteractivePlate::BringDownRecipe()
{
	std::cout<<"\nBring down da recipeh!";
}

void CVInteractivePlate::PostToPi()
{
	// Fetch first menu, yo.
	int menu = this->currentMenu->GetInt();
	if (recipes.Size() == 0)
	{
		std::cout<<"\nNo recipes! D:";
		return;
	}
	Recipe rec = recipes[menu % recipes.Size()];
	String urlToProject = rec.referenceID;
	if (urlToProject.Length() == 0)
	{
	
	}
	String postData = "cmd=" + urlToProject;
	std::cout<<"\nPost data: "<<postData;
	String location = webPage->GetString();
	HttpGet(location + remoteBrowser->GetString() + ".php?cmd="+rec.referenceID);
}


void CVInteractivePlate::SetSleepThread(bool sleepState)
{
	// Already sleeping?
	if (sleepState == sleeping)
		return;

	// Clear old estimators first?
	Graphics.QueueMessage(new GMClearEstimators(plateEntity));
	EstimatorFloat * floatE = new EstimatorFloat();
	if (!sleepState)
		floatE->AddStatesMs(2, 
			0.f, 0,
			1.f, 1000);
	else 
		floatE->AddStatesMs(2, 
			1.f, 0,
			0.f, 3000);
	Graphics.QueueMessage(new GMSlideEntityf(plateEntity, GT_ALPHA, floatE));

//	Graphics.QueueMessage(new GMSetEntityf(plateEntity, 
//	Graphics.QueueMessage(new GMSetEntity(plateEntity, GT_ALPHA, 
//	Graphics.QueueMessage(new GMSetEntityb(plateEntity, GT_VISIBILITY, !sleepState));
	sleeping = sleepState;
}

void CVInteractivePlate::GrabFoodImages()
{
	foodImages.Clear();
	// Grab all images in the directory!
	List<String> files;
	String dir = "img/Foodstuffs/";
	GetFilesInDirectory(dir, files);
	for (int i = 0; i < files.Size(); ++i)
	{
		String file = files[i];
		if (file.Contains(".png"))
		{
			foodImages.Add(dir + file);
		}
	}
	
}

void CVInteractivePlate::UpdateScale()
{
	if (lockPlateScale->GetFloat())
		Physics.QueueMessage(new PMSetEntity(plateEntity, PT_SET_SCALE, plateScale->GetFloat() * Vector3f(10,10,10)));
	else 
		PhysicsMan.QueueMessage(new PMSetEntity(plateEntity, PT_SET_SCALE, recalculatedScale));
}


void CVInteractivePlate::FetchRecipes(bool fromImageDir)
{
	/// Always grab images from the images-dir.
	GrabFoodImages();

	if (fromImageDir)
	{
		recipes.Clear();
		for (int i = 0; i < foodImages.Size(); ++i)
		{
			Recipe rec;
			rec.imageSource = foodImages[i];
			recipes.Add(rec);
		}
	}
	else 
	{
		String location = webPage->GetString();
		if (!location.EndsWith('/'))
			location += "/";
		HttpGet(location+"InnoKitchen/xml.php?limit=1000");
	}
}

