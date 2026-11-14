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
inline constexpr float SawitHeight = TreeHeight * 0.9f; // oil palm, a bit lower than forest trees

// --- Fire (Stage 2) ---
inline constexpr int InitialFireCount = 6;
inline constexpr unsigned FireSeed = 7;           // which trees start burning
inline constexpr float FireMaxIntensity = 100.0f; // "HP" of a fire
inline constexpr float FireStartIntensity = 60.0f;
inline constexpr float FireGrowthPerSecond = 6.0f; // a fire left alone grows back to full
inline constexpr float FireRegrowDelay = 1.0f;     // seconds without water before it grows again

// --- Spreading, burnt land and stage rules (Stage 3) ---
inline constexpr float SpreadInterval = 12.0f;            // seconds between spreads of one strong fire
inline constexpr float SpreadMinIntensityFraction = 0.5f; // weaker fires do not spread (keep spraying!)
inline constexpr int SpreadRadiusTiles = 2;               // a spark can jump to trees this many tiles away
inline constexpr float BurnToSawitSeconds = 30.0f;        // at full intensity; weak fires take up to 2x longer
inline constexpr float StageDuration = 150.0f;            // seconds
inline constexpr float SawitLoseFraction = 0.15f;         // lose when this share of the forest is sawit
inline constexpr float ResultDelay = 1.5f;                // pause before the win/lose screen

// --- Water sprayer & tank (Stage 2) ---
inline constexpr float TankCapacity = 100.0f;      // water units
inline constexpr float SprayDropsPerSecond = 30.0f;
inline constexpr float WaterPerDrop = 0.4f;        // tank units per drop -> 12 units/s, ~8 s of spray
inline constexpr float DropPower = 2.5f;           // fire intensity removed per drop that hits
inline constexpr float DropSpeed = 170.0f;         // world pixels per second
inline constexpr float DropLifetime = 0.42f;       // seconds -> range of roughly 4 tiles
inline constexpr float SpraySpreadDegrees = 7.0f;  // random cone half-angle
inline constexpr float RefillPerSecond = 45.0f;    // while standing at the pond edge
inline constexpr float RefillReach = 4.0f;         // pixels the hitbox is grown by to "touch" water

// Integer zoom keeps every texel the same size on screen.
inline constexpr int DefaultZoom = 3;
inline constexpr int MinZoom = 1;
inline constexpr int MaxZoom = 6;

// --- Sound levels (0..1) and timings. Tune by ear. ---
namespace sound {
inline constexpr float GameplayMusic = 0.45f;
inline constexpr float MenuMusic = 0.7f;
inline constexpr float StageStart = 0.8f;
inline constexpr float Footstep = 0.35f;
inline constexpr float StepInterval = 0.26f;    // seconds between two steps while walking
inline constexpr float Spray = 0.5f;            // water_spray loop while water comes out
inline constexpr float Refill = 0.6f;
inline constexpr float RefillInterval = 0.7f;   // the refill sound is ~0.7 s long
inline constexpr float TankEmpty = 0.6f;
inline constexpr float TankEmptyInterval = 0.8f;
inline constexpr float FireLoopMin = 0.06f;     // fire_loop: far away, one fire
inline constexpr float FireLoopMax = 0.4f;      // fire_loop: close, many fires
inline constexpr float FireHearingRange = 160.0f; // world pixels: fires further away are quiet
inline constexpr float FireOut = 0.8f;
inline constexpr float FireSpread = 0.7f;
inline constexpr float TreeToPalm = 0.9f;
inline constexpr float TimerWarning = 0.45f;
inline constexpr float TimerWarningSeconds = 10.0f; // the ticking starts at this many seconds left
inline constexpr float Result = 0.9f;
inline constexpr float UiClick = 0.7f;
inline constexpr float UiHover = 0.4f;
} // namespace sound

} // namespace game::config
