#include <cpr/cpr.h>
#include <string>
#include <regex>
#include <unordered_set>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <sstream>

#include "forge_downloader.hpp"

const VersionList& ForgeDownloader::getListOfMcVer() {
    if (!mc_cache.arr.empty()) return mc_cache;

    if(raw_xml_cache.empty()){
        cpr::Response r = cpr::Get(cpr::Url(url));
        if (!r.status_code == 200) {
            spdlog::error("Failed to fetch Minecraft versions (Forge). Status code: {}, Message: {}", r.status_code, r.error.message);
            throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
        }
        raw_xml_cache = std::move(r.text);
    }

        std::regex pattern(R"(<version>([\d]+\.[\d]+(?:\.[\d]+)?)-[^<]+</version>)");
        auto begin = std::sregex_iterator(raw_xml_cache.begin(), raw_xml_cache.end(), pattern);
        auto end   = std::sregex_iterator();

        std::unordered_set<std::string> seen_versions;
        for (auto i = begin; i != end; ++i) {
            std::string ver = (*i)[1].str();
            if (seen_versions.insert(ver).second)
                mc_cache.arr.push_back(ver);
        }
        std::sort(mc_cache.arr.begin(), mc_cache.arr.end(), [](const std::string& a, const std::string& b) {
            auto parse = [](const std::string& ver) {
                std::vector<int> parts;
                std::stringstream ss(ver);
                std::string part;
                while (std::getline(ss, part, '.'))
                    parts.push_back(std::stoi(part));
                return parts;
            };
            return parse(a) > parse(b);
        });

    spdlog::info("Fetched {} Minecraft versions (Forge)", mc_cache.arr.size());
    return mc_cache;
}

const BuildList& ForgeDownloader::getListOfBuild(const std::string& mc_version) {
    auto it = build_cache_map.find(mc_version);
    if (it != build_cache_map.end()) {
        build_cache.arr = it->second;
        return build_cache;
    }
    
    if(raw_xml_cache.empty()){
        cpr::Response r = cpr::Get(cpr::Url(url));
        if (!r.status_code == 200) {
            spdlog::error("Failed to fetch builds (Forge). Status code: {}, Message: {}", r.status_code, r.error.message);
            throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
        }
        raw_xml_cache = std::move(r.text);
    }
        std::string prefix = mc_version + "-";

        std::regex pattern(R"(<version>([^<]+)</version>)");
        auto begin = std::sregex_iterator(raw_xml_cache.begin(), raw_xml_cache.end(), pattern);
        auto end   = std::sregex_iterator();

        std::vector<std::string> builds;
        std::unordered_set<std::string> seen_versions;

        for (auto it = begin; it != end; ++it) {
            std::string full_ver = (*it)[1].str();
            if (full_ver.starts_with(prefix) && seen_versions.insert(full_ver).second) {
                builds.push_back(full_ver);
            }
        }
        std::sort(builds.begin(), builds.end(), [](const std::string& a, const std::string& b) {
            auto getForgeNumeric = [](const std::string& ver) {
                std::string numeric = ver;
                auto dashPos = numeric.find('-');   //Remove the Minecraft version prefix
                if (dashPos != std::string::npos)
                    numeric = numeric.substr(dashPos + 1);
                std::vector<int> parts;
                std::stringstream ss(numeric);
                std::string part;
                while (std::getline(ss, part, '.')) {   //Split by dots and convert to integers for proper comparison
                    try { parts.push_back(std::stoi(part)); }
                    catch (...) { parts.push_back(0); }
                }
                return parts;
            };
            return getForgeNumeric(a) > getForgeNumeric(b);
        });

        build_cache_map[mc_version] = builds;
        build_cache.arr = builds;

    spdlog::info("Fetched {} builds for Minecraft {} (Forge)", build_cache.arr.size(), mc_version);
    return build_cache;
}

void ForgeDownloader::downloadVersion(const VersionInfo& version) {
    const ForgeVersion& ver = static_cast<const ForgeVersion&>(version);
    std::string filename = "forge-" + ver.build + "-installer.jar";
    std::string path = instances_dir + filename;

    if (!std::filesystem::exists(path)) {
        std::string dl_url = download_url + ver.build + "/" + filename;
        cpr::Response r = cpr::Get(cpr::Url(dl_url));
        if (r.status_code == 200) {
            std::ofstream out(path, std::ios::binary);
            out.write(r.text.c_str(), r.text.size());
            out.close();
            spdlog::info("Downloaded {} to {}", filename, path);
        } else {
            spdlog::error("Failed to download version: {}. Status code: {}, Message: {}", ver.build, r.status_code, r.error.message);
            throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
        }
    } else {
        spdlog::info("{} already exists at {}", filename, path);
    }
}