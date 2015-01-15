/// Emil Hedemalm
/// 2014-09-15
/// Class for handling 2D-based games, which usually include constraints in X and Y, and uses a few layers of Z in order to render things properly.

#include "Game2D.h"

Game2D::Game2D(String name)
	: Game(name)
{
	z = 0;
}

Game2D::~Game2D()
{

}
