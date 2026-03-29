#pragma once

#include <string>

#include "downloader.hpp"

struct ForgeVersion : VersionInfo {
    ForgeVersion(std::string version, std::string build) : version(version), build(build) 
    {}

    std::string version;
    std::string build;
};

class ForgeDownloader : public Downloader {
public:
    const VersionList& getListOfMcVer() override;
    const BuildList& getListOfBuild(const std::string& mc_version) override;
    void downloadVersion(const VersionInfo& version) override;
private:
    VersionList mc_cache;
   BuildList build_cache;
    const std::string url = "https://maven.minecraftforge.net/releases/net/minecraftforge/forge/maven-metadata.xml";
    const std::string download_url = "https://maven.minecraftforge.net/net/minecraftforge/forge/";
};