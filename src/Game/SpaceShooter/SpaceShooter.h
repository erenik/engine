/// Emil Hedemalm
/// 2014-07-28
/** Class that weaves together all elements of a Space-shooter game.
	Encapsulation like this makes it easy to switch in/out various small/mini-games as pleased.
*/

#include "MathLib.h"
#include "Game/Game.h"
#include "Random/Random.h"
#include "Game/SpaceShooter/SpaceShooterWeaponType.h"

class SpaceShooterPlayerProperty;
class SpaceShooterPowerupProperty;
class SpaceShooterProjectileProperty;
class Entity;
class Message;

class SpaceShooter : public Game
{
public:
	SpaceShooter();
	~SpaceShooter();


	virtual void ProcessMessage(Message * message);
	/// Call on a per-frame basis.
	virtual void Process();
	/// Fetches all entities concerning this game.
	List<Entity*> GetEntities();


	/// If set (with true), will enabled tracking/movement of the player with the mouse.
	void UseMouseInput(bool useItOrNot);

	void Reset();
	// Call to re-create the playing field as it started out.
	void SetupPlayingField();

	/// Creates a new projectile entity, setting up model and scale appropriately.
	Entity * NewProjectile(SpaceShooterWeaponType weaponType, Vector3f atPosition);

	Entity * NewExplosion(Vector3f atPosition);

	/// Is it outside the frame?
	bool IsPositionOutsideFrame(Vector3f pos);

	/// Will.. remove from rendering/physics relevant entities and set the pause-state.
	void SetPause(bool pause);

	// For setting main plane for the game.
	void SetZ(float z);
	void SetFrameSize(Vector2i size);
	void SetShipScale(float scale);

	/// For manual player-control. Usually only Y will be taken into consideration.
	void SetPlayerPosition(Vector3f pos);

	/// Sets up physics integrator, etc. as needed.
	void SetupPhysics();
	/// Sets up stuff specific for the entities in this little game.
	void SetupPhysics(List<Entity*> forEntities);

	/// Sets up collision filters appropriately. 2 sides!
	void SetupCollisionFilter(List<Entity*> entities, bool allied = false);
	void SetProjectileScale(List<Entity*> entity);

	/// Spawns enemies for a level. This will spawn all enemies, far to the right. 
	void SpawnEnemies(int level = 0);

	/// Sets up the scrolling background as appropriate.
	void SetupBackground(int forLevel);

	// Update text on both entities displaying the scores.
	void OnScoreUpdated();
	/// Check if level is completed -> Spawn 'em again.
	void OnPlayerDestroyed(Entity * player);

	/// determines type and amount of enemies?
	int level;

	int gameState;
	enum 
	{
		SETTING_UP_PLAYFIELD,
		GAME_BEGUN,
	};
	
		// All entities.
//	List<Entity*> entities;// <- really good?
	List<Entity*> players;
	List<Entity*> enemies;
	List<Entity*> projectiles, powerups, explosions;
	List<Entity*> scoreEntities;

	Entity * player1;

	/// Size of the playing field.
	Vector2f gameSize;

	/// Absolute co-ordinates of the game field.
	float top, bottom, left, right;

	/// If the player only can steer in Y. Default false.
	bool yOnly;

	// For setting up the playing field.
	// Spacing between the left side and the player.
	float leftSpacing;		

	Entity * score1Entity; // , * score2Entity;

	/// o-o
	List<SpaceShooterProjectileProperty*> ballProperties;
	SpaceShooterPlayerProperty * player1Properties;

	// random number generator.
	Random pongRand;

	float constantZ;
	float shipScale;
	float projectileScale;

	/// 1 << 4 
	int collisionCategoryPlayer;
	/// 1 << 5
	int collisionCategoryEnemy;

	/// Dead time in milli-seconds.
	bool playerDead;
	int deadTimeMs;
	Time lastFrame;
	

private:
};



