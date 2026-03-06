#include <string>
#include <vector>
#include <fstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "downloaders/fabric_downloader.hpp"

const VersionList& FabricDownloader::getListOfMcVer() {
    cpr::Response r = cpr::Get(cpr::Url{mc_version_url});

    if (r.status_code == 200) {
        auto json = nlohmann::json::parse(r.text);
        for (const auto& item : json) {
            mc_cache.arr.push_back(item["version"].get<std::string>());
        }
    } else {
        spdlog::error("Failed to fetch Minecraft versions. Status code: {}, Message: {}", r.status_code, r.error.message);
        throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
    }
    return mc_cache;
}
const LoaderVersionList& FabricDownloader::getListOfLoaderVer(const std::string& mc_version) {
    cpr::Response r = cpr::Get(cpr::Url{fabric_version_url + "/" + mc_version});

    if (r.status_code == 200) {
        auto json = nlohmann::json::parse(r.text);
        for (const auto& item : json) {
            loader_cache.arr.push_back(item["loader"]["version"].get<std::string>());
        }
    }else {
        spdlog::error("Failed to fetch Fabric loader versions for Minecraft {}. Status code: {}, Message: {}", mc_version, r.status_code, r.error.message);
        throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
    }
    return loader_cache;
}
void FabricDownloader::downloadVersion(const VersionInfo& version, const std::string& download_path){
    const FabricVersion& ver = static_cast<const FabricVersion&>(version);
    std::string path = download_path + "/fabric-server-" + "mc." + ver.version + "-" + "loader." + ver.loader + "-" + "installer." + ver.installer_ver + ".jar";

    cpr::Response r = cpr::Get(cpr::Url{fabric_version_url + "/" + ver.version + "/" + 
        ver.loader + "/" + ver.installer_ver + "/server/jar"});

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
