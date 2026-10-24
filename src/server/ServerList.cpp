#include "ServerList.hpp"
#include "ServerListSerializer.hpp"
#include <QDir>

ServerList::ServerList(const QString &rootDir, QObject *parent)
    : QObject(parent), m_rootDir(rootDir)
{
    QDir().mkpath(rootDir);
}

bool ServerList::loadList()
{
    QString filePath = QDir(m_rootDir).filePath("servers.json");
    bool ok = ServerListSerializer::loadFromFile(filePath, *this);
    if (ok)
        emit listLoaded();
    return ok;
}

bool ServerList::saveList()
{
    QString filePath = QDir(m_rootDir).filePath("servers.json");
    return ServerListSerializer::saveToFile(filePath, *this);
}

void ServerList::addServer(BaseServer *server)
{
    if (!server)
        return;

    QString id = server->id();
    if (id.isEmpty())
        id = server->uuid();

    if (id.isEmpty())
        return;

    if (m_servers.contains(id))
    {
        if (m_servers.value(id) == server)
            return;
        removeServer(id);
    }

    if (server->parent() != this)
        server->setParent(this);

    m_servers.insert(id, server);

    connect(server, &BaseServer::nameChanged, this, [this, server]()
            { emit serverChanged(server); });
    connect(server, &BaseServer::groupChanged, this, [this, server]()
            { emit serverChanged(server); });

    emit serverAdded(server);
    saveList();
}

void ServerList::removeServer(const QString &id)
{
    auto it = m_servers.find(id);
    if (it == m_servers.end())
    {
        for (auto mapIt = m_servers.begin(); mapIt != m_servers.end(); ++mapIt)
        {
            if (mapIt.value() && (mapIt.value()->id() == id || mapIt.value()->uuid() == id))
            {
                it = mapIt;
                break;
            }
        }
    }

    if (it == m_servers.end())
        return;

    QString key = it.key();
    BaseServer *server = it.value();
    m_servers.erase(it);

    if (server)
    {
        disconnect(server, nullptr, this, nullptr);
        server->deleteLater();
    }

    emit serverRemoved(key);
    saveList();
}

void ServerList::deleteServerFiles(const QString &id)
{
    auto *server = getServer(id);
    if (!server)
        return;

    QString rootDir = server->serverRoot();
    removeServer(id);

    if (!rootDir.isEmpty())
        QDir(rootDir).removeRecursively();
}

BaseServer *ServerList::getServer(const QString &id) const
{
    auto it = m_servers.find(id);
    if (it != m_servers.end())
        return it.value();

    for (auto *server : m_servers)
    {
        if (server && (server->id() == id || server->uuid() == id))
            return server;
    }
    return nullptr;
}

QList<BaseServer *> ServerList::servers() const
{
    return m_servers.values();
}

int ServerList::count() const
{
    return m_servers.size();
}

QStringList ServerList::groups() const
{
    QStringList result;
    for (const auto *server : m_servers)
    {
        if (!server)
            continue;
        const QString group = server->group();
        if (!group.isEmpty() && !result.contains(group))
            result.append(group);
    }
    result.sort(Qt::CaseInsensitive);
    return result;
}

QList<BaseServer *> ServerList::serversInGroup(const QString &group) const
{
    QList<BaseServer *> result;
    for (auto *server : m_servers)
    {
        if (server && server->group() == group)
            result.append(server);
    }
    return result;
}
