#pragma once
#include <QObject>
#include <QDateTime>
#include <QString>
#include <QUuid>
#include <cstdint>
#include "ServerProperties.hpp"

class BaseServer : public QObject
{
    Q_OBJECT
public:
    explicit BaseServer(const QString &rootDir, QObject *parent = nullptr);
    virtual ~BaseServer() = default;

    void invalidate();
    bool isValid() const { return m_valid; }

    // id
    virtual QString id() const;
    virtual QString uuid() const;
    void regenerateUuid();

    QString name() const;
    QString group() const;
    QString iconKey() const;

    void setUuid(const QString &uuid);
    void setName(const QString &name);
    void setGroup(const QString &group);
    void setIconKey(const QString &key);

    // Platform
    QString platformName() const;
    QString mcVersion() const;
    QString buildVersion() const;
    QString downloadUrl() const;
    QString sha256() const;
    QString sha1() const;

    void setPlatformName(const QString &platform);
    void setMcVersion(const QString &version);
    void setBuildVersion(const QString &build);
    void setDownloadUrl(const QString &url);
    void setSha256(const QString &hash);
    void setSha1(const QString &hash);

    // Roots
    QString serverRoot() const;
    virtual QString gameRoot() const { return serverRoot(); }
    virtual QString modsRoot() const;
    virtual QString pluginsRoot() const;
    QString serverJarPath() const;
    QString serverPropertiesPath() const;
    QString serverEulaPath() const;
    QString logsDirPath() const;

    // Server Properties
    ServerProperties &properties();
    const ServerProperties &properties() const;
    void loadProperties();
    void saveProperties();

    int port() const;
    int maxPlayers() const;
    bool onlineMode() const;
    QString motd() const;

    // Time
    int64_t totalTimeLaunched() const;
    int64_t lastTimeLaunched() const;
    bool countTimeLaunched() const;
    void resetTimeLaunched();
    void setTotalTimeLaunched(int64_t t);
    void setLastTimeLaunched(int64_t t);

    QDateTime createdAt() const;
    QDateTime lastLaunchedAt() const;
    void setCreatedAt(const QDateTime &dt);
    void setLastLaunchedAt(const QDateTime &dt);

signals:
    void nameChanged(const QString &name);
    void groupChanged(const QString &group);

protected:
    QString m_rootDir;
    QString m_uuid;
    QString m_id;
    QString m_name;
    QString m_group;
    QString m_iconKey = "default";
    QString m_platformName;
    QString m_mcVersion;
    QString m_buildVersion;
    QString m_downloadUrl;
    QString m_sha256;
    QString m_sha1;
    QDateTime m_createdAt;
    QDateTime m_lastLaunchedAt;
    int64_t m_totalTimeLaunched = 0;
    int64_t m_lastTimeLaunched = 0;
    bool m_countTimeLaunched = true;
    bool m_valid = true;

    ServerProperties m_properties;
};