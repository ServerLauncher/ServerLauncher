#include <string>
#include <regex>
#include <filesystem>
#include <cpr/cpr.h>
#include <spdlog/spdlog.h>
#include <cstdlib>

#include "spigot_downloader.hpp"

const VersionList& SpigotDownloader::getListOfMcVer() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!mc_cache.arr.empty()) return mc_cache;

    if(raw_xml_cache.empty()){
        cpr::Response r = cpr::Get(cpr::Url(mc_version_url));
        if(!r.status_code == 200){
            spdlog::error("Failed to fetch Minecraft versions(Spigot). Status code: {}, Message: {}", r.status_code, r.error.message);
            throw std::runtime_error("Failed to fetch Minecraft versions. Status code: " + std::to_string(r.status_code));
        }
        raw_xml_cache = std::move(r.text);
    }

    std::regex version_pattern(R"(<version>([\d]+\.[\d]+(?:\.[\d]+)?)[^<]*<\/version>)");
    auto begin = std::sregex_iterator(raw_xml_cache.begin(), raw_xml_cache.end(), version_pattern);
    auto end = std::sregex_iterator();

    std::unordered_set<std::string> seen_versions;
    for (auto i = begin; i != end; ++i) {
        std::string ver = (*i)[1].str();
        if(seen_versions.insert(ver).second) { //If it's not already seen
            mc_cache.arr.insert(mc_cache.arr.begin(), ver);
        }
    }

    spdlog::info("Fetched Minecraft {} versions(Spigot)", mc_cache.arr.size());
    return mc_cache;
}
void SpigotDownloader::downloadBuild(const VersionInfo& version) {
    const SpigotVersion& ver = static_cast<const SpigotVersion&>(version);
    std::string path = instances_dir + "BuildTools.jar";

    if(!std::filesystem::exists(path)){
        cpr::Response r = cpr::Get(cpr::Url(build_url));
        if (r.status_code == 200) {
            std::ofstream out(path, std::ios::binary);
            out.write(r.text.c_str(), r.text.size());
            out.close();
            spdlog::info("Downloaded to {}", path);
        } else{
            spdlog::error("Failed to download version: {}. Status code: {}, Message: {}", ver.version, r.status_code, r.error.message);        
            throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
        }
    } else {
        spdlog::info("BuildTools already exists at {}", path);
    }
    std::filesystem::current_path(instances_dir);
    std::string cmd = "java -jar BuildTools.jar --rev " + ver.version;
    spdlog::info("Running BuildTools for version {}", ver.version);
    std::system(cmd.c_str());
}