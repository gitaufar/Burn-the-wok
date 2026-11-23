#include "progress/SaveData.h"

#include <engine/core/KeyValueFile.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <vector>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace game {
namespace {
using engine::KeyValueFile;
namespace fs = std::filesystem;

std::vector<fs::path> Paths() {
    std::vector<fs::path> paths;
#ifdef _WIN32
    wchar_t executable[32768];
    const DWORD count = GetModuleFileNameW(nullptr, executable, 32768);
    if (count && count < 32768) paths.push_back(fs::path(executable).parent_path() / "save.txt");
    wchar_t appData[32768];
    const DWORD appDataCount = GetEnvironmentVariableW(L"APPDATA", appData, 32768);
    if (appDataCount && appDataCount < 32768) paths.push_back(fs::path(appData) / "BurnTheWo" / "save.txt");
#else
    std::error_code error;
    const auto executable = fs::read_symlink("/proc/self/exe", error);
    if (!error) paths.push_back(executable.parent_path() / "save.txt");
    const auto* dataHome = std::getenv("XDG_DATA_HOME");
    const auto* home = std::getenv("HOME");
    if (dataHome && fs::path(dataHome).is_absolute()) {
        paths.push_back(fs::path(dataHome) / "BurnTheWo" / "save.txt");
    } else if (home && *home) {
        paths.push_back(fs::path(home) / ".local" / "share" / "BurnTheWo" / "save.txt");
    }
#endif
    return paths;
}

std::string StageKey(int index) { return "stage" + std::to_string(index+1); }

SaveData Load() {
    KeyValueFile::Values values;
    fs::path latest;
    fs::file_time_type latestTime{};
    for (const auto& path : Paths()) {
        std::error_code error;
        if (!fs::is_regular_file(path, error)) continue;
        const auto written = fs::last_write_time(path, error);
        if (!error && (latest.empty() || written > latestTime)) {
            latest = path;
            latestTime = written;
        }
    }
    if (!latest.empty() && !KeyValueFile::Read(latest, values)) return {};
    bool valid = true;
    auto read = [&](const std::string& key, int fallback, int low, int high) {
        const auto it = values.find(key);
        if (it == values.end()) return fallback;
        int number = 0;
        const auto* end = it->second.data() + it->second.size();
        const auto result = std::from_chars(it->second.data(), end, number);
        if (result.ec != std::errc{} || result.ptr != end || number < low || number > high) valid = false;
        return number;
    };
    SaveData data;
    data.points = read("points", 0, 0, std::numeric_limits<int>::max());
    data.unlocked = read("stage_unlocked", 1, 1, 3)-1;
    for (int i = 0; i < 3; ++i) {
        data.stars[i] = read(StageKey(i)+"_best_stars", 0, 0, 3);
        data.cleared[i] = read(StageKey(i)+"_cleared", 0, 0, 1) != 0;
    }
    data.storeNew = read("store_new", 0, 0, 1) != 0;
    for (int i = 0; i < Local; ++i)
        data.upgrades[i] = read(Supplies[i].key, 0, 0, Supplies[i].maxLevel);
    data.campaignTier = read("campaign_tier", 0, 0, 2);
    const auto duration = values.find("campaign_seconds_left");
    if (duration != values.end()) {
        const auto* end = duration->second.data() + duration->second.size();
        const auto parsed = std::from_chars(duration->second.data(), end, data.campaignSeconds);
        valid &= parsed.ec == std::errc{} && parsed.ptr == end && std::isfinite(data.campaignSeconds) &&
                 data.campaignSeconds >= 0 && data.campaignSeconds <= 2*CampaignSeconds;
    }
    if (data.campaignTier == 0) data.campaignSeconds = 0;
    return valid ? data : SaveData{};
}
}

SaveData& SaveData::Current() {
    static SaveData data = Load();
    return data;
}

bool SaveData::Commit(const SaveData& next) {
    KeyValueFile::Values values;
    auto write = [&](const std::string& key, int value) { values[key] = std::to_string(value); };
    write("points", next.points);
    write("stage_unlocked", next.unlocked+1);
    for (int i = 0; i < 3; ++i) {
        write(StageKey(i)+"_best_stars", next.stars[i]);
        write(StageKey(i)+"_cleared", next.cleared[i]);
    }
    write("store_new", next.storeNew);
    for (int i = 0; i < Local; ++i) write(Supplies[i].key, next.upgrades[i]);
    write("campaign_tier", next.campaignSeconds > 0 ? next.campaignTier : 0);
    values["campaign_seconds_left"] = std::to_string(next.campaignSeconds);
    const auto paths = Paths();
    for (std::size_t i = 0; i < paths.size(); ++i) {
        std::error_code error;
        fs::create_directories(paths[i].parent_path(), error);
        if (KeyValueFile::Write(paths[i], values)) {
            *this = next;
            return true;
        }
    }
    return false;
}

std::string CampaignClock(float campaignSeconds) {
    const int seconds = static_cast<int>(std::ceil(campaignSeconds));
    return std::to_string(seconds / 60) + ":" + (seconds % 60 < 10 ? "0" : "") +
           std::to_string(seconds % 60);
}
}
