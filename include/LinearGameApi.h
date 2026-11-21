#pragma once

// LinearGame.dll is the game simulation - world files, parts, physics bodies, the
// character controller, the camera, picking and the avatar - that Client.exe,
// Server.exe, Studio.exe and AiTrainer.exe all run. It is one DLL instead of the same
// objects compiled into four programs so that an update is small: a change to movement,
// a world file or a physics body replaces LinearGame.dll, not four executables of
// several megabytes each.
//
// Nothing in the library may name a window or call OpenGL. linearwin keeps its
// platform state per module, and a window handle has to be used by the module that
// created it, so the renderer, the shaders, the skybox and the shadow map stay in the
// executables that own the GL context.
//
// Both sides are linked with the same static C++ runtime, so a std::string or
// std::deque crosses the boundary without a second heap. The executables and the DLL
// are always packaged and updated as one set for that reason.
#if defined(LINEAR_GAME_BUILD)
#define LINEAR_GAME_API __declspec(dllexport)
#else
#define LINEAR_GAME_API __declspec(dllimport)
#endif

// C4251 is MSVC reporting that a private std:: container member of an exported class
// "needs to have dll-interface". It does not: clients never touch those members and
// never construct the class from its size, and exporting the STL would tie the DLL to a
// matching toolset. Push and pop around the declaration so the rest of the translation
// unit keeps the warning.
#if defined(_MSC_VER)
#define LINEAR_GAME_API_BEGIN __pragma(warning(push)) __pragma(warning(disable : 4251))
#define LINEAR_GAME_API_END __pragma(warning(pop))
#else
#define LINEAR_GAME_API_BEGIN
#define LINEAR_GAME_API_END
#endif
