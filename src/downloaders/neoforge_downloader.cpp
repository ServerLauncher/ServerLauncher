#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>
#include <regex>
#include <filesystem>
#include <algorithm>
#include <sstream>

#include "neoforge_downloader.hpp"

const VersionList& NeoForgeDownloader::getListOfMcVer() {
    std::lock_guard<std::mutex> lock(mutex);
    if(!mc_cache.arr.empty()) return mc_cache;
    
    if(raw_xml_cache.empty()){
        cpr::Response r = cpr::Get(cpr::Url(url + "maven-metadata.xml"));
        if(!r.status_code == 200){
            spdlog::error("Failed to fetch Minecraft versions (NeoForge). Status code: {}, Message: {}", r.status_code, r.error.message);
            throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
        }
        raw_xml_cache = std::move(r.text);
    }

    std::regex pattern(R"(<version>(\d+)\.(\d+)\.\d+[^<]*</version>)");
    auto begin = std::sregex_iterator(raw_xml_cache.begin(), raw_xml_cache.end(), pattern);
    auto end = std::sregex_iterator();

    std::unordered_set<std::string> seen_versions;
    std::string ver;

    for (auto i = begin; i != end; ++i) {
        if(std::stoi((*i)[1].str()) >= 26)
            ver = (*i)[1].str() + ((*i)[2].str() == "0" ? "" : "." + (*i)[2].str());
        else
            ver = "1." + (*i)[1].str() + ((*i)[2].str() == "0" ? "" : "." + (*i)[2].str());
            
        if(seen_versions.insert(ver).second)
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

    spdlog::info("Fetched {} Minecraft versions (NeoForge)", mc_cache.arr.size());
    return mc_cache;
}
const BuildList& NeoForgeDownloader::getListOfBuild(const std::string& mc_version) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = build_cache_map.find(mc_version);
    if (it != build_cache_map.end()) {
        build_cache.arr = it->second;
        return build_cache;
    }

    if(raw_xml_cache.empty()){
        cpr::Response r = cpr::Get(cpr::Url(url + "maven-metadata.xml"));
         if (!r.status_code == 200){
            spdlog::error("Failed to fetch builds (NeoForge). Status code: {}, Message: {}", r.status_code, r.error.message);
            throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
         }
         raw_xml_cache = std::move(r.text);
    }
        std::string match_prefix;
        if (mc_version.substr(0, 2) == "1.") {
            std::string without_one = mc_version.substr(2);
            if (without_one.find('.') == std::string::npos)
                match_prefix = without_one + ".0";
            else
                match_prefix = without_one;
        } else {
            match_prefix = mc_version;
        }
        std::regex pattern(R"(<version>((\d+)\.(\d+)\.[^<]+)</version>)");
        auto begin = std::sregex_iterator(raw_xml_cache.begin(), raw_xml_cache.end(), pattern);
        auto end   = std::sregex_iterator();

        std::unordered_set<std::string> seen_versions;
        std::vector<std::string> builds;

        for (auto it = begin; it != end; ++it) {
            std::string full_ver   = (*it)[1].str();
            std::string ver_prefix = (*it)[2].str() + "." + (*it)[3].str();
            if (ver_prefix == match_prefix && seen_versions.insert(full_ver).second) {
                builds.push_back(full_ver);
            }
        }
        std::sort(builds.begin(), builds.end(), [](const std::string& a, const std::string& b) {
            auto getNumeric = [](const std::string& ver) {
                std::string numeric = ver;
                auto dashPos = numeric.rfind('-');   //Remove the Minecraft version prefix
                if (dashPos != std::string::npos && !std::isdigit(numeric[dashPos + 1]))
                    numeric = numeric.substr(0, dashPos);
                std::vector<int> parts;
                std::stringstream ss(numeric);
                std::string part;
                while (std::getline(ss, part, '.')) {   //Split by dots and convert to integers for proper comparison
                    try { parts.push_back(std::stoi(part)); }
                    catch (...) { parts.push_back(0); }
                }
                return parts;
            };
            return getNumeric(a) > getNumeric(b);
        });

        build_cache_map[mc_version] = builds;
        build_cache.arr = builds;

    spdlog::info("Fetched {} builds for Minecraft {} (NeoForge)", build_cache.arr.size(), mc_version);
    return build_cache;
}
void NeoForgeDownloader::downloadVersion(const VersionInfo& version) {
    const NeoForgeVersion& ver = static_cast<const NeoForgeVersion&>(version);
    std::string neoforge_installer = "neoforge-" + ver.build_version + "-installer.jar";
    std::string path = instances_dir + neoforge_installer;

    if (!std::filesystem::exists(path)) {
        cpr::Response r = cpr::Get(cpr::Url(url + ver.build_version + "/" + neoforge_installer));
        if (r.status_code == 200) {
            std::ofstream out(path, std::ios::binary);
            out.write(r.text.c_str(), r.text.size());
            out.close();
            spdlog::info("Downloaded {} to {}", neoforge_installer, path);
        } else {
            spdlog::error("Failed to download version: {}. Status code: {}, Message: {}", ver.build_version, r.status_code, r.error.message);
            throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
        }
    } else {
        spdlog::info("{} already exists at {}", neoforge_installer, path);
    }
}