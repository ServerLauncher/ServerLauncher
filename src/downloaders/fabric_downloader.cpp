#include <string>
#include <vector>
#include <fstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <spdlog/spdlog.h>

#include "fabric_downloader.hpp"

const VersionList& FabricDownloader::getListOfMcVer() {
    if (!mc_cache.arr.empty()) return mc_cache;
    
    cpr::Response r = cpr::Get(cpr::Url{mc_version_url});

    if (r.status_code == 200) {
        auto json = nlohmann::json::parse(r.text);
        for (const auto& item : json) {
            mc_cache.arr.push_back(item["version"].get<std::string>());
        }
    } else {
        spdlog::error("Failed to fetch Minecraft versions(Fabric). Status code: {}, Message: {}", r.status_code, r.error.message);
        throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
    }
    spdlog::info("Fetched Minecraft {} versions(Fabric)", mc_cache.arr.size());
    return mc_cache;
}
const BuildList& FabricDownloader::getListOfBuild(const std::string& mc_version) {
    if (!build_cache.arr.empty()) return build_cache;
    
    cpr::Response r = cpr::Get(cpr::Url{build_url + "/" + mc_version});

    if (r.status_code == 200) {
        auto json = nlohmann::json::parse(r.text);
        for (const auto& item : json) {
            build_cache.arr.push_back(item["loader"]["version"].get<std::string>());
        }
    }else {
        spdlog::error("Failed to fetch Fabric loader versions for Minecraft {}. Status code: {}, Message: {}", mc_version, r.status_code, r.error.message);
        throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
    }
    return build_cache;
}
void FabricDownloader::downloadVersion(const VersionInfo& version){
    const FabricVersion& ver = static_cast<const FabricVersion&>(version);
    std::string path = instances_dir + "/fabric-server-" + "mc." + ver.version + "-" + "loader." + ver.build_version + "-" + "installer." + ver.installer_version + ".jar";
    
    if(!std::filesystem::exists(path)){
        cpr::Response r = cpr::Get(cpr::Url{build_url + "/" + ver.version + "/" + 
        ver.build_version + "/" + ver.installer_version + "/server/jar"});
        if (r.status_code == 200) {
            std::ofstream out(path, std::ios::binary);
            out.write(r.text.c_str(), r.text.size());
            out.close();
            spdlog::info("Downloaded to {}", path);
        } else{
            spdlog::error("Failed to download version: {}. Status code: {}, Message: {}", ver.version, r.status_code, r.error.message);
            throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
        }
    }
}
