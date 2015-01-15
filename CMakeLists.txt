# GameEngine Main CMakeLists file.

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Create options
set(LIBCURL "OFF" CACHE BOOL "Whether to compile and link with LibCURL. Required for HTTP requests")
set(OPENCV "ON" CACHE BOOL "Whether to compile and link with OpenCV. Required for image processing.")
set(OPUS "ON" CACHE BOOL "Whether to compile and link with (Ogg/)Opus. Required for certain sound playback.")
set(VORBIS "ON" CACHE BOOL "Whether to compile and link with (Ogg/)Vorbis. Required for certain sound playback.")
set(OPENAL "ON" CACHE BOOL "Whether to compile and link with OpenAL soft. Required for sound playback.")
set(THEORA "ON" CACHE BOOL "Whether to compile and link with (Ogg/)Theora. Required for certain video playback.")


set(PROJECT_NAME "Engine")
		
# Create our projects
#set(tetrisFolders "Projects/Tetris")
#addProject("Tetris" tetrisFolders)

add_subdirectory(src)

#set(runeRPGFolders "Projects/RuneRPG")
#addProject("RuneRPG" runeRPGFolders)
# set(musicPlayerFolders "Projects/MusicPlayer")
# addProject("MusicPlayer" musicPlayerFolders)
# set (sideScrollerFolders "Projects/SideScroller")
# addProject("SideScroller" sideScrollerFolders)
# set(spaceRaceFolders "Projects/SpaceRace")
# addProject("SpaceRace" spaceRaceFolders)

# set(horrorFolders "Projects/Horror")
# addProject("Horror" horrorFolders)
# set(aircraftFolders "Projects/Aircraft")
# addProject("Aircraft" aircraftFolders)

# set(MORPGFolders "Projects/MORPG")
# addProject("MORPG" MORPGFolders)

# set(TIFSFolders "Projects/TIFS")
# addProject("TIFS" TIFSFolders)

# # Create our projects
# set(ipmFolders "Projects/IPM" "CV")
# addProject("IPM" ipmFolders)
