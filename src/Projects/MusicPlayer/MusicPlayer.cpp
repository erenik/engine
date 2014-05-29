/// Emil Hedemalm
/// 2014-02-23
/// Main state for our music-player.

#include "MusicPlayer.h"
#include "StateManager.h"
#include "GameStates/GameStates.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "OS/WindowManager.h"
#include "Input/InputManager.h"
#include "Audio/TrackManager.h"
#include "Audio/Tracks/Track.h"
#include "Graphics/Messages/GMUI.h"
#include "UI/UIButtons.h"
#include "Message/Message.h"
#include "System/PreferencesManager.h"
#include "Audio/AudioManager.h"
#include "Message/VectorMessage.h"
#include "OS/Sleep.h"

#include "Windows.h"
#include "MMSystem.h"
#include "Mmdeviceapi.h"

const String applicationName = "MusicPlayer";

void SetApplicationDefaults()
{
	// Stuff.
	TextFont::defaultFontSource = "font3";
	UIElement::defaultTextureSource = "black";
}

void RegisterStates(){
	// Register our states. 
	// We'll only be using one state in the music-player
	StateMan.RegisterState(new MusicPlayer());
	StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_MUSIC_PLAYER));

}

MusicPlayer::MusicPlayer()
{
	name = "MusicPlayerState";
	id = GameStateID::GAME_STATE_MUSIC_PLAYER;
	
	/// Time fade-out began
	fadeOutBeganTime = 0;

	/// Stats to tune fade-out for sleeping.
	fadeOutBeginVolume = 0;
	fadeOutStartTime = 0;
	// 10 second default.
	fadeOutDuration = 10000;

	/// States to tune fade-in for waking.
	fadeInEndVolume = 1.0f;
	fadeInStartTime = 0;
	// 10 second default.
	fadeInDuration = 10000;

	AudioMan.SetMasterVolume(1.0f);
}

/// Virtual destructor to discard everything appropriately.
MusicPlayer::~MusicPlayer(){}
/// Function when entering this state, providing a pointer to the previous StateMan.
void MusicPlayer::OnEnter(GameState * previousState)
{
	Graphics.QueueMessage(new GMSetOverlay(""));
	Graphics.QueueMessage(new GMSetUI(ui));
	Graphics.outOfFocusSleepTime = 100;
	Graphics.sleepTime = 5;
	/// Disable real-time rendering where possible.
	Graphics.renderOnQuery = true;
	/// Disable background shit.
	Graphics.renderGrid = false;
	Graphics.renderFPS = false;
	/// Enable input keyboard focus.
	Input.ForceNavigateUI(true);
	searchString = "";

	/// Load preferences
	bool loaded = Preferences.Load();
	if (!loaded)
		return;
	// Save all tracks into preferences for this application, and save it to file.
	List<Track*> tracks = TrackMan.GetTracks();
	String tracksString;
	bool success = Preferences.GetString("KnownMedia", &tracksString);
	if (success){
		List<String> trackStrings = tracksString.Tokenize(";");
		for (int i = 0; i < trackStrings.Size(); ++i)
		{
			String trackData = trackStrings[i];
			List<String> trackDataTokenized = trackData.Tokenize(">");
			String trackName = trackDataTokenized[0];
			String trackSource = trackDataTokenized[1];
			Track * track = TrackMan.CreateTrack(trackName, trackSource);
		}
		Preferences.SetString("KnownMedia", tracksString);
	}
	OnTracksUpdated();	
}

/// Main processing function, using provided time since last frame.
void MusicPlayer::Process(float time)
{
//	std::cout<<"\nHello music player.";
	Sleep(100);
	/// Sleep more if not really active.
	if (!WindowMan.InFocus())
		Sleep(400);
	long long cTime = Timer::GetCurrentTimeMs();
	// Fade-out.
	if (fadeOutStartTime > 0)
	{
		/// Initial end time calculation
		if (fadeOutEndTime == 0)
		{	
			fadeOutBeginVolume = AudioMan.MasterVolume();
			fadeOutEndTime = fadeOutStartTime + fadeOutDuration;
		}
		// Wait before work.
		else if (fadeOutStartTime > cTime)
		{
		}
		// Actual fade-work.
		else if (fadeOutStartTime < cTime && fadeOutEndTime > cTime)
		{	
			float ratio = (fadeOutEndTime - cTime);
			ratio /= fadeOutDuration;
			float newMVol = fadeOutBeginVolume * ratio;
			AudioMan.SetMasterVolume(newMVol);
			OnVolumeUpdated();
			
			
			static float lastDecrease = 100;
			if (lastDecrease > newMVol + 0.02f)
			{
				keybd_event(VK_VOLUME_DOWN, 0x5B, 0, 0);
				keybd_event(VK_VOLUME_DOWN, 0x5B, KEYEVENTF_KEYUP, 0);
				lastDecrease = newMVol;
			}
		}	
		// end and cleanup.
		else if (fadeOutEndTime < cTime){
			// Stahp
			fadeOutStartTime = fadeOutEndTime = 0;
			AudioMan.SetMasterVolume(0);
			TrackMan.Pause();
			OnVolumeUpdated();
		}
	}
	// Fade-in.
	else if (fadeInStartTime > 0)
	{
		if (fadeInEndTime == 0)
		{
			TrackMan.Resume();		
			fadeInEndTime = fadeInStartTime + fadeInDuration;
		}
		else if (fadeInStartTime > cTime){
		}
		else if (fadeInStartTime < cTime && fadeInEndTime > cTime)
		{
			float ratio = (fadeInEndTime - cTime);
			ratio /= fadeInDuration;
			float newMVol = 1 - 1.0f * ratio;
			Clamp(newMVol, 0, 1.0f);
			AudioMan.SetMasterVolume(newMVol);
			OnVolumeUpdated();
			static float lastDecrease = 0;
			if (lastDecrease < newMVol - 0.02f)
			{
				keybd_event(VK_VOLUME_UP, 0x5B, 0, 0);
				keybd_event(VK_VOLUME_UP, 0x5B, KEYEVENTF_KEYUP, 0);
				lastDecrease = newMVol;
			}

		}
		else if (fadeInEndTime && fadeInEndTime < cTime){
			fadeInStartTime = fadeInEndTime = 0;
			AudioMan.SetMasterVolume(1.0f);
			OnVolumeUpdated();
		}
	}

	String fadeInfoString;

	/// Write seconds to fade-out begins.
	long long secondsLeft = fadeOutStartTime - cTime;
	secondsLeft /= 1000;
	if (secondsLeft > 0)
		fadeInfoString += String::ToString((int)secondsLeft)+" seconds til fade-out starts. ";
	else if (AbsoluteValue(secondsLeft) * 1000 < fadeOutDuration){
		fadeInfoString += "Fading out " +String::ToString((int)(fadeOutDuration - AbsoluteValue(secondsLeft) * 1000) / 1000)+" seconds. ";
	}
	
	/// Write seconds to fade-out begins.
	secondsLeft = fadeInStartTime - cTime;
	secondsLeft /= 1000;
	if (secondsLeft > 0)
		fadeInfoString += String::ToString((int)secondsLeft)+" seconds til fade-in starts. ";
	else if (AbsoluteValue(secondsLeft) * 1000 < fadeInDuration){
		fadeInfoString += "Fading in " +String::ToString((int)(fadeInDuration - AbsoluteValue(secondsLeft) * 1000) / 1000)+" seconds. ";
	}

	if (fadeInfoString.Length())
		Graphics.QueueMessage(new GMSetUIs("FadeInfo", GMUI::TEXT, fadeInfoString));

}

/// Function when leaving this state, providing a pointer to the next StateMan.
void MusicPlayer::OnExit(GameState * nextState)
{
	// Save all tracks into preferences for this application, and save it to file.
	List<Track*> tracks = TrackMan.GetTracks();
	String tracksString;
	for (int i = 0; i < tracks.Size(); ++i)
	{
		Track * track = tracks[i];
		String trackName = track->name;
		tracksString += track->name+">"+track->source+";";
	}
	Preferences.SetString("KnownMedia", tracksString);
	bool success = Preferences.Save();
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void MusicPlayer::ProcessMessage(Message * message)
{
	switch(message->type){
		case MessageType::FLOAT_MESSAGE:
		{
			FloatMessage * fm = (FloatMessage*) message;
			String msg = message->msg;
			float hours = fm->value;
			long long hoursInMs = hours * 60 * 60;
			hoursInMs *= 1000;
			long long cTime = Timer::GetCurrentTimeMs();
			if (msg == "SetFadeInTime")
			{
				fadeInStartTime = cTime + hoursInMs;
				fadeInEndTime = 0;
			}
			else if (msg == "SetFadeOutTime")
			{
				fadeOutStartTime = cTime + hoursInMs;
				fadeOutEndTime = 0;
			}
			else if (msg == "SetFadeDuration")
			{
				fadeOutDuration = fadeInDuration = hoursInMs;
			}
		}
		case MessageType::STRING:
		{
			String msg = message->msg;
			if (msg == "PlayTrack(this)"){
				String trackName = message->element->text;
				Track * track = TrackMan.PlayTrackByName(trackName);
				if (track)
					track->Loop(true);
			}
			else if (msg.Contains("TrackPlayed:")){
				String trackName = msg.Tokenize(":")[1];
				Graphics.QueueMessage(new GMSetUIs("CurrentTrackName", GMUI::TEXT, trackName)); 
			}
			else if (msg == "OnReloadUI"){
				OnTracksUpdated();
				OnVolumeUpdated();
			}
			else if (msg == "ResetVolume"){
				AudioMan.SetMasterVolume(1.0f);

				long long cTime = Timer::GetCurrentTimeMs();
			}
		
		}
	}
}

/// Creates the user interface for this state
void MusicPlayer::CreateUserInterface()
{
	// Is the ui already deleted here?
	if (ui)
		delete ui;
	ui = new UserInterface();
	bool success = ui->Load("gui/MusicPlayer/Main.gui");
}

/// For handling drag-and-drop files.
void MusicPlayer::HandleDADFiles(List<String> & files)
{
	/// Check them.
	for (int i = 0; i < files.Size(); ++i)
	{
		String filePath = files[i];
		if (filePath.Type() == String::WIDE_CHAR)
			filePath.ConvertToChar();
		/// Only accept OGG for now.
		if (!FilePath::FileEnding(filePath).Contains("ogg"))
			continue;
		/// 
		String fileName = FilePath::GetFileName(filePath);
		/// Check that an identical copy doesn't already exist.
		if (TrackMan.GetTrackBySource(filePath))
			continue;
		/// Add to library
		TrackMan.CreateTrack(fileName, filePath);

	}
	OnTracksUpdated();
}

/// Update volume label
void MusicPlayer::OnVolumeUpdated()
{
	float masterVol = AudioMan.MasterVolume();
	Graphics.QueueMessage(new GMSetUIs("MasterVolume", GMUI::TEXT, String::ToString(masterVol, 5)));
}

/// To update UI
void MusicPlayer::OnTracksUpdated()
{
	Graphics.QueueMessage(new GMClearUI("KnownMedia"));	
	List<Track*> tracks = TrackMan.GetTracks();
	for (int i = 0; i < tracks.Size(); ++i)
	{
		Track * track = tracks[i];
		String trackName = track->name;
		UIButton * trackButton = new UIButton(trackName);
		trackButton->sizeRatioY = 0.1f;
		trackButton->activationMessage = "PlayTrack(this)";
		trackButton->textureSource = UIElement::defaultTextureSource;
		Graphics.QueueMessage(new GMAddUI(trackButton, "KnownMedia"));
	}
}
