#pragma once
#include <QVector>
#include <QDateTime>

//index.json
struct MetaPlatform {
    QString uid;
    QString name;
    QString sha256;
    QString url;
};

struct MetaIndex {
    int formatVersion = 1;
    QDateTime generatedAt;
    QVector<MetaPlatform> platforms;

    const MetaPlatform* findPlatformByUid(const QString& uid) const {
        for(const auto& platform : platforms) {
            if(platform.uid == uid) {
                return &platform;
            }
        }
        return nullptr;
    }
};

//package.json
struct MetaBuild {
    QString mcVersion;
    QString sha256;
    QString url;
    QString type;
};

struct MetaPackage {
    int formatVersion = 1;
    QString uid;
    QString name;
    QVector<QString> recommended;
    QVector<MetaBuild> versions;

    const MetaBuild* findByMcVersion(const QString& mc_version) const {
        for(const auto& build : versions){
            if(build.mcVersion == mc_version)
                return &build;
        }
        return nullptr;
    }

    bool isRecommended(const QString& mc_version) const {
        return recommended.contains(mc_version);
    }
};

//version.json
struct MetaDownload {
    QString name;
    QString url;
    QString sha1;
    QString sha256;
};

struct MetaBuilds {
    QString build;
    QString type;
    QDateTime releaseTime;
    bool recommended = false;
    MetaDownload download;
};

struct MetaVersion {
    int formatVersion = 1;
    QString uid;
    QString mcVersion;
    QVector<MetaBuilds> builds;

    const MetaBuilds* findByBuild(const QString& build) const {
        for(const auto& b : builds) {
            if(b.build == build)
                return &b;
        }
        return nullptr;
    }
};