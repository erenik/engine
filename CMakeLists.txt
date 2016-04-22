# GameEngine Main CMakeLists file.

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Create options
set(LIBCURL "OFF" CACHE BOOL "Whether to compile and link with LibCURL. Required for HTTP requests")
set(OPENCV "OFF" CACHE BOOL "Whether to compile and link with OpenCV. Required for image processing.")
set(OPENAL "ON" CACHE BOOL "Whether to compile and link with OpenAL soft. Required for sound playback.")
set(OPUS "OFF" CACHE BOOL "Whether to compile and link with (Ogg/)Opus. Required for certain sound playback.")
set(VORBIS "ON" CACHE BOOL "Whether to compile and link with (Ogg/)Vorbis. Required for certain sound playback.")
set(THEORA "OFF" CACHE BOOL "Whether to compile and link with (Ogg/)Theora. Required for certain video playback.")
set(LIBS_DIR "C:/libs" CACHE STRING "Root directory for additional dependencies (OpenGL, CV, AL, etc.)")
set(INSTALL_DEPENDENCIES "OFF" CACHE BOOL "Whether to automatically download and try set up dependencies or not.")
set(PROJ_SPEC_DIR "dirName" CACHE STRING "Project-specific directory. By default in the /engine/Projects/ folder, but may be absolute path too.")

set(PROJECT_NAME "Engine")
		
# Create our projects
#set(tetrisFolders "Projects/Tetris")
#addProject("Tetris" tetrisFolders)

#set(runeRPGFolders "Projects/RuneRPG")

# Include functions.
include(CMakeMacros.txt)
# Create project using specified project dir(s).
message("${PROJ_SPEC_DIR}")
add_subdirectory(${PROJ_SPEC_DIR} ${PROJ_SPEC_DIR}/out/)

