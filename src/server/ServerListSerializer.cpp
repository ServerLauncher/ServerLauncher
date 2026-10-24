#include "ServerListSerializer.hpp"
#include "BaseServer.hpp"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

bool ServerListSerializer::loadFromFile(const QString &filePath, ServerList &list)
{
    QFile file(filePath);
    if (!file.exists())
        return true;

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "[ServerList] Cannot open" << filePath;
        return false;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    file.close();

    if (err.error != QJsonParseError::NoError)
    {
        qWarning() << "[ServerList] Parse error:" << err.errorString();
        return false;
    }

    QJsonObject root = doc.object();
    int version = root.value("formatVersion").toInt();
    if (version != FORMAT_VERSION)
    {
        qWarning() << "[ServerList] Unknown format version:" << version;
        return false;
    }

    for (const auto &item : root.value("servers").toArray())
    {
        auto *server = serverFromJson(item.toObject(), list.rootDir());
        if (server)
            list.addServer(server);
        else
            qWarning() << "[ServerList] Failed to load server from JSON";
    }

    return true;
}

bool ServerListSerializer::saveToFile(const QString &filePath, const ServerList &list)
{
    QJsonArray arr;
    for (auto *server : list.servers())
        arr.append(serverToJson(server));

    QJsonObject root;
    root["formatVersion"] = FORMAT_VERSION;
    root["servers"] = arr;

    QFileInfo(filePath).dir().mkpath(".");

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "[ServerList] Cannot write to" << filePath;
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

BaseServer *ServerListSerializer::serverFromJson(const QJsonObject &obj, const QString &rootDir)
{
    QString uuid = obj.value("uuid").toString();
    if (uuid.isEmpty())
    {
        qWarning() << "[ServerList] Server entry has no uuid, skipping";
        return nullptr;
    }

    QString serverDir = QDir(rootDir).filePath(uuid);
    if (!QDir(serverDir).exists())
    {
        qWarning() << "[ServerList] Directory missing for server:" << uuid;
        return nullptr;
    }

    auto *s = new BaseServer(serverDir);

    s->setUuid(uuid);
    s->setName(obj.value("name").toString("Unnamed Server"));
    s->setGroup(obj.value("group").toString());
    s->setIconKey(obj.value("iconKey").toString("default"));
    s->setPlatformName(obj.value("platform").toString());
    s->setMcVersion(obj.value("mcVersion").toString());
    s->setBuildVersion(obj.value("buildVersion").toString());
    s->setDownloadUrl(obj.value("downloadUrl").toString());
    s->setSha256(obj.value("sha256").toString());
    s->setSha1(obj.value("sha1").toString());
    s->setTotalTimeLaunched(obj.value("totalTimeLaunched").toInteger(0));
    s->setLastTimeLaunched(obj.value("lastTimeLaunched").toInteger(0));
    s->setCreatedAt(QDateTime::fromString(obj.value("createdAt").toString(), Qt::ISODate));
    s->setLastLaunchedAt(QDateTime::fromString(obj.value("lastLaunchedAt").toString(), Qt::ISODate));

    s->saveProperties();

    return s;
}

QJsonObject ServerListSerializer::serverToJson(const BaseServer *server)
{
    QJsonObject obj;

    obj["uuid"] = server->uuid();
    obj["name"] = server->name();
    obj["group"] = server->group();
    obj["iconKey"] = server->iconKey();
    obj["platform"] = server->platformName();
    obj["mcVersion"] = server->mcVersion();
    obj["buildVersion"] = server->buildVersion();
    obj["downloadUrl"] = server->downloadUrl();
    obj["sha256"] = server->sha256();
    obj["sha1"] = server->sha1();
    obj["totalTimeLaunched"] = server->totalTimeLaunched();
    obj["lastTimeLaunched"] = server->lastTimeLaunched();
    obj["createdAt"] = server->createdAt().toString(Qt::ISODate);
    obj["lastLaunchedAt"] = server->lastLaunchedAt().toString(Qt::ISODate);

    return obj;
}