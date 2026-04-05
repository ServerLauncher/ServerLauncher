#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <spdlog/spdlog.h>

#include "paper_downloader.hpp"

const VersionList& PaperDownloader::getListOfMcVer() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!mc_cache.arr.empty()) return mc_cache;

    if (raw_json_cache.empty()) {
        cpr::Response r = cpr::Get(cpr::Url(versions_url));
        if (r.status_code != 200) {
            spdlog::error("Failed to fetch Minecraft versions (Paper). Status code: {}, Message: {}", r.status_code, r.error.message);
            throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
        }
        raw_json_cache = std::move(r.text);
    }

    auto json = nlohmann::json::parse(raw_json_cache);
    for (const auto& item : json["versions"])
        mc_cache.arr.insert(mc_cache.arr.begin(), item.get<std::string>());

    spdlog::info("Fetched {} Minecraft versions (Paper)", mc_cache.arr.size());
    return mc_cache;
}

const BuildList& PaperDownloader::getListOfBuild(const std::string& mc_version) {
    std::lock_guard<std::mutex> lock(mutex);

    auto it = build_cache_map.find(mc_version);
    if (it != build_cache_map.end()) {
        build_cache.arr = it->second;
        return build_cache;
    }

    cpr::Response r = cpr::Get(cpr::Url(build_url + mc_version + "/builds"));

    if (r.status_code == 200) {
        auto json = nlohmann::json::parse(r.text);
        for (const auto& item : json["builds"])
            build_cache.arr.insert(build_cache.arr.begin(), std::to_string(item["build"].get<int>()));

        spdlog::info("Fetched {} builds for Minecraft {} (Paper)", build_cache.arr.size(), mc_version);
    } else {
        spdlog::error("Failed to fetch builds (Paper). Status code: {}, Message: {}", r.status_code, r.error.message);
        throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
    }
    
    build_cache_map[mc_version] = build_cache.arr;

    return build_cache;
}

void PaperDownloader::downloadVersion(const VersionInfo& version) {
    const PaperVersion& ver = static_cast<const PaperVersion&>(version);
    std::string filename = "paper-" + ver.version + "-" + ver.build_version + ".jar";
    std::string path = instances_dir + filename;

    if (!std::filesystem::exists(path)) {
        std::string download_url = build_url + ver.version + "/builds/" + ver.build_version + "/downloads/" + filename;
        cpr::Response r = cpr::Get(cpr::Url(download_url));
        if (r.status_code == 200) {
            std::ofstream out(path, std::ios::binary);
            out.write(r.text.c_str(), r.text.size());
            out.close();
            spdlog::info("Downloaded to {}", path);
        } else {
            spdlog::error("Failed to download version: {}. Status code: {}, Message: {}", ver.version, r.status_code, r.error.message);
            throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
        }
    } else {
        spdlog::info("{} already exists at {}", filename, path);
    }
}