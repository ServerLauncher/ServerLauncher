#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>
#include <regex>
#include <filesystem>
#include <algorithm>
#include <sstream>

#include "neoforge_downloader.hpp"

const VersionList& NeoForgeDownloader::getListOfMcVer() {
    if(!mc_cache.arr.empty()) return mc_cache;
    
    cpr::Response r = cpr::Get(cpr::Url(url + "maven-metadata.xml"));
    std::string ver;

    if(r.status_code == 200){
        std::regex pattern(R"(<version>(\d+)\.(\d+)\.\d+[^<]*</version>)");
        auto begin = std::sregex_iterator(r.text.begin(), r.text.end(), pattern);
        auto end = std::sregex_iterator();

        std::unordered_set<std::string> seen_versions;
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
            return parse(a) < parse(b);
        });
    }else{
        spdlog::error("Failed to fetch Minecraft versions (NeoForge). Status code: {}, Message: {}", r.status_code, r.error.message);
        throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
    }
    spdlog::info("Fetched {} Minecraft versions (NeoForge)", mc_cache.arr.size());
    return mc_cache;
}
const BuildList& NeoForgeDownloader::getListOfBuild(const std::string& mc_version) {
    if (!build_cache.arr.empty()) return build_cache;

    cpr::Response r = cpr::Get(cpr::Url(url + "maven-metadata.xml"));

    if (r.status_code == 200) {
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
        auto begin = std::sregex_iterator(r.text.begin(), r.text.end(), pattern);
        auto end   = std::sregex_iterator();

        std::unordered_set<std::string> seen_versions;
        for (auto it = begin; it != end; ++it) {
            std::string full_ver   = (*it)[1].str();
            std::string ver_prefix = (*it)[2].str() + "." + (*it)[3].str();
            if (ver_prefix == match_prefix && seen_versions.insert(full_ver).second) {
                build_cache.arr.push_back(full_ver);
            }
        }
        std::sort(build_cache.arr.begin(), build_cache.arr.end(), [](const std::string& a, const std::string& b) {
            auto getNumeric = [](const std::string& ver) {
                std::string numeric = ver;
                auto dashPos = numeric.rfind('-');
                if (dashPos != std::string::npos && !std::isdigit(numeric[dashPos + 1]))
                    numeric = numeric.substr(0, dashPos);
                std::vector<int> parts;
                std::stringstream ss(numeric);
                std::string part;
                while (std::getline(ss, part, '.')) {
                    try { parts.push_back(std::stoi(part)); }
                    catch (...) { parts.push_back(0); }
                }
                return parts;
            };
            return getNumeric(a) < getNumeric(b);
        });
    } else {
        spdlog::error("Failed to fetch builds (NeoForge). Status code: {}, Message: {}", r.status_code, r.error.message);
        throw std::runtime_error(std::to_string(r.status_code) + " " + r.error.message);
    }

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