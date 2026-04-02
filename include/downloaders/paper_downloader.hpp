#pragma once

#include <string>

#include "downloader.hpp"

struct PaperVersion : public VersionInfo {
    PaperVersion(std::string version, std::string build_version): version(version), build_version(build_version) 
    {}
    
    std::string version;
    std::string build_version;
};

class PaperDownloader : public Downloader {
public:
    const VersionList& getListOfMcVer() override;
    const BuildList& getListOfBuild(const std::string& mc_version) override;
    void downloadVersion(const VersionInfo& version) override;
private:
    std::mutex mutex;
    VersionList mc_cache;
    BuildList build_cache;
    const std::string versions_url = "https://api.papermc.io/v2/projects/paper/";
    const std::string build_url = "https://api.papermc.io/v2/projects/paper/versions/";
};