#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <spdlog/spdlog.h>

#include "purpur_downloader.hpp"

const VersionList& PurpurDownloader::getListOfMcVer() {
    if (!mc_cache.arr.empty()) return mc_cache;

    cpr::Response r = cpr::Get(cpr::Url(url));

    if (r.status_code == 200) {
        auto json = nlohmann::json::parse(r.text);
        for (const auto& item : json["versions"]) {
            mc_cache.arr.push_back(item.get<std::string>());
        }
        spdlog::info("Fetched Minecraft {} versions(Purpur)", mc_cache.arr.size());
    } else {
        spdlog::error("Failed to fetch Minecraft versions(Purpur). Status code: {}, Message: {}", r.status_code, r.error.message);
        throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
    }
    return mc_cache;
}
const BuildList& PurpurDownloader::getListOfBuild(const std::string& mc_version){
    if (!build_cache.arr.empty()) return build_cache;

    cpr::Response r = cpr::Get(cpr::Url(url + mc_version));

    if (r.status_code == 200) {
        auto json = nlohmann::json::parse(r.text);
        for (const auto& item : json["builds"]["all"]) {
            build_cache.arr.push_back(item.get<std::string>());
        }
        spdlog::info("Fetched {} builds for Minecraft {} (Purpur)", build_cache.arr.size(), mc_version);
    } else {
        spdlog::error("Failed to fetch builds (Purpur). Status code: {}, Message: {}", r.status_code, r.error.message);
        throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
    }
    return build_cache;
}
void PurpurDownloader::downloadVersion(const VersionInfo& version){
    const PurpurVersion& ver = static_cast<const PurpurVersion&>(version);
    std::string path = instances_dir + "purpur-" + ver.version + "-" + ver.build_version + ".jar";
    
    if(!std::filesystem::exists(path)){
        cpr::Response r = cpr::Get(cpr::Url(url + ver.version + "/" + ver.build_version + "/download"));
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