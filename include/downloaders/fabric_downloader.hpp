#pragma once

#include <string>
#include "downloader.hpp"

struct FabricVersion : public VersionInfo {
    FabricVersion(std:: string version, std::string loader, std::string installer_ver): version(version), loader(loader), installer_ver(installer_ver) 
    {}
    
    std::string version;
    std::string loader;
    std::string installer_ver;
};

class FabricDownloader : public Downloader {
public:
    const VersionList& getListOfMcVer() override;
    const LoaderVersionList& getListOfLoaderVer(const std::string& mc_version) override;
    void downloadVersion(const VersionInfo& version, 
                         const std::string& download_path) override;
private:
    VersionList mc_cache;
    LoaderVersionList loader_cache;
    const std::string mc_version_url = "https://meta.fabricmc.net/v2/versions/game";
    const std::string fabric_version_url = "https://meta.fabricmc.net/v2/versions/loader";
};