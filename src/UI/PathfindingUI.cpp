//
// #include "PathfindingUI.h"
//
//
// /// The UIs for the game
// extern UserInterface * ui[MAX_GAME_STATES];
//
// /** Function called when an element is activated,
// 	by mouse-clicking, pressing enter on selction or otherwise. */
// void PathfindingUI::activate(UIElement* activeElement){
//
// }
//
// int LoadNavMesh(UIElement * element){
// 	char src[MAX_PATH];
// 	strcpy(src, "waypointMaps/");
// 	strcat(src, element->text);
// 	bool success = WaypointMan.SelectNavMesh(src);
// 	if (!success){
// 		std::cout<<"\nUnable to select target navMesh...";
// 		return -1;
// 	}
// 	NavMesh * nm = WaypointMan.ActiveNavMesh();
// 	PathMan.GetPath(nm->defaultStart, nm->defaultGoal);
// 	return 0;
// }
//
// int SetAlgorithm(UIElement * element){
// 	NavMesh * nm = WaypointMan.ActiveNavMesh();
// 	PathMan.SetSearchAlgorithm(element->text);
// 	PathMan.GetPath(nm->defaultStart, nm->defaultGoal);
// 	return 0;
// }
//
//
// int OptimizeNavMesh(UIElement * element){
// 	Graphics.renderNavMesh = false;
// 	WaypointMan.Optimize();
// 	Graphics.renderNavMesh = true;
// 	return 0;
// }
// extern int GoToMainMenu(UIElement * element);
//
// void PathfindingUI::create(){
//
// 	/*
// 	element = new UIElement();
// 	element->ratio = 1024.0f / 655.0f;
// 	element->scalable = true;
// 	element->zDepth = 0;
// 	element->texture = NULL; // TexMan.LoadTexture("img/mainmenu_bg.png");
// 	element->SetName("background");
// 	element->alignment = UIElement::MAXIMIZED;
// 	element->hoverable = false;
//
// 	UIElement * button = new UIElement();
// 	button->mesh = new Square();
// 	button->texture = TexMan.LoadTexture("img/mainmenu/mainMenuButton.png");
// 	button->alignmentX = 0.1f;
// 	button->alignmentY = 0.925f;
// 	button->ratio = 128.0f / 32.0f;
// 	button->sizeRatioX = 0.2f;
// 	button->sizeRatioY = 0.15f;
// 	button->scalable = true;
// 	button->SetName("mainMenuButton");
// 	button->activateable = true;
// 	button->onActivate = GoToMainMenu;
// 	element->addChild(button);
//
// 	button = new UIElement();
// 	button->mesh = new Square();
// 	button->texture = TexMan.LoadTexture("img/80Gray50Alpha.png");
// 	button->alignmentX = 0.3f;
// 	button->alignmentY = 0.925f;
// 	button->ratio = 128.0f / 32.0f;
// 	button->sizeRatioX = 0.2f;
// 	button->sizeRatioY = 0.15f;
// 	button->scalable = true;
// 	button->SetName("optimizeButton");
// 	button->text = new char[MAX_PATH];
// 	button->text = "Optimize NavMesh");
// 	button->activateable = true;
// 	button->onActivate = OptimizeNavMesh;
// 	element->addChild(button);
//
// 	UIElement * mapDiv = new UIElement();
// 	mapDiv->mesh = new Square();
// 	mapDiv->texture = TexMan.LoadTexture("img/black50Alpha.png");
// 	mapDiv->alignmentX = 0.1f;
// 	mapDiv->alignmentY = 0.6f;
// 	mapDiv->ratio = 128.0f / 32.0f;
// 	mapDiv->sizeRatioX = 0.2f;
// 	mapDiv->sizeRatioY = 0.3f;
// 	mapDiv->scalable = true;
// 	mapDiv->SetName("mapDiv");
// 	mapDiv->activateable = false;
// 	element->addChild(mapDiv);
//
// 	for (int i = 0; i < 3; ++i){
// 		button = new UIElement();
// 		button->mesh = new Square();
// 		button->texture = TexMan.LoadTexture("img/80Gray50Alpha.png");
// 		button->alignmentX = 0.5f;
// 		button->alignmentY = 0.9f - 0.16f * i;
// 		button->sizeRatioX = 0.95f;
// 		button->sizeRatioY = 0.15f;
// 		button->scalable = true;
// 		button->activateable = true;
// 		button->onActivate = LoadNavMesh;
// 		button->SetName("mapButton");
// 		button->text = new char[MAX_PATH];
// 		switch(i){
// 			case 0:
// 				strcpy(button->text, "Map1.txt");
// 				break;
// 			case 1:
// 				strcpy(button->text, "Map2.txt");
// 				break;
// 			case 2:
// 				strcpy(button->text, "Map3.txt");
// 				break;
// 			default:
// 				strcpy(button->text, "Bleh");
// 		}
// 		mapDiv->addChild(button);
// 	}
//
// 	UIElement * algorithmDiv = new UIElement();
// 	algorithmDiv->mesh = new Square();
// 	algorithmDiv->texture = TexMan.LoadTexture("img/black50Alpha.png");
// 	algorithmDiv->alignmentX = 0.15f;
// 	algorithmDiv->alignmentY = 0.2f;
// 	algorithmDiv->ratio = 128.0f / 32.0f;
// 	algorithmDiv->sizeRatioX = 0.3f;
// 	algorithmDiv->sizeRatioY = 0.3f;
// 	algorithmDiv->scalable = true;
// 	algorithmDiv->SetName("algorithmDiv");
// 	algorithmDiv->activateable = false;
// 	element->addChild(algorithmDiv);
//
// 	for (int i = 0; i < 4; ++i){
// 		button = new UIElement();
// 		button->mesh = new Square();
// 		button->texture = TexMan.LoadTexture("img/80Gray50Alpha.png");
// 		button->alignmentX = 0.5f;
// 		button->alignmentY = 0.9f - 0.16f * i;
// 		button->sizeRatioX = 0.95f;
// 		button->sizeRatioY = 0.15f;
// 		button->scalable = true;
// 		button->activateable = true;
// 		button->onActivate = SetAlgorithm;
// 		button->SetName("mapButton");
// 		button->text = new char[MAX_PATH];
// 		switch(i){
// 			case 0:
// 				strcpy(button->text, "AStar");
// 				break;
// 			case 1:
// 				strcpy(button->text, "BreadthFirst");
// 				break;
// 			case 2:
// 				strcpy(button->text, "DepthFirst");
// 				break;
// 			default:
// 				strcpy(button->text, "CustomAlgorithm");
// 		}
// 		algorithmDiv->addChild(button);
// 	}
// 	*/
// }
//
// /** Renders the whole UIElement structure.
// 	Enables custom perspective for the UI. */
// void PathfindingUI::Render(GraphicsState& graphics){
// 	/// Set UI Shader program
// 	graphics.activeShader = Graphics.shadeMan.GetShaderProgram("UI");
// 	if (graphics.activeShader->built)
// 		glUseProgram(graphics.activeShader->shaderProgram);
// 	else {
// 		glUseProgram(0);
// 		graphics.activeShader = 0;
// 		return;
// 	}
//
// 	/// Update projection matrix
// 	Matrix4f projection = Matrix4f();
// 	projection.LoadIdentity();
// 	projection.InitOrthoProjectionMatrix(0, (float)width, 0, (float)height, -1.0f, -100.0f);
// //	projection.InitProjectionMatrix(-1000, 1000, -500, 500, -1, -100);
// 	projection.translate(0, 0, 1.0f);
//
// 	/// Just testing that the matrix is set correctly..
// 	Vector4f point = Vector4f((float)width, (float)height, 0, 1.0f);
// 	point = projection * point;
//
// 	point = Vector4f(width/2.0f, height/2.0f, 0, 1.0f);
// 	point = projection * point;
//
// 	point = Vector4f(0, 0, 0, 1.0f);
// 	point = projection * point;
//
// //	point = Vector4f(width/3, height*3/2, 0, 1);
// //	point = projection * point;
//
// 	// Load projection matrix into shader
// 	graphics.activeShader->uniformProjectionMatrix = glGetUniformLocation(graphics.activeShader->shaderProgram, "projectionMatrix");
// 	graphics.activeShader->uniformModelMatrix = glGetUniformLocation(graphics.activeShader->shaderProgram, "modelMatrix");
// 	glUniformMatrix4fv(graphics.activeShader->uniformProjectionMatrix, 1, false, projection.getPointer());
//
// 	graphics.modelMatrixF = graphics.modelMatrixD.LoadIdentity();
//
// 	// Enable alpha-blendinggg!
// 	glEnable(GL_BLEND);
//
// 	/// Just render for now
// 	element->render(graphics);
//
// 	/// Set to default shader program again
// 	glUseProgram(0);
// }
//
