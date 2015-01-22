/// Emil Hedemalm
/// 2015-01-21
/// Ship.

#include "SpaceShooter2D.h"
#include "Ship.h"

#include "File/File.h"
#include "String/StringUtil.h"



Ship::Ship()
{
	spawned = false;
	entity = NULL;
	ai = true;
	allied = false;
	maxHitPoints = hitPoints = 10;
	canMove = false;
	canShoot = true;
	spawnInvulnerability = false;
	hasShield = false;
	shieldValue = maxShieldValue = 10;
}

Ship::~Ship()
{
	
}

void Ship::Damage(int amount, bool ignoreShield)
{
	if (spawnInvulnerability)
	{
	//	std::cout<<"\nInvulnnnn!";
		return;
	}			
	if (hasShield && !ignoreShield)
	{
		shieldValue -= amount;
		amount = -shieldValue;
		if (shieldValue < 0 )
			shieldValue = 0;
		if (this->allied)
			spaceShooter->UpdatePlayerShield();
		if (amount < 0)
			return;
		shieldValue = 0;
	}
	hitPoints -= amount;
	if (this->allied)
		spaceShooter->UpdatePlayerHP();
	if (hitPoints <= 0)
		Destroy();
}

void Ship::Destroy()
{
	if (entity)
	{
		ShipProperty * sp = entity->GetProperty<ShipProperty>();
		if (sp)
			sp->sleeping = true;
		// Add a temporary emitter to the particle system to add some sparks to the collision
		SparksEmitter * tmpEmitter = new SparksEmitter(entity->position);
		tmpEmitter->SetRatioRandomVelocity(1.0f);
		tmpEmitter->SetEmissionVelocity(4.5f);
		tmpEmitter->constantEmission = 750;
		tmpEmitter->instantaneous = true;
		tmpEmitter->SetScale(0.2f);
		tmpEmitter->SetParticleLifeTime(3.5f);
		tmpEmitter->SetColor(Vector4f(1.f, 0.5f, 0.1f, 1.f));
		Graphics.QueueMessage(new GMAttachParticleEmitter(tmpEmitter, sparks));

		MapMan.DeleteEntity(entity);
		spaceShooter->shipEntities.Remove(entity);
		entity = NULL;
		// Explosion?
	}
}


// Load ship-types.
bool Ship::LoadTypes(String file)
{
	List<String> lines = File::GetLines(file);
	if (lines.Size() == 0)
		return false;

	String separator;
	/// Column-names. Parse from the first line.
	List<String> columns;
	String firstLine = lines[0];
	// Keep empty strings or all will break.
	columns = TokenizeCSV(firstLine);

	// For each line after the first one, parse data.
	for (int j = 1; j < lines.Size(); ++j)
	{
		String & line = lines[j];
		// Keep empty strings or all will break.
		List<String> values = TokenizeCSV(line);
		for (int i = 0; i < values.Size(); ++i)
		{
			String v = values[i];
	//		std::cout<<"\nValue "<<i<<": "<<v;
		}
		// If not, now loop through the words, parsing them according to the column name.
		// First create the new spell to load the data into!
		Ship ship;
		for (int k = 0; k < values.Size(); ++k)
		{
			String column;
			bool error = false;
			/// In-case extra data is added beyond the columns defined above..?
			if (columns.Size() > k)
				column = columns[k];
			String value = values[k];
			if (value == "n/a")
				continue;
			column.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (column == "Name")
				ship.name = value;
			else if (column == "Type")
				ship.type = value;
			else if (column == "Can Move")
				ship.canMove = value.ParseBool();
			else if (column == "Can Shoot")
				ship.canShoot = value.ParseBool();
			else if (column == "Weapon Name")
			{
				List<String> weaponNames = value.Tokenize(",");
				// Fetch weapons?
				for (int i = 0; i < weaponNames.Size(); ++i)
				{
					String name = weaponNames[i];
					name.RemoveInitialWhitespaces();
					name.RemoveTrailingWhitespaces();
					Weapon weapon = Weapon::Get(name);
					ship.weapons.Add(weapon);
				}
			}
			else if (column == "Weapon Ammunition")
			{
				List<String> ammos = value.Tokenize(",");
				for (int i = 0; i < ammos.Size(); ++i)
				{
					Weapon & weapon = ship.weapons[i];
					String ammo = ammos[i];
					if (ammo == "inf")
						weapon.ammunition = -1;
					weapon.ammunition = ammo.ParseInt();
				}
			}
			else if (column == "Weapon Cooldown")
			{
				if (value.Contains("Burst"))
				{
					// PArse burst> Burst(rounds, delayBetweenShots, cooldown)
					Weapon & weapon = ship.weapons[0];
					List<String> toks = value.Tokenize("(),");
					weapon.burst = true;
					weapon.burstRounds = toks[1].ParseInt();
					weapon.burstRoundDelayMs = (int) (toks[2].ParseFloat() * 1000);
					weapon.cooldownMs = (int) (toks[3].ParseFloat() * 1000);
				}
				else 
				{
					List<String> cooldowns = value.Tokenize(",");
					if (cooldowns.Size() != ship.weapons.Size())
					{
						// Print error message
						std::cout<<"\nERROR: Ship \'"<<ship.name<<"\' trying to assign "<<cooldowns.Size()<<" weapon cooldowns to "<<ship.weapons.Size()<<" weapons."; 
						continue;
					}
					for (int i = 0; i < ship.weapons.Size(); ++i)
					{
						Weapon & weapon = ship.weapons[i];
						weapon.cooldownMs = (int) (cooldowns[i].ParseFloat() * 1000);
						int p = i;
					}
				}
			}
			else if (column == "Movement pattern")
				ship.movementPattern = value;
			else if (column == "Speed")
				ship.speed = value.ParseFloat() * 0.2f;
			else if (column == "Has Shield")
				ship.hasShield = value.ParseBool();
			else if (column == "Shield Value")
				ship.shieldValue = ship.maxShieldValue = value.ParseInt();
			else if (column == "Shield Regen Rate")
				ship.shieldRegenRate = value.ParseInt() / 1000.f;
			else if (column == "Hit points")
				ship.maxHitPoints = ship.hitPoints = value.ParseInt();
			else if (column == "Graphic model")
				ship.graphicModel = value;
		}
		// Check for pre-existing ship of same name, remove it if so.
		for (int i = 0; i < types.Size(); ++i)
		{
			Ship & type = types[i];
			if (type.name == ship.name)
			{
				types.RemoveIndex(i);
				--i;
			}
		}
		types.Add(ship);
	}
	return true;
}


/// Creates new ship of specified type.
Ship Ship::New(String shipByName)
{
	shipByName.Replace('_', ' ');
	for (int i = 0; i < types.Size(); ++i)
	{
		Ship & type = types[i];
		if (type.name == shipByName)
			return type;
	}
	// For now, just add a default one.
	std::cout<<"\nERROR: Couldn't find ship by name \'"<<shipByName<<"\'";
	return Ship();
}
