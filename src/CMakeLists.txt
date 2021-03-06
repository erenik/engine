# GameEngine Main CMakeLists file.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Create options
set(ENGINE_SRC_DIR "C:/git/engine/src" CACHE STRING "Path to the engine repository where this file lies.")
set(LIBCURL "OFF" CACHE BOOL "Whether to compile and link with LibCURL. Required for HTTP requests")
set(OPENCV "OFF" CACHE BOOL "Whether to compile and link with OpenCV. Required for image processing.")
set(OPENAL "ON" CACHE BOOL "Whether to compile and link with OpenAL soft. Required for sound playback.")
set(OPENAL_INCLUDE_DIR "C:/Program Files (x86)/OpenAL 1.1 SDK/include" CACHE STRING "" )
set(OPENAL_LIBRARY "C:/Program Files (x86)/OpenAL 1.1 SDK/libs/Win64" CACHE STRING "" )
set(OGG_LIB_DIR "C:/libs/libogg/win32/VS2010/x64/" CACHE STRING "" )
set(OPUS "OFF" CACHE BOOL "Whether to compile and link with (Ogg/)Opus. Required for certain sound playback.")
set(VORBIS "ON" CACHE BOOL "Whether to compile and link with (Ogg/)Vorbis. Required for certain sound playback.")
set(VORBIS_LIB_DIR "C:/libs/libvorbis/win32/VS2010/x64/")
set(THEORA "OFF" CACHE BOOL "Whether to compile and link with (Ogg/)Theora. Required for certain video playback.")
set(LIBS_DIR "C:/libs" CACHE STRING "Root directory for additional dependencies (OpenGL, CV, AL, etc.)")
set(INSTALL_DEPENDENCIES "OFF" CACHE BOOL "Whether to automatically download and try set up dependencies or not.")

message("Binary dir: ${CMAKE_BINARY_DIR}")
message("Source dir: ${CMAKE_SOURCE_DIR}")

message("Defining file group macro")

# This macro will go through the source dirs and group the files in a nice tree
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
	message("Print list....")
	FOREACH(f ${listToPrint})
		message("File: ${f}")
	ENDFOREACH()
	message("List end.")
endmacro(printList)

# Removes all files belonging to target directory, using given list and list name. Inrements variable ${filesRemoved} for each file removed.
macro(remove_dir dir fromList listName filesR)
	FOREACH(f ${fromList})
		if (${f} MATCHES ${dir})
			SET(ignore "No")
			if (${ignore} MATCHES "No")
				math(EXPR sum "${filesRemoved}+1")
				SET(filesRemoved ${sum})
				list(REMOVE_ITEM ${listName} ${f})		
			endif()
		endif()
	ENDFOREACH()
endmacro(remove_dir)

# Removes target directory from both source and header files. Saves amount of removed files in ${filesRemoved}
macro(RemoveDir dir)
	SET(filesRemoved 0)
	remove_dir(${dir} "${source_files}" source_files ${filesRemoved}) 
	remove_dir(${dir} "${header_files}" header_files ${filesRemoved})
endmacro(RemoveDir)
	
# Defines the list of project sub-folders that a basic project should NOT include.
# All new projects should add their own folder to this list so that they are excluded correctly from other builds.
set(projectFolders 
	# Folders of classes based on frameworks which could be a hassle to set up could also be listed here, if you are lazy about getting appropriate libs or just do not want to include certain parts.
	"/CV/"
	"/MiniGames/"
)

macro(UnpackLibZip name)
	message("Unpacking ${name} ...")
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E tar xvf "${name}" 
		WORKING_DIRECTORY ${LIBS_DIR}
		OUTPUT_QUIET
	)	
endmacro(UnpackLibZip)

macro(InstallDependencies)
	message("Checking for downloaded dependencies.")
	set(OpenALZipPath ${LIBS_DIR}/libopenal.zip)
	set(libglewPath ${LIBS_DIR}/libglew.zip)
	set(liboggPath ${LIBS_DIR}/libogg.zip)
	set(libvorbisPath ${LIBS_DIR}/libvorbis.zip)
	set(opencvPath ${LIBS_DIR}/opencv.exe)
	set(zlibPath ${LIBS_DIR}/zlib.zip)

	if(EXISTS "${OpenALZipPath}")
	else()
		message("Downloading OpenAL soft")
		file(DOWNLOAD "https://www.openal.org/downloads/OpenAL11CoreSDK.zip" ${OpenALZipPath} 
			LOG ${LIBS_DIR}/downloadlog.txt SHOW_PROGRESS
		)
		UnpackLibZip(${OpenALZipPath})
	endif()
	if (EXISTS "${libglewPath}")
	else()
		message("Downloading glew (OpenGL Extension Wrangler)")
		file(DOWNLOAD "http://downloads.sourceforge.net/project/glew/glew/1.12.0/glew-1.12.0-win32.zip" ${libglewPath} 
			LOG ${LIBS_DIR}/downloadlog.txt SHOW_PROGRESS
		)
		UnpackLibZip(${libglewPath})
	endif()
	if (EXISTS "${liboggPath}")
	else()
		message("Downloading libogg (Xiph.org)")
		file(DOWNLOAD "http://downloads.xiph.org/releases/ogg/libogg-1.3.2.zip" ${liboggPath} 
			LOG ${LIBS_DIR}/downloadlog.txt SHOW_PROGRESS
		)
		UnpackLibZip(${liboggPath})
	endif()
	
	if (EXISTS "${libvorbisPath}")
	else()
		message("Downloading libvorbis (Xiph.org)")
		file(DOWNLOAD "http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.zip" ${libvorbisPath} 
			LOG ${LIBS_DIR}/downloadlog.txt SHOW_PROGRESS
		)
		UnpackLibZip(${libvorbisPath})
	endif()
	
	if (OPENCV)
		if (EXISTS "${opencvPath}")
		else()
			message("Downloading OpenCV")
			file(DOWNLOAD "http://downloads.sourceforge.net/project/opencvlibrary/opencv-win/2.4.10/opencv-2.4.10.exe" ${opencvPath} 
				LOG ${LIBS_DIR}/downloadlog.txt SHOW_PROGRESS
			)
			UnpackLibZip(${opencvPath})
		endif()
	else()
	endif()

	if (EXISTS "${zlibPath}")
	else()
		message("Downloading zlib")
		file(DOWNLOAD "https://www.zlib.net/zlib1211.zip" ${zlibPath}
			LOG ${LIBS_DIR}/downloadlog.txt SHOW_PROGRESS 
		)
		UnpackLibZip(${zlibPath})
	endif()

#	message("Unpacking archives...")
	# Change working dir


#	execute_process(
#		COMMAND ${CMAKE_COMMAND} -E tar xvf "${opencvPath}" 
#		WORKING_DIRECTORY ${LIBS_DIR}
#		OUTPUT_QUIET
#	)
	
#	execute_process(
#		COMMAND ${CMAKE_COMMAND} -E tar xvf "${OpenALZipPath}" 
#		WORKING_DIRECTORY ${LIBS_DIR}
#		OUTPUT_QUIET
#	)

#	add_custom_command(
#		TARGET run
#		COMMAND ${CMAKE_COMMAND} -E tar xvf "${OpenALZipPath}"
#		WORKING_DIRECTORY ${LIBS_DIR}
#	)
#	add_dependencies(run)
	
#	execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvf "${OpenALZipPath}" -C ${LIBS_DIR})
	
#	
#	add_custom_command(
#		OUTPUT ${LIBS_DIR}/
#		COMMAND 
#			unzip -o ${OpenALZipPath}
#		DEPENDS
#			${OpenALZipPath}
#	)
	
endmacro(InstallDependencies)

macro(ReadFileConcatComma filePathAbsolute)
    message("Reading file at ${filePathAbsolute}")
    FILE(READ "${filePathAbsolute}" contents)
    # Convert file contents into a CMake list (where each element in the list
    # is one line of the file)
    STRING(REGEX REPLACE ";" "\\\\;" contents "${contents}")
    STRING(REGEX REPLACE "\n" ";" contents "${contents}")
    set(commaList ${contents})
endmacro(ReadFileConcatComma)

# Macro to read in a .txt file with 1 include directory per line (for dependencies)
macro(AddIncludeDirs filePathAbsolute)
    ReadFileConcatComma(${filePathAbsolute})
    STRING(REGEX REPLACE "LIBS_DIR" ${LIBS_DIR} commaList "${commaList}")
	message("After replace: " ${commaList})
	set(lib_include_dirs ${lib_include_dirs} ${commaList} )
	message("lib_include_dirs: ${lib_include_dirs}")
endmacro(AddIncludeDirs)

# Macro to read in a .txt file with 1 include directory per line (for dependencies)
macro(AddLibraryDirs filePathAbsolute)
    ReadFileConcatComma(${filePathAbsolute})
    STRING(REGEX REPLACE "LIBS_DIR" ${LIBS_DIR} commaList "${commaList}")
    message("After replace: " ${commaList})
	set(lib_library_dirs ${commaList} )
	message("lib_library_dirs: ${lib_library_dirs}")
endmacro(AddLibraryDirs)


macro(AddFilesInDir pathAbsolute)
    message("Adding source files from path ${pathAbsolute}")	
	# Gather all .cpp and .h source files recursively.
	FILE(GLOB_RECURSE more_source_files "${pathAbsolute}/*.cpp")
	FILE(GLOB_RECURSE more_header_files "${pathAbsolute}/*.h")		
    list(LENGTH more_source_files numFiles)
    list(LENGTH more_header_files numFiles2)
    list(APPEND source_files ${more_source_files})
    list(APPEND header_files ${more_header_files})
    Message(" amount of files found: " ${numFiles}+${numFiles2})
endmacro(AddFilesInDir)

# Macro to add a project for this game-engine!
macro (addProject ProjectName projectSrcDir engineSrcDir)
    project(${ProjectName})
	set(project_name ${ProjectName})

	# Generate a Header file with indications as to which libraries are being built and linked against.
	SET(libs "// Auto-generated header file based on CMake build options. -- Emil")
		IF (LIBCURL)
			SET(libs ${libs} "\n#define LIBCURL")
		ENDIF(LIBCURL)
		IF (OPENCV)
			SET(libs ${libs} "\n#define OPENCV")
		ENDIF(OPENCV)
		IF (OPENAL)
			SET(libs ${libs} "\n#define OPENAL")
		ENDIF(OPENAL)
		IF (OPUS OR VORBIS OR THEORA)
			SET(libs ${libs} "\n#define BUILD_OGG")
		ENDIF (OPUS OR VORBIS OR THEORA)
		IF (OPUS)
			SET(libs ${libs} "\n#define OPUS")
		ENDIF(OPUS)
		IF (VORBIS)
			SET(libs ${libs} "\n#define VORBIS")
		ENDIF(VORBIS)
		IF (THEORA)
			SET(libs ${libs} "\n#define THEORA")
		ENDIF(THEORA)
		IF(LIBPNG)
			SET(libs ${libs} "\n#define LIBPNG")
		ENDIF(LIBPNG)
#	Write to BINARY dir instead of SOURCE dir to make it project/CMake independent.
	file(WRITE "${CMAKE_BINARY_DIR}/Libs.h" ${libs})
	# Include it too.
	set(lib_include_dirs ${lib_include_dirs} ${CMAKE_BINARY_DIR} )

	
	# Win32 MSVC++
	IF(MSVC)
		# Download and install dependencies if marked.
		IF (INSTALL_DEPENDENCIES)
			InstallDependencies()
		ENDIF(INSTALL_DEPENDENCIES)
		
		# Add include dirs
		AddIncludeDirs(${engineSrcDir}/Win32_IncludeDirs.txt)
		AddLibraryDirs(${engineSrcDir}/Win32_LibraryDirs.txt)
		
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
	# https://msdn.microsoft.com/en-us/library/fsk896zz.aspx?f=255&MSPPError=-2147217396
	# /MP for multi-processor compilation - use for both debug and release
	# /MD for Multi-threaded DLL. MT for Multi-threaded stand-alone. /MDd for Multi-threaded debug DLL.
#	set(CMAKE_CXX_FLAGS_RELEASE "/MD /DEBUG")
#	message("Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
	# /Od - no optimization.
	# /ZI - enable edit and continue builds.
	# /INCREMENTAL needed for edit and continue iirc.
	# NOREF and NOICF needed for edit and continue
	set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Oi /MP /GL /FD /EHsc /MT /Gy /W3 /nologo /c /Zi /TP /errorReport:prompt	/MP ")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO /OPT:REF /OPT:ICF /DEBUG")
	# linker flags o.o'
	set(CMAKE_CXX_FLAGS_DEBUG  "/Od /ZI /MP /MDd")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/OPT:NOREF /OPT:NOICF /INCREMENTAL /DEBUG")

	set(CMAKE_CXX_STANDARD 14)

#	set(CMAKE_CXX_FLAGS_DEBUG        "/Od /MP /EHsc /RTC1 /MTd /W3 /nologo /c /ZI /TP /errorReport:prompt")
#	message("Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")

	# References: /OPT:REF - Eliminate Unreferenced Data
	# Enable COMDAT Folding: /OPT:ICF - Remove Redundant COMDATs
	# Enable Function-Level Linking: (Edit and Continue builds) /Gy

	message("Adding project: ${ProjectName}")

    AddFilesInDir(${ENGINE_SRC_DIR})
    AddFilesInDir(${CMAKE_SOURCE_DIR})
	
	# Remove all project subfolders with the exception of those specified in the 	second argument!		
	FOREACH(projectSubFolder ${projectFolders})
		RemoveDir(${projectSubFolder})
	ENDFOREACH()
		
	# LINUX library linking
	IF(UNIX)
		message("Configuring for UNIX OS")
		add_definitions(
			-DGLEW_BUILD # For glew to link properly..?
		)
		set(custom_libraries libGL.so  libGLU.so "-lGLEW")
		# Link to X11
		set(custom_libraries ${custom_libraries} "-lX11 -lpthread")
		# AL
		set(custom_libraries ${custom_libraries} "-lopenal")
		# Ogg Vorbis
		set(custom_libraries ${custom_libraries} "-logg" "-lvorbis" "-lvorbisfile")
		# Theora
		set(custom_libraries ${custom_libraries} "-ltheora" "-ltheoradec")
		# Open CV
		set(custom_libraries ${custom_libraries} "-lopencv_core -lopencv_highgui -lopencv_imgproc")
		# zlib
		set(custom_libraries ${custom_libraries} "-lz")
	ENDIF(UNIX)

	# Win32 MSVC++
	IF(MSVC)
		message("Configuring for Windows MSVC")

		set(CMAKE_VS_PLATFORM_NAME_DEFAULT "x64")
		set(CMAKE_GENERATOR_PLATFORM "x64")
		message("Generating for Architecture: ${CMAKE_GENERATOR_PLATFORM}")	

		# Preprocessor definitions
		add_definitions(
			-DGLEW_STATIC
			-D_CRT_SECURE_NO_WARNINGS
			-DUNICODE
			-D_UNICODE
			# For statically linked OpenAL!
		#	-DAL_LIBTYPE_STATIC
			# Target platform
#			-D_WIN32_WINNT=0x0600
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
			xinput.lib
			glew32s.lib
		#	glu32.lib
		#	opengl32.lib
			ws2_32.lib
			zlibstaticd.lib
		)
		# FMOD Studio, Audio
	#	set(DEBUG_LIBS ${DEBUG_LIBS} fmod_vc.lib)

		# Windows media library for setting system volume
		set(DEBUG_LIBS ${DEBUG_LIBS} Winmm.lib)
		
			
		# Release library dependencies
		set(RELEASE_LIBS 
			# For Microsoft game pads via DX SDK.
			Winmm.lib
			xinput.lib
			glew32s.lib
			glu32.lib
			opengl32.lib
			ws2_32.lib
			zlibstatic.lib
		)

		# Windows Core Audio
#		set(DEBUG_LIBS ${DEBUG_LIBS} Audioses.dll Mmdevapi.dll)
#		set(RELEASE_LIBS ${DEBUG_LIBS} Audioses.dll Mmdevapi.dll)
		
		# Open AL
		if (OPENAL)
			set(DEBUG_LIBS ${DEBUG_LIBS} OpenAL32.lib)
			set(RELEASE_LIBS ${RELEASE_LIBS} OpenAL32.lib)
		ENDIF(OPENAL)
			
		# OGG Container for Vorbis, Opus, Theora, etc.
		if ((VORBIS OR OPUS) OR THEORA)
#			message("Linking with OGG")
			set(DEBUG_LIBS ${DEBUG_LIBS} ${OGG_LIB_DIR}/Debug/libogg_static.lib)
			set(RELEASE_LIBS ${RELEASE_LIBS} ${OGG_LIB_DIR}/Release/libogg_static.lib)
		ENDIF((VORBIS OR OPUS) OR THEORA)
			
		# VORBIS Audio codec
		if (VORBIS)
#			message("Linking with VORBIS")
			set(DEBUG_LIBS ${DEBUG_LIBS} 
				${VORBIS_LIB_DIR}/Debug/libvorbis_static.lib
				${VORBIS_LIB_DIR}/Debug/libvorbisfile_static.lib
			)
			set(RELEASE_LIBS ${RELEASE_LIBS} 
				${VORBIS_LIB_DIR}/Release/libvorbis_static.lib
				${VORBIS_LIB_DIR}/Release/libvorbisfile_static.lib
			)
		ENDIF(VORBIS)

		# OPUS Audio codec/compression library
		if (OPUS)
			message("Linking with OPUS")
			set(DEBUG_LIBS ${DEBUG_LIBS} opusd.lib celtd.lib
				silk_commond.lib silk_fixedd.lib silk_floatd.lib opusfiled.lib
			)
			set(RELEASE_LIBS ${RELEASE_LIBS} opus.lib celt.lib
				silk_common.lib silk_fixed.lib silk_float.lib opusfile.lib
			)
		ENDIF(OPUS)

		# Theora video codec
		if (THEORA)
			set(DEBUG_LIBS ${DEBUG_LIBS} libtheora_static_MTd.lib)
			set(RELEASE_LIBS ${RELEASE_LIBS} libtheora_static_release.lib)
		ENDIF(THEORA)
				
		# OpenCV - Image processing and input/output
		if (OPENCV)
#			message("Linking with OpenCV")
			set(DEBUG_LIBS ${DEBUG_LIBS} 
				opencv_core2410d.lib 
				opencv_highgui2410d.lib 
		#		opencv_imgproc248d.lib 
		#		opencv_video248d.lib
			)
			set(RELEASE_LIBS ${RELEASE_LIBS} 
				opencv_core2410.lib 
				opencv_highgui2410.lib 
#				opencv_imgproc248.lib 
#				opencv_video248.lib
			)
		ENDIF(OPENCV)

		# Curl - HTTP and other internet stuffs
		if (LIBCURL)
#			message("Linking with libcurl.")
			#message("Libs: " ${RELEASE_LIBS})
			set(DEBUG_LIBS ${DEBUG_LIBS}
				libcurld_static.lib
			)
			set(RELEASE_LIBS ${RELEASE_LIBS}
				libcurl_static.lib
			)
		ENDIF(LIBCURL)
					
		# Group files into the same structure as they appear in the file directories.
		create_file_group(${ENGINE_SRC_DIR} "engine/")
		create_file_group(${CMAKE_SOURCE_DIR} "/")
	ENDIF(MSVC)

	message("Setting include directories")

	# Additional include directories specified here (?)
	include_directories(
		${lib_include_dirs}
		${OPENAL_INCLUDE_DIR}
		${CMAKE_SOURCE_DIR}
		${ENGINE_SRC_DIR}
		${ENGINE_SRC_DIR}/Util
		${ENGINE_SRC_DIR}/OS
		${ENGINE_SRC_DIR}/Projects
	)
	# Lib dirs.
	link_directories(
		${lib_library_dirs}
		${OPENAL_LIBRARY}
	)
	
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

	set(PROJECT_BIN_DIR ${CMAKE_SOURCE_DIR}/../bin)
	message("PROJECT_BIN_DIR: ${PROJECT_BIN_DIR}")
	
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


	if(MSVC AND ${MSVC_VERSION} GREATER_EQUAL 1915)
		# Due to issues of std::shared_ptr in use with aligned Vector4f.
		target_compile_definitions( ${TargetName} PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE )
	endif()

# end of AddProject macro/function, add projects below!
endmacro(addProject)


