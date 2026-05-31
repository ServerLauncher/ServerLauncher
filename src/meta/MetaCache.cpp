#include "MetaCache.hpp"
#include "MetaParser.hpp"

#include <QFile>
#include <QSaveFile>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>

MetaCache::MetaCache(const QString& cacheDir, QObject* parent)
    : QObject(parent), m_cacheDir(cacheDir) {
}

QString MetaCache::cacheFilePath() const {
    return QDir(m_cacheDir).filePath("index.json");
}

bool MetaCache::loadFromDisk(QString& errorMessage) {
    const QString path = cacheFilePath();
    QFile file(path);

    if (!file.exists()) {
        m_loadStatus = LoadStatus::NotCached;
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = QString("Failed to open cache file: %1").arg(file.errorString());
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    m_fileSha256 = QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
    m_fileSha1 = QString(QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex());

    if (!MetaParser::parse(data, m_index, errorMessage)) {
        qWarning() << "Corrupt meta cache, removing:" << errorMessage;
        QFile::remove(path);
        m_fileSha256.clear();
        m_fileSha1.clear();
        return false;
    }

    m_loadStatus = LoadStatus::Local;
    return true;
}

bool MetaCache::updateFromNetwork(const QByteArray& data,
                                const QString& expectedSha1,
                                const QString& expectedSha256,
                                QString& errorMessage) {
    if (!expectedSha256.isEmpty()) {
        const QString actualSha256 = QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
        if (actualSha256 != expectedSha256) {
            errorMessage = QString("SHA256 mismatch: expected %1, got %2").arg(expectedSha256, actualSha256);
            return false;
        }
    }
    if (!expectedSha1.isEmpty()) {
        const QString actualSha1 = QString(QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex());
        if (actualSha1 != expectedSha1) {
            errorMessage = QString("SHA1 mismatch: expected %1, got %2").arg(expectedSha1, actualSha1);
            return false;
        }
    }

    if(!MetaParser::parse(data, m_index, errorMessage)) {
        return false;
    }

    QDir().mkpath(m_cacheDir);
    QSaveFile file(cacheFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = QString("Cannot open cache file for writing: %1")
                           .arg(file.errorString());
        return false;
    }

    file.write(data);
    if (!file.commit()) {
        errorMessage = QString("Failed to write cache file: %1").arg(file.errorString());
        return false;
    }

    m_fileSha256 = expectedSha256.isEmpty() ? QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex()) : expectedSha256;
    m_fileSha1 = expectedSha1.isEmpty() ? QString(QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex()) : expectedSha1;

    m_lastDataSize = data.size();

    m_loadStatus = LoadStatus::Remote;
    emit indexUpdated();
    return true;
}