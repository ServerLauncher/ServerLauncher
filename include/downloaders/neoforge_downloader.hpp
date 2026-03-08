#pragma once

#include <string>

#include "downloader.hpp"

struct NeoForgeVersion : public VersionInfo {
    NeoForgeVersion(std::string build_version): build_version(build_version) 
    {}
    
    std::string build_version;
};

class NeoForgeDownloader : public Downloader {
public:
    const VersionList& getListOfMcVer() override;
    const LoaderVersionList& getListOfLoaderVer(const std::string& mc_version) override;
    void downloadVersion(const VersionInfo& version) override;
private:
    VersionList mc_cache;
    LoaderVersionList loader_cache;
    const std::string url = "https://maven.neoforged.net/releases/net/neoforged/neoforge/";
};