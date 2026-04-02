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
    const BuildList& getListOfBuild(const std::string& mc_version) override;
    void downloadVersion(const VersionInfo& version) override;
private:
    std::mutex mutex;
    std::string raw_xml_cache;
    std::unordered_map<std::string, std::vector<std::string>> build_cache_map;
    VersionList mc_cache;
    BuildList build_cache;
    const std::string url = "https://maven.neoforged.net/releases/net/neoforged/neoforge/";
};