#pragma once
#include <QString>
#include "ServerList.hpp"

class ServerListSerializer
{
public:
    static bool loadFromFile(const QString &filePath, ServerList &list);
    static bool saveToFile(const QString &filePath, const ServerList &list);

private:
    static BaseServer *serverFromJson(const QJsonObject &obj, const QString &rootDir);
    static QJsonObject serverToJson(const BaseServer *server);

    static constexpr int FORMAT_VERSION = 1;
};