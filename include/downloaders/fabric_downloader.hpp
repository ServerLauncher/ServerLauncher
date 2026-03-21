#pragma once

#include <string>
#include "downloader.hpp"

struct FabricVersion : public VersionInfo {
    FabricVersion(std:: string version, std::string build_version, std::string installer_version): version(version), build_version(build_version), installer_version(installer_version) 
    {}
    
    std::string version;
    std::string build_version;
    std::string installer_version;
};

class FabricDownloader : public Downloader {
public:
    const VersionList& getListOfMcVer() override;
    const BuildList& getListOfBuild(const std::string& mc_version) override;
    void downloadVersion(const VersionInfo& version) override;
private:
    VersionList mc_cache;
    BuildList build_cache;
    const std::string mc_version_url = "https://meta.fabricmc.net/v2/versions/game";
    const std::string build_url = "https://meta.fabricmc.net/v2/versions/loader";
};