#include "MetaCache.hpp"
#include "MetaParser.hpp"
#include "LoadMetaTask.hpp"
#include <QFile>
#include <QSaveFile>
#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QDebug>

MetaCache::MetaCache(const QString& cacheDir, QObject* parent)
    : QObject(parent), m_cacheDir(cacheDir) {
}

bool MetaCache::loadFromDisk(QString& errorMessage) {
    const QString path = cacheFilePath();
    QFile file(path);

    if (!file.exists())
        return true;

    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = QString("Cannot open cache file: %1").arg(file.errorString());
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    if (!parse(data, errorMessage)) {
        qWarning() << "Corrupt cache, removing:" << errorMessage;
        QFile::remove(path);
        return false;
    }

    m_loadStatus = LoadStatus::Local;
    return true;
}

bool MetaCache::updateFromNetwork(const QByteArray& data, QString& errorMessage) {
    if(!parse(data, errorMessage))
        return false;

    QDir().mkpath(QFileInfo(cacheFilePath()).absolutePath());
    QSaveFile file(cacheFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = QString("Cannot open file for writing: %1").arg(file.errorString());
        return false;
    }

    file.write(data);

    if (!file.commit()) {
        errorMessage = QString("Failed to commit file: %1").arg(file.errorString());
        return false;
    }

    m_fileSha256 = QString::fromLatin1(
        QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());

    m_fileSha1 = QString::fromLatin1(
        QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex());

    m_lastDataSize = data.size();
    m_loadStatus   = LoadStatus::Remote;
    return true;
}

Task* MetaCache::loadTask(const QString& url, QNetworkAccessManager* nam) {
    return new LoadMetaTask(this, url, nam, this);
}

MetaIndexCache::MetaIndexCache(const QString& cacheDir, QObject* parent)
    : MetaCache(cacheDir, parent)
{ }

QString MetaIndexCache::cacheFilePath() const {
    return QDir(m_cacheDir).filePath("index.json");
}

bool MetaIndexCache::parse(const QByteArray& data, QString& errorMessage) {
    if (!MetaParser::parseIndex(data, m_index, errorMessage))
        return false;
    emit indexUpdated();
    return true;
}

MetaPackageCache::MetaPackageCache(const QString& cacheDir, const QString& uid, QObject* parent)
    : MetaCache(cacheDir, parent), m_uid(uid)
    
{ }

QString MetaPackageCache::cacheFilePath() const {
    return QDir(m_cacheDir).filePath(m_uid + "/package.json");
}

bool MetaPackageCache::parse(const QByteArray& data, QString& errorMessage) {
    if (!MetaParser::parsePackage(data, m_package, errorMessage))
        return false;
    emit packageUpdated();
    return true;
}

MetaVersionCache::MetaVersionCache(const QString& cacheDir, const QString& uid,
        const QString& mc_version, QObject* parent) 
        : MetaCache(cacheDir, parent), m_mcVersion(mc_version), m_uid(uid)
{ }

QString MetaVersionCache::cacheFilePath() const {
    return QDir(m_cacheDir).filePath(m_uid + "/" + m_mcVersion + ".json");
}

bool MetaVersionCache::parse(const QByteArray& data, QString& errorMessage) {
    qDebug() << "MetaVersionCache::parse called for" << m_uid << m_mcVersion;
    if (!MetaParser::parseVersion(data, m_version, errorMessage)){
        qDebug() << "MetaVersionCache::parse FAILED:" << errorMessage;
        return false;
    }
    emit versionUpdated();
    return true;
}