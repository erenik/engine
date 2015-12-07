/// Emil Hedemalm
/// 2015-02-05
/// Contains definitions and information about the threading setup of the current build/project.

#ifndef THREAD_SETUP_H
#define THREAD_SETUP_H

/// When using a single thread for all of graphics, physics, multimedia and audio processing.
#define GRAPHICS_PHYSICS_MM_AUDIO_THREAD


/// Set up some variables (macro-definitions) depending on what threading setup we want to use.
#ifdef GRAPHICS_PHYSICS_MM_AUDIO_THREAD
	// Some standard files used for logging.
	#define GRAPHICS_THREAD "log/GraphicsThread.txt"
	#define PHYSICS_THREAD "log/PhysicsLog.txt"
	#define AUDIO_THREAD "log/AudioLog.txt"
	#define MAIN_THREAD	"log/MainThreadLog.txt"

	#define GERRORS gErrors
	#define PERRORS pErrors
	#define AERRORS aErrors
	#define MERRORS mErrors

#endif // GRAPHICS_PHYSICS_MM_AUDIO_THREAD


#endif // THREAD_SETUP_H



