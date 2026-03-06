#include <iostream>

#include "fabric_downloader.hpp"

int main(){
     FabricVersion ver("1.21.11", "0.18.4", "1.0.1");
     FabricDownloader downloader;
     downloader.downloadVersion(ver, "C:/Users/User/Downloads");
     auto& versions = downloader.getListOfMcVer();
     std::cout << versions;
}