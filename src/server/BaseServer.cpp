#include "BaseServer.hpp"
#include <QDir>
#include <QUuid>

BaseServer::BaseServer(const QString &rootDir, QObject *parent)
    : QObject(parent),
      m_rootDir(rootDir),
      m_uuid(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      m_createdAt(QDateTime::currentDateTime())
{
}

void BaseServer::invalidate()
{
    m_valid = false;
}

QString BaseServer::id() const { return m_id; }
QString BaseServer::uuid() const { return m_uuid; }

void BaseServer::regenerateUuid()
{
    m_uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void BaseServer::setUuid(const QString &uuid) { m_uuid = uuid; }

QString BaseServer::name() const { return m_name; }
QString BaseServer::group() const { return m_group; }
QString BaseServer::iconKey() const { return m_iconKey; }

void BaseServer::setName(const QString &name)
{
    m_name = name;
    emit nameChanged(name);
}

void BaseServer::setGroup(const QString &group)
{
    m_group = group;
    emit groupChanged(group);
}

void BaseServer::setIconKey(const QString &iconKey) { m_iconKey = iconKey; }

QString BaseServer::platformName() const { return m_platformName; }
QString BaseServer::mcVersion() const { return m_mcVersion; }
QString BaseServer::buildVersion() const { return m_buildVersion; }
QString BaseServer::downloadUrl() const { return m_downloadUrl; }
QString BaseServer::sha256() const { return m_sha256; }
QString BaseServer::sha1() const { return m_sha1; }

void BaseServer::setPlatformName(const QString &p) { m_platformName = p; }
void BaseServer::setMcVersion(const QString &v) { m_mcVersion = v; }
void BaseServer::setBuildVersion(const QString &b) { m_buildVersion = b; }
void BaseServer::setDownloadUrl(const QString &u) { m_downloadUrl = u; }
void BaseServer::setSha256(const QString &h) { m_sha256 = h; }
void BaseServer::setSha1(const QString &h) { m_sha1 = h; }

QString BaseServer::serverRoot() const { return m_rootDir; }
QString BaseServer::modsRoot() const { return QDir(m_rootDir).filePath("mods"); }
QString BaseServer::pluginsRoot() const { return QDir(m_rootDir).filePath("plugins"); }
QString BaseServer::serverJarPath() const { return QDir(m_rootDir).filePath("server.jar"); }
QString BaseServer::serverPropertiesPath() const { return QDir(m_rootDir).filePath("server.properties"); }
QString BaseServer::serverEulaPath() const { return QDir(m_rootDir).filePath("eula.txt"); }
QString BaseServer::logsDirPath() const { return QDir(m_rootDir).filePath("logs"); }

ServerProperties &BaseServer::properties() { return m_properties; }
const ServerProperties &BaseServer::properties() const { return m_properties; }

void BaseServer::loadProperties()
{
    m_properties.loadFromFile(serverPropertiesPath());
}

void BaseServer::saveProperties()
{
    m_properties.saveToFile(serverPropertiesPath());
}

int BaseServer::port() const { return m_properties.getInt("server-port", 25565); }
int BaseServer::maxPlayers() const { return m_properties.getInt("max-players", 20); }
bool BaseServer::onlineMode() const { return m_properties.getBool("online-mode", true); }
QString BaseServer::motd() const { return m_properties.get("motd"); }

int64_t BaseServer::totalTimeLaunched() const { return m_totalTimeLaunched; }
int64_t BaseServer::lastTimeLaunched() const { return m_lastTimeLaunched; }
bool BaseServer::countTimeLaunched() const { return m_countTimeLaunched; }
QDateTime BaseServer::createdAt() const { return m_createdAt; }
QDateTime BaseServer::lastLaunchedAt() const { return m_lastLaunchedAt; }

void BaseServer::resetTimeLaunched()
{
    m_totalTimeLaunched = 0;
    m_lastTimeLaunched = 0;
}

void BaseServer::setTotalTimeLaunched(int64_t t) { m_totalTimeLaunched = t; }
void BaseServer::setLastTimeLaunched(int64_t t) { m_lastTimeLaunched = t; }
void BaseServer::setCreatedAt(const QDateTime &dt) { m_createdAt = dt; }
void BaseServer::setLastLaunchedAt(const QDateTime &dt) { m_lastLaunchedAt = dt; }