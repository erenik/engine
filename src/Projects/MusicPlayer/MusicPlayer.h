/// Emil Hedemalm
/// 2014-02-23
/// Main state for our music-player.

#include "GameStates/GameState.h"

class MusicPlayer : public GameState {
public:
	MusicPlayer();
	/// Virtual destructor to discard everything appropriately.
	virtual ~MusicPlayer();
	/// Function when entering this state, providing a pointer to the previous StateMan.
	virtual void OnEnter(GameState * previousState);
	/// Main processing function, using provided time since last frame.
	virtual void Process(float time);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	virtual void OnExit(GameState * nextState);
	
	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	/// Creates the user interface for this state
	virtual void CreateUserInterface();
	
	/// For handling drag-and-drop files.
	virtual void HandleDADFiles(List<String> & files);
private:
	/// Update volume label
	void OnVolumeUpdated();

	/// To update UI
	void OnTracksUpdated();
	String searchString;
	
	
	/// Time fade-out began
	long long fadeOutBeganTime;

	/// Stats to tune fade-out for sleeping.
	float fadeOutBeginVolume;
	long long fadeOutStartTime;
	long long fadeOutEndTime;
	int fadeOutDuration;

	/// States to tune fade-in for waking.
	float fadeInEndVolume;
	long long fadeInStartTime;
	long long fadeInEndTime;
	int fadeInDuration;

};



