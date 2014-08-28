/// Emil Hedemalm
/// 2014-08-28
/// An animation, which controls rendered entities on the screen as a spell is being animated, or controls particles effects or w/e is needed.

/// This also includes animation of the character! o.o .. or?
class BattleAnimation 
{
public:
	BattleAnimation();

	/// o.o
	void Process(int timeInMs);

	/// When it is over o-o
	bool isOver;
};


