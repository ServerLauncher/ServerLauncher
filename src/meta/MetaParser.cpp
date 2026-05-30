#include "MetaParser.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

bool MetaParser::parse(const QByteArray& data, MetaIndex& index, QString& errorMessage) {
    auto doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        errorMessage = "Failed to parse JSON";
        return false;
    }

    const auto root = doc.object();

    index.formatVersion = root.value("formatVersion").toInt();
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