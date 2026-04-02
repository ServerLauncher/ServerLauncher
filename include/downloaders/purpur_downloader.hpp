#pragma once

#include <string>

#include "downloader.hpp"

struct PurpurVersion : public VersionInfo {
    PurpurVersion(std::string version, std::string build_version): version(version), build_version(build_version) 
    {}
    
    std::string version;
    std::string build_version;
};

class PurpurDownloader : public Downloader {
public:
    const VersionList& getListOfMcVer() override;
    const BuildList& getListOfBuild(const std::string& mc_version) override;
    void downloadVersion(const VersionInfo& version) override;
private:
    std::mutex mutex;
    VersionList mc_cache;
    BuildList build_cache;
    const std::string url = "https://api.purpurmc.org/v2/purpur/";
};