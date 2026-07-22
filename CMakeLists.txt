cmake_minimum_required(VERSION 3.19)
project(omp-anticheat LANGUAGES C CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# the pawn api (natives + onplayercheatdetected) needs the pawn + pawn-natives
# submodules. turn it off to build a lean, self-contained detection core that
# still kicks/bans via config but exposes nothing to gamemodes.
option(AC_ENABLE_PAWN "Build the Pawn natives + callback module" ON)
# real builds must be 32-bit (see below). turn this off only for a host
# syntax/type check of the code on a 64-bit machine - the result won't load.
option(AC_BUILD_32BIT "Build a 32-bit binary (required to actually load in open.mp)" ON)

# open.mp sa-mp clients (and therefore the open.mp server and every component it
# loads) are 32-bit. a 64-bit build will not load. on gcc/clang we force -m32;
# on msvc pass -a win32 to the generator instead.
if(AC_BUILD_32BIT AND NOT MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -m32")
endif()

# open.mp sdk (interface lib) + network lib (networkbitstream, only forward
# declared by the sdk) live in git submodules under lib/.
add_subdirectory(lib/sdk)
add_subdirectory(lib/network)

set(AC_SOURCES
	src/main.cpp
	src/anticheat_component.cpp
	src/config.cpp
	src/scheduler.cpp
	src/enforcement.cpp
	src/detection/memory_check.cpp
	src/detection/version_check.cpp
	src/detection/poison_check.cpp
	src/detection/mobile_check.cpp
	src/detection/raknet_check.cpp
	src/detection/faker5_check.cpp
	src/detection/scc_check.cpp
)

if(AC_ENABLE_PAWN)
	list(APPEND AC_SOURCES src/natives.cpp)
endif()

add_library(${PROJECT_NAME} SHARED ${AC_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE OMP-SDK OMP-Network)

if(AC_ENABLE_PAWN)
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		AC_ENABLE_PAWN
		HAVE_STDINT_H=1
		PAWN_CELL_SIZE=32
	)
	target_include_directories(${PROJECT_NAME} PRIVATE
		lib # for <pawn-natives/...> pulled in by the sdk pawn headers
		lib/pawn/source # for <amx/amx.h> and <linux/getch.c>
		lib/pawn/source/linux
	)
endif()

# open.mp loads "anticheat.dll" / "anticheat.so" (no "lib" prefix) from the
# server's components/ folder.
set_target_properties(${PROJECT_NAME} PROPERTIES
	PREFIX ""
	OUTPUT_NAME "anticheat"
	C_VISIBILITY_PRESET hidden
	CXX_VISIBILITY_PRESET hidden
)
