#include "MetaParser.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>

bool MetaParser::parseIndex(const QByteArray& data, MetaIndex& index, QString& errorMessage) {
    QJsonParseError parseError;
    auto doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = QString("JSON parse error: %1 (offset %2)")
            .arg(parseError.errorString()).arg(parseError.offset);
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
    if (!index.generatedAt.isValid()) {
        qWarning() << "MetaParser::parseIndex: invalid generatedAt timestamp";
    }

    const auto platforms = root.value("platforms").toArray();
    index.platforms.clear();

    for (const auto& platform : platforms) {
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

bool MetaParser::parsePackage(const QByteArray& data, MetaPackage& package, QString& errorMessage) {
    QJsonParseError parseError;
    auto doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = QString("JSON parse error: %1 (offset %2)")
            .arg(parseError.errorString()).arg(parseError.offset);
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
    for (const auto& entry : root.value("recommended").toArray()) {
        package.recommended.push_back(entry.toString());
    }

    package.versions.clear();
    for (const auto& entry : root.value("versions").toArray()) {
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

bool MetaParser::parseVersion(const QByteArray& data, MetaVersion& version, QString& errorMessage) {
    QJsonParseError parseError;
    auto doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = QString("JSON parse error: %1 (offset %2)")
            .arg(parseError.errorString()).arg(parseError.offset);
        return false;
    }

    const auto root = doc.object();

    const int formatVersion = root.value("formatVersion").toInt(-1);
    if (formatVersion != 1) {
        errorMessage = QString("Unsupported formatVersion: %1").arg(formatVersion);
        return false;
    }

    version.formatVersion = formatVersion;
    version.uid = root.value("uid").toString();
    version.mcVersion = root.value("mcVersion").toString();

    version.builds.clear();
    for (const auto& entry : root.value("builds").toArray()) {
        const auto obj = entry.toObject();
        MetaBuilds builds;
        builds.build = obj.value("build").toString();
        builds.type = obj.value("type").toString();
        builds.releaseTime = QDateTime::fromString(obj.value("releaseTime").toString(), Qt::ISODate);
        if (!builds.releaseTime.isValid()) {
            qWarning() << "MetaParser::parseVersion: invalid releaseTime for build" << builds.build;
        }
        builds.recommended = obj.value("recommended").toBool();

        auto dl = obj.value("download").toObject();
        builds.download.name = dl.value("name").toString();
        builds.download.url = dl.value("url").toString();
        builds.download.sha1 = dl.value("sha1").isNull() ? QString() : dl.value("sha1").toString();
        builds.download.sha256 = dl.value("sha256").isNull() ? QString() : dl.value("sha256").toString();
        builds.download.md5 = dl.value("md5").isNull() ? QString() : dl.value("md5").toString();
        version.builds.push_back(std::move(builds));
    }

    return true;
}