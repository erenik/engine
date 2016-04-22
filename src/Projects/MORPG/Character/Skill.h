
/// Enum of all skills. Details are in separate Skill_ClassName.h 
enum {
	CLASS_1_SKILLS = 100,
	CLASS_2_SKILLS = 200,
	CLASS_3_SKILLS = 300,
	CLASS_4_SKILLS = 400,
	CLASS_5_SKILLS = 500,
	CLASS_6_SKILLS = 600,
	CLASS_7_SKILLS = 700,
	CLASS_8_SKILLS = 800,
	CLASS_9_SKILLS = 900,
	CLASS_10_SKILLS = 1000,
	CLASS_11_SKILLS = 1100,
	CLASS_12_SKILLS = 1200,
	CLASS_15_SKILLS = 1500,
	
	// 0 to 99 are classless skills
	FIGHTER_SKILLS = CLASS_1_SKILLS,
	ACOLYTE_SKILLS = CLASS_2_SKILLS,
	MONK_SKILLS = CLASS_3_SKILLS,
	RANGER_SKILLS = CLASS_4_SKILLS, 
	WIZARD_SKILLS = CLASS_5_SKILLS, 
	DUELIST_SKILLS = CLASS_6_SKILLS,
	ROGUE_SKILLS = CLASS_7_SKILLS, 
	HOLY_PROTECTOR_SKUKKS = CLASS_8_SKILLS,
	SLAYER_SKILLS = CLASS_9_SKILLS,
	DEATHS_DISCIPLE_SKILLS = CLASS_10_SKILLS,
	MAX_SKILLS = CLASS_15_SKILLS
};

/// Returns description of skill.
String GetSkillDescription(int i);
char GetSkillRequiredClass(int i);
char GetSkillRequiredClassLevel(int i);
/// Queries if activatable - at all 
bool IsActivatable(int i);
/// If cooldown is ready.
bool IsReady(int i, Character & character);
/// Query to ready skill i, or start casting it for magicks. Returns true if ready.
bool Ready(int i, Character & character);
/// Starts execution of the skill, animations, dealing damage, etc. Returns true if it started successfully. False if bad/failed request.
bool Start(int i, Character & character);
/// For those skills taking time. Returns true when it should go to End.
bool OnFrame(int i, Character & character);
bool End(int i, Character & character);




