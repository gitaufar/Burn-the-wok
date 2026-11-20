cmake_minimum_required(VERSION 3.16)
project(BurnTheWok LANGUAGES CXX)

# Visual Studio remains the Windows build entry point.
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This CMake build targets Linux. On Windows, use Burn The Wok.slnx.")
endif()

find_package(SDL2 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)

add_library(Engine STATIC
    Engine/src/audio/Audio.cpp
    Engine/src/core/Application.cpp
    Engine/src/core/AssetPath.cpp
    Engine/src/core/Log.cpp
    Engine/src/core/SceneManager.cpp
    Engine/src/entity/EntityWorld.cpp
    Engine/src/graphics/Camera2D.cpp
    Engine/src/graphics/Image.cpp
    Engine/src/graphics/Shader.cpp
    Engine/src/graphics/SpriteBatch.cpp
    Engine/src/graphics/Texture.cpp
    Engine/src/graphics/Window.cpp
    Engine/src/input/Input.cpp
    Engine/src/physics/Collision.cpp
)
target_compile_features(Engine PUBLIC cxx_std_20)
set_target_properties(Engine PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(Engine PUBLIC Engine/include)
target_compile_definitions(Engine PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(Engine PRIVATE SDL2::SDL2 GLEW::GLEW OpenGL::GL PNG::PNG)

set(GAME_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Burn The Wok/src")
add_executable(burn-the-wok
    "${GAME_SOURCE_DIR}/main.cpp"
    "${GAME_SOURCE_DIR}/audio/SoundBank.cpp"
    "${GAME_SOURCE_DIR}/art/PlaceholderArt.cpp"
    "${GAME_SOURCE_DIR}/entities/Fire.cpp"
    "${GAME_SOURCE_DIR}/entities/Particle.cpp"
    "${GAME_SOURCE_DIR}/entities/Player.cpp"
    "${GAME_SOURCE_DIR}/entities/StageActor.cpp"
    "${GAME_SOURCE_DIR}/entities/WaterDrop.cpp"
    "${GAME_SOURCE_DIR}/systems/FireSystem.cpp"
    "${GAME_SOURCE_DIR}/systems/Sprayer.cpp"
    "${GAME_SOURCE_DIR}/scenes/GameplayScene.cpp"
    "${GAME_SOURCE_DIR}/scenes/IntroScene.cpp"
    "${GAME_SOURCE_DIR}/scenes/MenuScene.cpp"
    "${GAME_SOURCE_DIR}/scenes/ResultScene.cpp"
    "${GAME_SOURCE_DIR}/world/ForestMap.cpp"
)
target_include_directories(burn-the-wok PRIVATE "${GAME_SOURCE_DIR}")
target_link_libraries(burn-the-wok PRIVATE Engine)
set_target_properties(burn-the-wok PROPERTIES
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

# Keep all assets beside the executable, including narration and WAV files.
# Run the copy on every build so asset-only edits are picked up too.
add_custom_target(game-assets ALL
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Burn The Wok/assets"
        "${CMAKE_CURRENT_BINARY_DIR}/assets"
    COMMENT "Copying game assets"
    VERBATIM
)
add_dependencies(burn-the-wok game-assets)
