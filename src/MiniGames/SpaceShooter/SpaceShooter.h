/// Emil Hedemalm
/// 2014-07-28
/** Class that weaves together all elements of a Space-shooter game.
	Encapsulation like this makes it easy to switch in/out various small/mini-games as pleased.
*/

#include "MathLib.h"
#include "Game/Game2D.h"
#include "Random/Random.h"
#include "Game/SpaceShooter/SpaceShooterWeaponType.h"

class SpaceShooterPlayerProperty;
class SpaceShooterPowerupProperty;
class SpaceShooterProjectileProperty;
class Entity;
class Message;
class SpaceShooterIntegrator;
class SpaceShooterCD;
class SpaceShooterCR;
class Sparks;

class SpaceShooter : public Game2D
{
public:
	SpaceShooter();
	~SpaceShooter();

	/// Performs one-time initialization tasks. This should include initial allocation and initialization.
	virtual void Initialize();
	/// Resets the entire game. Similar to a hardware reset on old console games.
	virtual void Reset();

	virtual void ProcessMessage(Message * message);
	/// Call on a per-frame basis.
	virtual void Process();
	/// Fetches all entities concerning this game.
	List<Entity*> GetEntities();
	/// Fetches all static entities.
	virtual List<Entity*> StaticEntities();


	/// If set (with true), will enabled tracking/movement of the player with the mouse.
	void UseMouseInput(bool useItOrNot);

	// Call to re-create the playing field as it started out.
	void SetupPlayingField();

	/// Creates a new projectile entity, setting up model and scale appropriately.
	Entity * NewProjectile(SpaceShooterWeaponType weaponType, ConstVec3fr atPosition, ConstVec3fr withInitialVelocity);
	/// Creates a new explision entity! See ExplosionTypes enum in SpaceShooterExplosionProperty.h.
	Entity * NewExplosion(ConstVec3fr atPosition, int type);

	/// Is it outside the frame?
	bool IsPositionOutsideFrame(ConstVec3fr pos);

	/// Will.. remove from rendering/physics relevant entities and set the pause-state.
	void SetPause(bool pause);

	/// Determines what enemies are spawned, and where.
	void SetLevel(int l);
	// For setting main plane for the game.
	void SetShipScale(float scale);
	virtual void SetZ(float newZ);

	/// 1.0 or -1.0 only.
	void SetFlipX(float newX);


	/// For manual player-control. Usually only Y will be taken into consideration.
	void SetPlayerPosition(ConstVec3fr pos);

	/// Sets up physics integrator, etc. as needed.
	void SetupPhysics();
	/// Sets up stuff specific for the entities in this little game.
	void SetupPhysics(List<Entity*> forEntities);

	/// Sets up collision filters appropriately. 2 sides!
	void SetupCollisionFilter(List<Entity*> entities, bool allied = false);
	void SetProjectileScale(List<Entity*> entity);

	/// Spawns enemies for a level. This will spawn all enemies, far to the right. 
	void SpawnEnemies(int level);
	
	/// Sets up the scrolling background as appropriate.
	void SetupBackground(int forLevel);

	/// Update text on both entities displaying the scores.
	void OnScoreUpdated();
	/// o.o
	void UpdatePlayerHP();
	/// Check if level is completed -> Spawn 'em again.
	void OnPlayerDestroyed(Entity * player);
	
	/// o.o
//	bool ;

	/// determines type and amount of enemies?
	int level;
	
		// All entities.
//	List<Entity*> entities;// <- really good?
	List<Entity*> players;
	List<Entity*> enemies;
	List<Entity*> projectiles, powerups, explosions;
	List<Entity*> scoreEntities;

	Entity * player1;

	/// Value of 1.0 or -1.0. To be multiplied to ALL X-positions and velicities in order to flip the entire game as wanted o.o
	float flipX;

	/// Absolute co-ordinates of the game field.
	float top, bottom, left, right;

	/// If the player only can steer in Y. Default false.
	bool yOnly;

	// For setting up the playing field.
	// Spacing between the left side and the player.
	float leftSpacing;		

	Entity * scoreEntity; // , * score2Entity;
	Entity * hpEntity;

	int score;

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
	

	
	SpaceShooterIntegrator * spaceShooterIntegrator;
	SpaceShooterCR * spaceShooterCR;
	SpaceShooterCD * spaceShooterCD;

	/// Particle system to extend on explosions effects.
	Sparks * sparks;

private:
};



