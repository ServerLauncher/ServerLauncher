#pragma once

#include <string>
#include <iostream>
#include <vector>

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
    virtual const LoaderVersionList& getListOfLoaderVer(const std::string& mc_version) = 0;
    virtual void downloadVersion(const VersionInfo& version, const std::string& download_path) = 0;
    virtual const VersionList& getListOfMcVer() = 0;
};