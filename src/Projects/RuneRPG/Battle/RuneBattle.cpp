/// Emil Hedemalm
/// 2014-06-20
/// Further dividing the source files...

#include "RuneBattle.h"
#include "File/File.h"

bool RuneBattle::Load(String fromSource)
{
	List<String> lines = File::GetLines(fromSource);
	if (lines.Size() == 0)
		return false;
	source = fromSource;
	// Default to loading all active players too.
	addCurrentPlayers = true;
	enum {
		NONE,
		LOAD_PLAYERS,
		LOAD_ENEMIES,
	};
	int loadingState = LOAD_ENEMIES;
	for (int i = 0; i < lines.Size(); ++i){
		String & line = lines[i];
		// Try load the battler from the relative directory.
		if (line.Contains("//"))
			continue;
		line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (line.Contains("name")){
			line.Remove("name");
			if (line.Contains("\"")){
				name = line.Tokenize("\"")[1];
			}
			else {
				line.RemoveInitialWhitespaces();
				name = line;
			}
		}
		else if (line.Contains("ActivePlayers"))
			addCurrentPlayers = true;
		else if (line.Contains("players"))
			loadingState = LOAD_PLAYERS;
		else if (line.Contains("enemies"))
			loadingState = LOAD_ENEMIES;
		else if (line.Length() < 3)
			continue;
		else if (loadingState == LOAD_PLAYERS)
			playerNames.Add(line);
		else if (loadingState == LOAD_ENEMIES)
			enemyNames.Add(line);
	}
	source = source;
	return true;
}
