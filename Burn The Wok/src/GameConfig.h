#pragma once

// Tuning values for the game. Everything here is a PLACEHOLDER for the Stage 1 demo;
// stage/balancing data moves to a data file in Stage 5 (see docs/03-gameplay-design.md).
namespace game::config {

inline constexpr const char* WindowTitle = "Burn The Wo";
inline constexpr int WindowWidth = 1280;
inline constexpr int WindowHeight = 720;

inline constexpr int TileSize = 16; // world pixels per tile (final size to be agreed with the art team)
inline constexpr int MapWidthTiles = 48;
inline constexpr int MapHeightTiles = 32;
inline constexpr unsigned MapSeed = 2026; // same seed -> same placeholder map every run

inline constexpr float PlayerSpeed = 80.0f; // world pixels per second
inline constexpr float PlayerHeight = 26.0f;
// A compact top-down perspective: trees stand clearly above the ranger.
inline constexpr float TreeHeight = PlayerHeight * 1.6f;

// Integer zoom keeps every texel the same size on screen.
inline constexpr int DefaultZoom = 3;
inline constexpr int MinZoom = 1;
inline constexpr int MaxZoom = 6;

} // namespace game::config
