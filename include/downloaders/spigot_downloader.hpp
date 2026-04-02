#pragma once

#include "downloader.hpp"

struct SpigotVersion : public VersionInfo {
    SpigotVersion(std::string version): version(version)
    {}
    
    std::string version;
};

class SpigotDownloader : public Downloader {
public:
    const VersionList& getListOfMcVer() override;
    const BuildList& getListOfBuild(const std::string& mc_version) override {
        throw std::logic_error("Spigot does not have separate builds");
    }
    void downloadVersion(const VersionInfo& version) override {
        throw std::logic_error("Use downloadBuild instead of it");
    }
    void downloadBuild(const VersionInfo& version);
private:
    std::mutex mutex;
    std::string raw_xml_cache;
    VersionList mc_cache;
    const std::string mc_version_url = "https://hub.spigotmc.org/nexus/content/repositories/snapshots/org/spigotmc/spigot-api/maven-metadata.xml";
    const std::string build_url = "https://hub.spigotmc.org/jenkins/job/BuildTools/lastSuccessfulBuild/artifact/target/BuildTools.jar";
};