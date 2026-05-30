#pragma once
#include <QVector>
#include <QDateTime>

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

    const MetaPlatform* findPlatformbyUid(const QString& uid) const {
        for(const auto& platform : platforms) {
            if(platform.uid == uid) {
                return &platform;
            }
        }
        return nullptr;
    }
};