// Emil Hedemalm
// 2013-07-09

// An RPG-based class/struct

#ifndef BATTLE_ACTION_H
#define BATTLE_ACTION_H

#include <String/AEString.h>
#include "Effect.h"
struct BattleState;

/// For sides, specifies which kind of battlers may be affected, no matter how the spell will be cast or distributed.
namespace AffectedSide {
	enum {
		BAD_SIDE,
		ALLIES,
		ENEMIES,
		ALL,
		ENVIRONMENT,
		AFFECTED_SIDES,
	};
};

/// For target filtering
namespace TargetFilter 
{ enum {
    NULL_TARGET,
	/// The caster
	SELF, 
	/// For selecting a battler target.
    ALLY,
    ENEMY,
    RANDOM,
    ALLIES,
    ENEMIES,
    ALL,
	/// For selecting a target based on the map
	POINT,
    TARGET_FILTERS,
};};

/// Shape of the spell. This affects to some degree how the animation will be scaled, rotated or otherwise..?
namespace SpellShape 
{ enum {
	POINT,
	CIRCLE,
	SPHERE,
	LINE,
	CONE,
	GLOBAL, 
};};

class BattleAction {
    friend class BattleActionLibrary;
public:
    BattleAction();
	/// Virtual destructor so subclasses are handled correctly.
	virtual ~BattleAction();

	/// Wosh.
    bool LoadFromFile(String source);
	/// Sets affected sides and targetting filter by string.
	int SetTargetFilterByString(String s);

	/// Processing functions
    virtual void OnBegin(BattleState & battleState);
    /// Should return true once the action (including animation, sound etc.) has been finished.
    virtual bool Process(BattleState & battleState);
    virtual void OnEnd(BattleState & battleState);
    /// If it should pause battle while it's processing (typical turn-based combat).
    bool PausesBattleProcessing();



    /// Variables!
    /// Name is good.
    String name;
    /// Category it belongs to, for dividing and auto-generating menus.
    String categoryName;
    struct BattleActionCategory * category;
	/// Effects applied by performing this action.
	List<Effect*> effects;
    /// Should be unique, assigned by the battle action library when bound to it for eased usage.
    int actionID;
    /// What is this for?
    bool pausesBattle;
    /// How to handle targetting.
    int targetFilter;

	/// Duration in milliseconds
	int duration;

    List<String> onBegin;
    List<String> onFrame;
    List<String> onEnd;

    /// For debugging...
    String className;
};

struct BattleActionCategory{
    BattleActionCategory();
	/// Add an action. Might add sorting later into here.
	void Add(BattleAction * action);
    List<BattleAction*> actions;
    String name;
    /// For special-cases and eased grouping into UI-systems. This essentially makes the category link to an action by the same name.
    bool isAction;
};

#define BALib (*BattleActionLibrary::Instance())

class BattleActionLibrary {
    BattleActionLibrary();
    ~BattleActionLibrary();
    static BattleActionLibrary * bal;
public:
    static void Allocate();
    static void Deallocate();
    static BattleActionLibrary * Instance();
    bool LoadFromDirectory(String directory);
    bool Add(BattleAction * ba, String intoCategory);
    bool CreateCategory(String newCategoryName);

	BattleAction * Get(String byName);
	/// Fetches list by strings
	List<BattleAction*> GetBattleActions(List<String> byNames);

	BattleActionCategory * GetCategory(String byName);
    int TotalsActions() const { return battleActions.Size(); };
    void PrintAll() const;
private:
    List<BattleAction*> battleActions;
    List<BattleActionCategory*> categories;
};

#endif
