// Emil Hedemalm
// Semi-old stuff

#include "String/AEString.h"
#include "../Fonts/Font.h"
#include "../GraphicsManager.h"

void GraphicsManager::RenderAI(){
	if (!renderAI)
		return;

	/*
	int characters = AIWorld.Characters();
	for (int i = 0; i < characters; ++i){
		Entity * e = AIWorld.GetCharacter(i);
		if (!e)
			continue;
		String name = e->name;
		if (!name)
			continue;

		/// Get the character's transform
		graphicsState.modelMatrixF = e->transformationMatrix;
		/// Translate it up ever so slghtly ^^ (radius maybe?)
		graphicsState.modelMatrixF.translate(0, e->radius, 0);
		SetShaderProgram(0);
		glColor4f(0.0, 1.0, 1.0,1.0);
		/// Test-render text ^^
		defaultFont->RenderText(name, graphicsState);

	}
	if (characters == 0)
		/// Test-render text ^^
		;//defaultFont->RenderText("There are no more characters...", graphicsState);
	*/
}