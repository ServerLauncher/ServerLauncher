#pragma once
#include <QObject>
#include <QList>
#include <QMap>
#include "BaseServer.hpp"

class ServerList : public QObject
{
    Q_OBJECT
public:
    explicit ServerList(const QString &rootDir, QObject *parent = nullptr);
    ~ServerList() override = default;

    bool loadList();
    bool saveList();

    void addServer(BaseServer *server);
    void removeServer(const QString &id);
    void deleteServerFiles(const QString &id);
    BaseServer *getServer(const QString &id) const;

    QList<BaseServer *> servers() const;
    int count() const;

    QStringList groups() const;
    QList<BaseServer *> serversInGroup(const QString &group) const;

    QString rootDir() const { return m_rootDir; }

signals:
    void serverAdded(BaseServer *server);
    void serverRemoved(const QString &id);
    void serverChanged(BaseServer *server);
    void listLoaded();

private:
    QString m_rootDir;
    QMap<QString, BaseServer *> m_servers;
};
