#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <spdlog/spdlog.h>

struct VersionList {
    std::vector<std::string> arr;
};
struct LoaderVersionList {
    std::vector<std::string> arr;
};
struct VersionInfo {
    virtual ~VersionInfo() = default;
};
template <typename T>
concept HasArrVector = requires(T t) {
    { t.arr } -> std::convertible_to<std::vector<std::string>>;
};
template <HasArrVector T>
inline std::ostream& operator<<(std::ostream& os, const T& list) {
    for (const auto& ver : list.arr) {
        os << ver << "\n";
    }
    return os;
}

class Downloader {
public:
    Downloader() {
        const char* appdata = std::getenv("LOCALAPPDATA");
        if (!appdata) {
            throw std::runtime_error("LOCALAPPDATA environment variable not found");
        }
        work_dir = std::string(appdata) + "/MSC/";
        instances_dir = work_dir + "instances/";
        std::filesystem::create_directories(work_dir);
        spdlog::info("Created {} directory", work_dir);
        std::filesystem::create_directories(instances_dir);
        spdlog::info("Created {} directory", instances_dir);
    }
    virtual const LoaderVersionList& getListOfLoaderVer(const std::string& mc_version) = 0;
    virtual void downloadVersion(const VersionInfo& version) = 0;
    virtual const VersionList& getListOfMcVer() = 0;
protected:
    std::string work_dir;
    std::string instances_dir;
};