// Emil Hedemalm
// 2013-07-09

// An RPG-based class/struct
#include "BattleAction.h"
#include <fstream>
#include <cstring>

BattleAction::BattleAction(){
    pausesBattle = false;
	category = NULL;
};

/// Virtual destructor so subclasses are handled correctly.
BattleAction::~BattleAction()
{

}


bool BattleAction::LoadFromFile(String source){
    /// Do magical stuff.
    std::fstream file;
	file.open(source.c_str(), std::ios_base::in);
	if (!file.is_open()){
		std::cout<<"\nERROR: Unable to open file stream to "<<source;
		file.close();
		return false;
	}
	int start  = (int) file.tellg();
	file.seekg( 0, std::ios::end );
	int fileSize = (int) file.tellg();
	char * data = new char [fileSize];
	memset(data, 0, fileSize);
	file.seekg( 0, std::ios::beg);
	file.read((char*) data, fileSize);
	file.close();
	String fileContents(data);
	delete[] data; data = NULL;
	List<String> lines = fileContents.GetLines();
	int tempLineNumber;
	for (int i = 0; i < lines.Size(); ++i){
		String & line = lines[i];
		// Try load the battler from the relative directory.
		if (line.Contains("//"))
			continue;
		List<String> tokens = line.Tokenize(" \t");
		if (tokens.Size() == 0)
			continue;
		String token = tokens[0];
		String token2;
		if (tokens.Size() >= 2)
			token2 = tokens[1];
		line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		std::cout<<"\nLine "<<i<<": "<<line;
		if (line.Length() < 2)
			continue;
		else if (line.Contains("name")){
			name = token2;
		}
		else if (token == "category"){
			categoryName = token2;
		}
		else if (token == "duration"){
			duration = token2.ParseFloat()*1000;
		}
		else if (token == "target"){
		    token2.SetComparisonMode(String::NOT_CASE_SENSITIVE);
            if (token2 == "Enemy")
                targetFilter = TargetFilter::ENEMY;
            else if (token2 == "Player")
                targetFilter = TargetFilter::ALLY;
            else if (token2 == "Enemies")
                targetFilter = TargetFilter::ENEMIES;
            else if (token2 == "Players")
                targetFilter = TargetFilter::ALLIES;
            else if (token2 == "Enemy")
                targetFilter = TargetFilter::ENEMY;
		}
		else if (token == "OnBegin"){
            tempLineNumber = i+1;
		}
		else if (token == "OnBeginEnd"){
		    for (int j = tempLineNumber; j < i && j < lines.Size(); ++j){
                onBegin.Add(lines[j]);
		    }
		    tempLineNumber = -1;
		}
		else if (token == "OnFrame"){
			tempLineNumber = i+1;
		}
		else if (token == "OnFrameEnd"){
		    for (int j = tempLineNumber; j < i && j < lines.Size(); ++j){
                onFrame.Add(lines[j]);
		    }
		    tempLineNumber = -1;
		}
		else if (token == "OnEnd"){
            tempLineNumber = i+1;
		}
		else if (token == "Done"){
			if (tempLineNumber == -1)
                continue;
            for (int j = tempLineNumber; j < i; ++j){
                onEnd.Add(lines[j]);
		    }
		}
	}
	/// Remove all comment-lines here, ye.
	String::RemoveComments(onBegin, "//", "/*","*/");
	String::RemoveComments(onFrame, "//", "/*","*/");
	String::RemoveComments(onEnd, "//", "/*","*/");
	return true;
};

/// Fetches battle target by string.
int BattleAction::SetTargetFilterByString(String string)
{
	string.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (string.Contains("SELF"))
		targetFilter = TargetFilter::SELF;
	else if (string.Contains("ALLY"))
		targetFilter = TargetFilter::ALLY;
	else if (string.Contains("ENEMY"))
		targetFilter = TargetFilter::ENEMY;
	else if (string.Contains("RANDOM"))
		targetFilter = TargetFilter::RANDOM;
	else if (string.Contains("allies"))
		targetFilter = TargetFilter::ALLIES;
	else if (string.Contains("enemies"))
		targetFilter = TargetFilter::ENEMIES;
	else if (string.Contains("point"))
		targetFilter = TargetFilter::POINT;
	else if (string.Contains("All"))
		targetFilter = TargetFilter::ALL;
	return targetFilter;
}

void BattleAction::OnBegin(BattleState & battleState){
    std::cout<<"\nBattleAction::OnBegin called, subclass it yo.";
    assert(false);
}
/// Should return true once the action (including animation, sound etc.) has been finished.
bool BattleAction::Process(BattleState & battleState){
    std::cout<<"\nBattleAction::Process called, subclass it yo. Default will declare the action as complete straight away!";
	assert(false);
	return true; // Must return something here, or it won't compile D:
}
void BattleAction::OnEnd(BattleState & battleState){
    std::cout<<"\nBattleAction::OnEnd called, subclass it yo.";
    assert(false);
}
/// If it should pause battle while it's processing (typical turn-based combat).
bool BattleAction::PausesBattleProcessing() {
    return pausesBattle;
};

BattleActionCategory::BattleActionCategory(){
	isAction = false;
}

/// Add an action. Might add sorting later into here.
void BattleActionCategory::Add(BattleAction * action)
{
	if (actions.Exists(action)){
		std::cout<<"\nCategory already contains action. Skipping.";
		return;
	}
	actions.Add(action);
}

BattleActionLibrary * BattleActionLibrary::bal = NULL;
BattleActionLibrary::BattleActionLibrary(){}
BattleActionLibrary::~BattleActionLibrary(){
    CLEAR_AND_DELETE(categories);
    CLEAR_AND_DELETE(battleActions);
}
void BattleActionLibrary::Allocate(){
    assert(bal == NULL);
    bal = new BattleActionLibrary();
}
void BattleActionLibrary::Deallocate(){
    assert(bal);
    delete bal;
    bal = NULL;
}
BattleActionLibrary * BattleActionLibrary::Instance(){
    assert(bal);
    return bal;
}

/// Looks for a "Battlers.list" which should then specify the battler-files to load.
bool BattleActionLibrary::LoadFromDirectory(String dir){

	std::cout<<"\nBattleActionLibrary::LoadFromDirectory called.";
	String filePath = dir + "actions.list";
	std::fstream file;
	file.open(filePath.c_str(), std::ios_base::in);
	if (!file.is_open()){
		std::cout<<"\nERROR: Unable to open file stream to "<<filePath;
		file.close();
		return false;
	}
	int start  = (int) file.tellg();
	file.seekg( 0, std::ios::end );
	int fileSize = (int) file.tellg();
	char * data = new char [fileSize];
	memset(data, 0, fileSize);
	file.seekg( 0, std::ios::beg);
	file.read((char*) data, fileSize);
	file.close();
	String fileContents(data);
	delete[] data; data = NULL;
	List<String> lines = fileContents.GetLines();
	std::cout<<"\n"<<lines.Size()<<" lines read.";
	for (int i = 0; i < lines.Size(); ++i){
		String & line = lines[i];
		// Try load the battler from the relative directory.
		if (line.Contains("//"))
			continue;
		else if (line.Length() < 2)
			continue;
		BattleAction * ba = new BattleAction();
		String actionSource = dir + line;
		if (!actionSource.Contains(".ba"))
			actionSource += ".ba";
		bool result = ba->LoadFromFile(actionSource);
		if (!result){
			delete ba;
		}
		else {
			for (int b = 0; b < battleActions.Size(); ++b){
				String name = battleActions[b]->name;
				if (name == ba->name){
					std::cout<<"\nWARNING: Action with name "<<name<<" already exists! Deleting previous entry.";
					battleActions.Remove(battleActions[b]);
				}
			}
			battleActions.Add(ba);
		}
	}
	std::cout<<"\nBattleActions now "<<battleActions.Size();

	std::cout<<"\nRe-calculating action categories...";
    categories.ClearAndDelete();
    for (int i = 0; i < battleActions.Size(); ++i){
        BattleAction * ba = battleActions[i];
        bool categoryFound = false;
        for (int c = 0; c < categories.Size(); ++c){
            BattleActionCategory * bac= categories[c];
            if (ba->categoryName == bac->name){
                /// Add it.
				bac->Add(ba);
                ba->category = bac;
                categoryFound = true;
                break;
            }
        }
        /// Create category if it wasn't found earlier.
        if (!categoryFound){
            std::cout<<"\nCreating category "<<ba->categoryName<<"...";
            BattleActionCategory * bac = new BattleActionCategory();
			if (ba->categoryName == ba->name)
				bac->isAction = true;
            bac->name = ba->categoryName;
            categories.Add(bac);
            bac->actions.Add(ba);
            ba->category = bac;
        }
    }
	return true;
}


bool BattleActionLibrary::Add(BattleAction * ba, String intoCategory){
    BattleActionCategory * c = NULL;
    for (int i = 0; i < categories.Size(); ++i)
        if (categories[i]->name == intoCategory){
            c = categories[i];
            break;
        }
    if (c == NULL)
        return false;
    c->actions.Add(ba);
    return true;
}

bool BattleActionLibrary::CreateCategory(String newCategoryName){
    /*for (int i = 0; i < categories.Size(); ++i){
        if (categories[i]->name == newCategoryName)
            return false;
    }
    BAttleAc
    categories.Add(new BattleActionCategory(newCategoryName));
    */
	return false;
}
void BattleActionLibrary::PrintAll() const{
    std::cout<<"\nPrinting all battle actions!";
    for (int i = 0; i < categories.Size(); ++i){
        std::cout<<"\nCategory "<<i<<": "<<categories[i]->name;
        BattleActionCategory * bac = categories[i];
        for (int j = 0; j < bac->actions.Size(); ++j){
			std::cout<<"\nBattle Action "<<j<<": "<<bac->actions[j]->name;
        }
    }
}

BattleAction * BattleActionLibrary::Get(String byName){
    BattleAction * ba = NULL;
    for (int i = 0; i < battleActions.Size(); ++i){
        ba = battleActions[i];
        if (ba->name == byName)
            return ba;
    }
    std::cout<<"\nWARNING: Could not find any battle action by given name: "<<byName;
	return NULL;
}
/// Fetches list by strings
List<BattleAction*> BattleActionLibrary::GetBattleActions(List<String> byNames)
{
	List<BattleAction*> actions;
	BattleAction * ba = NULL;
	for (int a = 0; a < byNames.Size(); ++a){
		String byName = byNames[a];
		for (int i = 0; i < battleActions.Size(); ++i){
			ba = battleActions[i];
			if (ba->name == byName)
			{
				actions.Add(ba);
			}
		}
	}
	return actions;
}

BattleActionCategory * BattleActionLibrary::GetCategory(String byName){
	for (int i = 0; i < categories.Size(); ++i)
		if (categories[i]->name == byName)
			return categories[i];
	return NULL;
}
