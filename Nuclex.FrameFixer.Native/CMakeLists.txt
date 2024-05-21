#!/usr/bin/cmake
cmake_minimum_required (VERSION 3.8)

# -------------------------------------------------------------------------------------------------

project(
	NuclexTelecideNative
	VERSION 1.0.0
	DESCRIPTION "Converts .wav files to Opus without clipping"
)

option(
	BUILD_DOCS
	"Whether to generate documentation via Doxygen"
	OFF
)

option(
	BUILD_UNIT_TESTS
	"Whether to build the unit test executable. This will require an extra \
	compilation of the entire source tree as well as the GoogleTest library."
	ON
)

option(
	BUILD_BENCHMARK
	"Whether to build the benchmark executable. This will require an extra \
	compilation of the entire source tree as well as the Celero library."
	OFF
)

option(
	ENABLE_YADIF
	"Whether to support YADIF (a relatively good deinterlacer supported by \
	various media players and ffmpeg. Otherwise, simple interpolation is used"
	ON
)

# -------------------------------------------------------------------------------------------------

# Qt: automatically run the "Meta-Object Compiler" which reads C++ header files and
# generates additional code from Qt's C++ extensions.
#   https://doc.qt.io/qt-5/moc.html
set(CMAKE_AUTOMOC ON)

# Qt: automatically run the "User Interface Compiler" on user interface definition
# files (.ui) and generate matching C++ header files.
#   https://doc.qt.io/qt-5/uic.html
set(CMAKE_AUTOUIC ON)

# Qt: automatically run the "Resource Compiler" on Qt resource files (.qrc) to read
# all referenced resources and generate C++ sources storing the file contents.
#   https://doc.qt.io/qt-5/rcc.html
set(CMAKE_AUTORCC ON)

# find includes in the corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS UserInterface)

# Contains compiler options, compiler tag for output directory, etc.
include("../BuildSystem/cmake/cplusplus.cmake")

# The Unix build pipeline doesn't automatically include threading, so search for
# the pthreads library in order to link against it later on.
#   https://en.wikipedia.org/wiki/Pthreads
find_package(Threads REQUIRED)

# Locate Qt, the cross-platform User Interface and base API abstraction library
# we're using for all UI stuff
find_package(Qt5 COMPONENTS REQUIRED Widgets)

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(AVFILTER_INCLUDE_DIR libavfilter/avfilter.h)
find_library(AVFILTER_LIBRARY avfilter)

# Add Nuclex.Support.Native as a sub-project, we link it for utility methods.
if(NOT (TARGET NuclexSupportNative))
	add_subdirectory(
		${PROJECT_SOURCE_DIR}/../Nuclex.Support.Native
		${CMAKE_BINARY_DIR}/NuclexSupportNative
	)
endif()

# Add Nuclex.Platform.Native as a sub-project, it aids in directory lookups.
if(NOT (TARGET NuclexPlatformNative))
	add_subdirectory(
		${PROJECT_SOURCE_DIR}/../Nuclex.Platform.Native
		${CMAKE_BINARY_DIR}/NuclexPlatformNative
	)
endif()

# Add Nuclex.Pixels.Native as a sub-project, it provides all the image processing code
if(NOT (TARGET NuclexPixelsNative))
	add_subdirectory(
		${PROJECT_SOURCE_DIR}/../Nuclex.Pixels.Native
		${CMAKE_BINARY_DIR}/NuclexPixelsNative
	)
endif()

message(STATUS "Enabled options for Nuclex.Telecide.Native:")
message(STATUS "  ⚫ Build core library")

if(BUILD_UNIT_TESTS)
	message(STATUS "  ⚫ Build unit tests")

	# Add GoogleTest as a sub-project so we can link our unit test executable
	if(NOT (TARGET GoogleTest))
		add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/gtest ${CMAKE_BINARY_DIR}/gtest)
	endif()
endif()

if(BUILD_BENCHMARK)
	message(STATUS "  ⚫ Build benchmark")

	# Add Celero as a sub-project so we can link our benchmark executable
	if(NOT (TARGET Celero))
		add_subdirectory(${PROJECT_SOURCE_DIR}/../ThirdParty/celero ${CMAKE_BINARY_DIR}/celero)
	endif()
endif()

# Use CMake's own package for locating Doxygen on the system
if(BUILD_DOCS)
	find_package(Doxygen)
endif()

# Add OpenCV if Yadif is enabled. The Yadif implemented used here is an OpenCV port
# to avoid pulling in complex depencies from AviSynth, AVSynth, VapourSynth or ffmpeg.
if(ENABLE_YADIF)
	find_package(OpenCV REQUIRED)
endif()

# -------------------------------------------------------------------------------------------------

# Project structure
#
#   ProjectName/
#     Source/                   All source files, using deeper directories as needed
#     Include/ProjectName/      All public headers, using deeper directories as needed
#     Tests/                    All unit tests, using deeper directories as needed
#     Benchmarks/               All benchmark files, using deeper directories as needed
#
# CMake documentation:
#   |  Note: We do not recommend using GLOB to collect a list of
#   |  source files from your source tree. If no CMakeLists.txt file
#   |  changes when a source is added or removed then the generated
#   |  build system cannot know when to ask CMake to regenerate.
#
# As so very often, CMake becomes a hurdle rather than helping.
# I'm not going to manually maintain a list of source files. Rebuilds
# where files are added, removed or renamed need to be from scratch.
#
file(
	GLOB_RECURSE userInterfaceFiles
	CONFIGURE_DEPENDS
	"UserInterface/*.*"
)
file(
	GLOB_RECURSE sourceFiles
	CONFIGURE_DEPENDS
	"Source/*.cpp"
	"Source/*.c"
)
file(
	GLOB_RECURSE headerFiles
	CONFIGURE_DEPENDS
	"Include/Nuclex/Telecide/*.h"
)
file(
	GLOB_RECURSE unittestFiles
	CONFIGURE_DEPENDS
	"Tests/*.cpp"
)
file(
	GLOB_RECURSE benchmarkFiles
	CONFIGURE_DEPENDS
	"Benchmarks/*.cpp"
)

# -------------------------------------------------------------------------------------------------

function(add_third_party_libraries target_name)

	target_link_libraries(
		${target_name}
    PUBLIC NuclexSupportNative
    PUBLIC NuclexPlatformNative
    PUBLIC NuclexPixelsNative
    PRIVATE Qt5::Widgets
    PRIVATE ${OpenCV_LIBS}
    PRIVATE ${AVCODEC_LIBRARY} 
    PRIVATE ${AVFORMAT_LIBRARY} 
    PRIVATE ${AVUTIL_LIBRARY} 
    PRIVATE ${AVFILTER_LIBRARY}
		PRIVATE Threads::Threads
  )

  target_include_directories(
		${target_name}
    PRIVATE ${AVCODEC_INCLUDE_DIR} 
    PRIVATE ${AVFORMAT_INCLUDE_DIR} 
    PRIVATE ${AVUTIL_INCLUDE_DIR} 
    PRIVATE ${AVFILTER_INCLUDE_DIR}
  )

	# On Unix systems, the application and unit test executable should look for
	# dependencies in its own directory first.
	set_target_properties(
		${target_name} PROPERTIES
		BUILD_RPATH_USE_ORIGIN ON
		BUILD_WITH_INSTALL_RPATH ON
		INSTALL_RPATH_USE_LINK_PATH OFF
		INSTALL_RPATH "\${ORIGIN}"
	)

endfunction()

# -------------------------------------------------------------------------------------------------

# name of the .exe file, window flag and the list of things to compile
add_executable(NuclexTelecideNative)

# Enable compiler warnings only if this application is compiled on its own.
# If it's used as a sub-project, the including project's developers aren't
# interested in seeing warnings from a project they're not maintaining.
if(${CMAKE_PROJECT_NAME} STREQUAL "NuclexTelecideNative")
	enable_target_compiler_warnings(NuclexTelecideNative)
else()
	disable_target_compiler_warnings(NuclexTelecideNative)
endif()

# Add directory with public headers to include path
target_include_directories(
	NuclexTelecideNative
	PUBLIC "Include"
)

# Add public headers and sources to compilation list
# (headers, too, in case CMake is used to generate an IDE project)
target_sources(
	NuclexTelecideNative
	PUBLIC ${headerFiles}
	PRIVATE ${sourceFiles}
	PRIVATE ${userInterfaceFiles}
)

# Add include directories and static libraries the application depends on
add_third_party_libraries(NuclexTelecideNative)

# -------------------------------------------------------------------------------------------------

set_property(GLOBAL PROPERTY QUIET_INSTALL ON)

#file(
#	COPY ${PROJECT_SOURCE_DIR}/Telecide.ini
#	DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}/
#)

# Install the executable into a subdirectory of this CMakeLists.txt file
# under ./bin/linux-gcc9.3-amd64-debug/ (the second-level directory is called
# "compiler tag" and dynamically formed -- it ensures that when linking
# a pre-compiled shared library, the correct library is used).
install(
	TARGETS NuclexTelecideNative
	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# Do the same for Nuclex.Pixels.Native. Since we depend on this library
# and have set the rpath accordingly, it needs to be in the same directory
install(
	TARGETS NuclexPixelsNative
	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# Do the same for Nuclex.Platform.Native. Since we depend on this library
# and have set the rpath accordingly, it needs to be in the same directory
install(
	TARGETS NuclexPlatformNative
	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# Do the same for Nuclex.Support.Native. Since we depend on this library
# and have set the rpath accordingly, it needs to be in the same directory
install(
	TARGETS NuclexSupportNative
	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/${NUCLEX_COMPILER_TAG}
)

# Install .pdb files on Windows platforms for the main application
install_debug_symbols(NuclexTelecideNative)

# -------------------------------------------------------------------------------------------------

if(BUILD_DOCS)

	if(NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Can't build documentation because Doxygen was not found")
	endif()

	add_custom_target(
		NuclexTelecideNativeDocs ALL
		COMMAND ${DOXYGEN_EXECUTABLE} "Nuclex.Telecide.Native.doxygen.cfg"
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	)

endif()

# -------------------------------------------------------------------------------------------------
