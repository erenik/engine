/// Emil Hedemalm
/// 2014-09-15
/// Class for handling 2D-based games, which usually include constraints in X and Y, and uses a few layers of Z in order to render things properly.

#include "Game.h"

class Game2D : public Game 
{
public:
	Game2D(String name);
	virtual ~Game2D();

	virtual void SetZ(float newZ) = 0;

	/// Main Z used for the game. Deviations up to a few units is usually acceptable.
	float z;

};











