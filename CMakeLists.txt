# GameEngine Main CMakeLists file.

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

message("Defining file group macro")
#This macro will go through the source dirs and group the files in a nice tree <-- Stolen from Aksel
macro(create_file_group ROOT_DIR groupname)
#	message("Creating file group ${groupname}")
	FILE(GLOB thing RELATIVE ${ROOT_DIR} ${ROOT_DIR}/*)
	FOREACH(file ${thing})
		IF(IS_DIRECTORY ${ROOT_DIR}/${file})
			#message("Delving deeper into ${file}")
			create_file_group("${ROOT_DIR}/${file}" "${groupname}/${file}")
		ELSE()
			#message("Grouping file ${file}")
			STRING(REGEX REPLACE "/" "\\\\" formattedGroup ${groupname})
			SOURCE_GROUP("${formattedGroup}" FILES
				"${ROOT_DIR}/${file}"
			)
		ENDIF()
	ENDFOREACH()
endmacro(create_file_group)

message("Defining HideQtMocFiles macro")
#This will hide unnecessary QT generated files that noone wants.
macro(hide_qt_files src_dir)
	message("Looking for moc files in ${PROJECT_BINARY_DIR}/src/graphics/moc*")
	FILE(GLOB_RECURSE qt_files 
		${src_dir} REGEX "moc_*"
	)
	FILE(GLOB_RECURSE qt_ui_files 
		${src_dir} REGEX "ui_*"
	)
	SOURCE_GROUP("QT_Generated_stuff" FILES
			${qt_files}
			${qt_ui_files}
		)
endmacro(hide_qt_files)

macro(printList listToPrint)
	FOREACH(f ${listToPrint})
		message("File: ${f}")
	ENDFOREACH()
endmacro(printList)

# Removes all files belonging to target directory, using given list and list name. Inrements variable ${filesRemoved} for each file removed.
macro(remove_dir dir fromList listName filesR dirsToIgnore)
#	message("")
#	printList("${fromList}")
#	message ("List: ${listName}")
#	SET(filesRemoved 0)		
#	message("Files removed: ${filesR}")
	FOREACH(f ${fromList})
		if (${f} MATCHES ${dir})
		#	message(${dir})
			SET(ignore "No")
			# check if specifically requested to keep this
			FOREACH(dirToIgnore ${${dirsToIgnore}})
#				message(DirToIgnore: ${dirToIgnore})
				if (${f} MATCHES ${dirToIgnore})
					SET(ignore "Yes")
				endif()
			ENDFOREACH()
			
		#	message("F: ${f} MATCHES ${dir}")
		#	message("found stuff ${f}")
		#	message("found stuff ${filesRemoved}")
			if (${ignore} MATCHES "No")
				math(EXPR sum "${filesRemoved}+1")
				SET(filesRemoved ${sum})
				list(REMOVE_ITEM ${listName} ${f})		
			endif()
		endif()
	ENDFOREACH()
#	message("remove_dir macro called with dir: ${dir}")
#	message("list: ${fromList}")
#	message("Files removed from ${listName}: ${filesRemoved}")
endmacro(remove_dir)

# Removes target directory from both source and header files. Saves amount of removed files in ${filesRemoved}
macro(RemoveDir dir dirsToIgnore)
	SET(filesRemoved 0)
	remove_dir(${dir} "${source_files}" source_files ${filesRemoved} ${dirsToIgnore}) 
	remove_dir(${dir} "${header_files}" header_files ${filesRemoved} ${dirsToIgnore})
	message("RemoveDir with dir: ${dir}. Files removed: ${filesRemoved}")
endmacro(RemoveDir)
	
# Defines the list of project sub-folders that a basic project should NOT include.
# All new projects should add their own folder to this list so that they are excluded correctly from other builds.
set(projectFolders 
	# Folders of classes based on frameworks which could be a hassle to set up could also be listed here, if you are lazy about getting appropriate libs or just do not want to include certain parts.
	"CV"
	"Projects"
)
	
# Macro to add a project for this game-engine!
macro (addProject ProjectName projectFoldersToNotExclude)
	project(${ProjectName})
	set(project_name ${ProjectName})

	# Win32 MSVC++
	IF(MSVC)
		# In order to retain edit-and-continue build mode, must specify SAFESEH:NO in newer VS version.
		#IF(${_MACHINE_ARCH_FLAG} MATCHES X86)
		message("MSVC detected, trying to set /SAFESEH:NO to enable Edit and Continue builds.")
			message("Setting linker flags for /SAFESEH:NO")
			SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
			SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
			SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
		#ENDIF()
	ENDIF()
	
	# For enabling/disabling Debug-assertions (among other things)
	# http://msdn.microsoft.com/en-us/library/abx4dbyh%28v=vs.80%29.aspx
	message("Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
	set(CMAKE_CXX_FLAGS_RELEASE        "/O2 /Oi /GL /FD /EHsc /MT /Gy /W3 /nologo /c /Zi /TP /errorReport:prompt")
	message("Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")

	message("Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
	set(CMAKE_CXX_FLAGS_DEBUG        "/Od /Gm /EHsc /RTC1 /MTd /W3 /nologo /c /ZI /TP /errorReport:prompt")
	message("Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")

	# References: /OPT:REF - Eliminate Unreferenced Data
	# Enable COMDAT Folding: /OPT:ICF - Remove Redundant COMDATs
	# Enable Function-Level Linking: (Edit and Continue builds) /Gy

	set(DATA_ROOT ${CMAKE_SOURCE_DIR}/bin)

	# Generate config file with path for debugging in VS
	configure_file(
	 "${PROJECT_SOURCE_DIR}/src/config.h.in"
	 "${PROJECT_BINARY_DIR}/config.h"
	)
	
	message("Adding project: ${ProjectName}")
	
	# Collect source and header files
	set(source_dir src)
	include_directories(${CMAKE_BINARY_DIR})
	FILE(GLOB_RECURSE source_files "${source_dir}/*.cpp")
	FILE(GLOB_RECURSE header_files "${source_dir}/*.h")
		
	# Remove all project subfolders with the exception of those specified in the second argument!		
	FOREACH(projectSubFolder ${projectFolders})
#		set(exclude "Yes")
#		FOREACH (projectFolder ${projectFoldersToNotExclude})
#			message("~~~~~~~Project folder: ${projectFolder} MATCHES ${projectSubFolder}")
#			if (${projectSubFolder} MATCHES ${projectFolder})
#				message("-Folder name ${projectSubFolder} matches, skipping.")
#				# Skip it if it is one of the ones we want to include.
#				set(exclude "No")
#			endif()
#		ENDFOREACH()
		# Exclude it if specified
#		if (${exclude} MATCHES "Yes")
			RemoveDir(${projectSubFolder} ${projectFoldersToNotExclude})
#		endif()
	ENDFOREACH()

#	message("print list?")
#	printList("${source_files}")
#	message("source files: ${source_files}")

	# Add dependency-folders on linux too?
	# Recusive addition of all dependencies in the Dependencies folder, 
	# finding files in each's separate include/lib-dir.
	set(depDir ${CMAKE_SOURCE_DIR}/Dependencies)
	FILE(GLOB dependencies RELATIVE "${depDir}" "${depDir}/*")
	FOREACH(dependency ${dependencies})			
		include_directories(${CMAKE_SOURCE_DIR}/Dependencies)
		# For each folder, include their lib/include folders
		IF(IS_DIRECTORY ${depDir}/${dependency})
			link_directories("${depDir}/${dependency}/lib")
			include_directories("${depDir}/${dependency}")
		ENDIF()
	ENDFOREACH()	
	
		
	
	# LINUX library linking
	IF(UNIX)
		message("Configuring for UNIX OS")
		set(custom_libraries libGL.so  libGLU.so "-lGLEW")
		# AL
		set(custom_libraries ${custom_libraries} "-lalut -lopenal")
		# Ogg Vorbis
		set(custom_libraries ${custom_libraries} "-logg" "-lvorbis" "-lvorbisfile")
		# Theora
		set(custom_libraries ${custom_libraries} "-ltheora" "-ltheoradec")
		# Open CV
		set(custom_libraries ${custom_libraries} "-lopencv_core -lopencv_highgui -lopencv_imgproc")
	ENDIF(UNIX)

	# Win32 MSVC++
	IF(MSVC)
		message("Configuring for Windows MSVC")
		# Preprocessor definitions
		add_definitions(
			-DGLEW_STATIC
			-D_CRT_SECURE_NO_WARNINGS
			-DUNICODE
			-D_UNICODE
		)
		
		# FMOD Dirs
#		include_directories("C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api")

		
		# -D Definition 
		# -l Linker path (additional include directories)
		# -L lib files
		set(CMAKE_CXX_FLAGS_INIT 
			"GLEW_STATIC" 
			"_CRT_SECURE_NO_WARNINGS"
		)
		# Depending on what we're compiling, add stuff
		# Debug library dependencies
		set(DEBUG_LIBS 
			# For Microsoft game pads via DX SDK.
		#	xinput.lib
			glew32_MTd.lib
		#	glu32.lib
		#	opengl32.lib
			ws2_32.lib
			# Audio
			OpenAL32.lib
			libogg_static_MTd.lib
			libvorbis_static_MTd.lib
			libvorbisfile_static_MTd.lib
			libtheora_static_MTd.lib
		)
		# FMOD Studio, Audio
		set(DEBUG_LIBS ${DEBUG_LIBS} fmod_vc.lib)

		# Windows media library for setting system volume
		set(DEBUG_LIBS ${DEBUG_LIBS} Winmm.lib)
		
		# Open CV
		set(DEBUG_LIBS ${DEBUG_LIBS} opencv_core248d.lib opencv_highgui248d.lib opencv_imgproc248d.lib)
	
		# Release library dependencies
		set(RELEASE_LIBS 
			# For Microsoft game pads via DX SDK.
			xinput.lib
			glew32s.lib
			glu32.lib
			opengl32.lib
			ws2_32.lib
			# Audio
			OpenAL32.lib
			libogg_static_release.lib
			libvorbis_static_release.lib
			libvorbisfile_static_release.lib
			libtheora_static_release.lib
		)
		# OpenCV
		set(RELEASE_LIBS ${RELEASE_LIBS} opencv_core248.lib opencv_highgui248.lib opencv_imgproc248.lib)

		create_file_group(${PROJECT_SOURCE_DIR}/${source_dir} "/")
	ENDIF(MSVC)

	message("Setting include directories")

	#Additional include directories specified here (?)
	include_directories(
		${include_directories}
		${source_dir}
		${source_dir}/Util
		${source_dir}/OS
		${source_dir}/Projects
	)
	
	#printList("${source_files}")

			
	# Set binaries to build.
	message("Adding executable")
	add_executable(
		${project_name} WIN32
		${source_files} 
		${header_files}
		${additional_files}
	)

	# Set up target link libraries for debug and release
	message("Setting up link libraries")
	IF(MSVC)
		FOREACH(d_lib ${DEBUG_LIBS})
			target_link_libraries(${PROJECT_NAME} debug ${d_lib})
		ENDFOREACH()
		
		FOREACH(r_lib ${RELEASE_LIBS})
			target_link_libraries(${PROJECT_NAME} optimized ${r_lib})
		ENDFOREACH()
	ELSE(MSVC)
		target_link_libraries(${PROJECT_NAME} ${custom_libraries})
	ENDIF(MSVC)

	# Set working dir 
	
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
	
	set(PROJECT_BIN_DIR "../bin")
	set(TargetName ${PROJECT_NAME})
	SET_TARGET_PROPERTIES(${TargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BIN_DIR})
	SET_TARGET_PROPERTIES(${TargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BIN_DIR})
	SET_TARGET_PROPERTIES(${TargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BIN_DIR})
	SET_TARGET_PROPERTIES(${TargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECT_BIN_DIR})
	SET_TARGET_PROPERTIES(${TargetName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${PROJECT_BIN_DIR}) 
	SET_TARGET_PROPERTIES(${TargetName} PROPERTIES DEBUG_POSTFIX ".debug")
	SET_TARGET_PROPERTIES(${TargetName} PROPERTIES PDB_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/pdb)
	SET_TARGET_PROPERTIES(${TargetName} PROPERTIES PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pdb) 
	SET_TARGET_PROPERTIES(${TargetName} PROPERTIES PDB_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/pdb)
	SET_TARGET_PROPERTIES(${TargetName} PROPERTIES PDB_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/pdb)
	SET_TARGET_PROPERTIES(${TargetName} PROPERTIES ILK_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/ilk)
# end of AddProject macro/function, add projects below!
endmacro(addProject)

# Create our projects
set(runeRPGFolders "Projects/RuneRPG")
addProject("RuneRPG" runeRPGFolders)
set(musicPlayerFolders "Projects/MusicPlayer")
addProject("MusicPlayer" musicPlayerFolders)
set (sideScrollerFolders "Projects/SideScroller")
addProject("SideScroller" sideScrollerFolders)
set(spaceRaceFolders "Projects/SpaceRace")
addProject("SpaceRace" spaceRaceFolders)

set(horrorFolders "Projects/Horror")
addProject("Horror" horrorFolders)


# Create our projects
set(ipmFolders "Projects/IPM")
addProject("IPM" ipmFolders)

