#include "MetaParser.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

bool MetaParser::parseIndex(const QByteArray& data, MetaIndex& index, QString& errorMessage) {
    auto doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        errorMessage = "Failed to parse JSON";
        return false;
    }

    const auto root = doc.object();

    const int formatVersion = root.value("formatVersion").toInt(-1);
    if (formatVersion != 1) {
        errorMessage = QString("Unsupported formatVersion: %1").arg(formatVersion);
        return false;
    }

    index.formatVersion = formatVersion;
    index.generatedAt = QDateTime::fromString(root.value("generatedAt").toString(), Qt::ISODate);
    const auto platforms = root.value("platforms").toArray();

    index.platforms.clear();

    for(const auto& platform : platforms) {
        const auto obj = platform.toObject();
        MetaPlatform info;
        
        info.uid = obj.value("uid").toString();
        info.name = obj.value("name").toString();
        info.sha256 = obj.value("sha256").toString();
        info.url = obj.value("url").toString();
        index.platforms.push_back(std::move(info));
    }

    return true;
}

bool MetaParser::parsePackage(const QByteArray& data, MetaPackage& package, QString& errorMessage){
    auto doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        errorMessage = "Failed to parse JSON";
        return false;
    }

    const auto root = doc.object();

    const int formatVersion = root.value("formatVersion").toInt(-1);
    if (formatVersion != 1) {
        errorMessage = QString("Unsupported formatVersion: %1").arg(formatVersion);
        return false;
    }

    package.formatVersion = formatVersion;
    package.uid = root.value("uid").toString();
    package.name = root.value("name").toString();

    package.recommended.clear();
    for(const auto& entry : root.value("recommended").toArray()){
        package.recommended.push_back(entry.toString());
    }

    package.versions.clear();
    for(const auto& entry : root.value("versions").toArray()){
        const auto obj = entry.toObject();
        MetaBuild build;
        build.mcVersion = obj.value("mcVersion").toString();
        build.sha256 = obj.value("sha256").toString();
        build.type = obj.value("type").toString();
        build.url = obj.value("url").toString();
        package.versions.push_back(std::move(build));
    }

    return true;
}