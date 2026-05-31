#pragma once
#include "MetaInfo.hpp"
#include <QObject>
#include <QString>
#include <QByteArray>

class MetaCache: public QObject{
    Q_OBJECT
public:
    enum class LoadStatus {
        NotCached,
        Local,
        Remote
    };

    explicit MetaCache(const QString& cacheDir, QObject* parent = nullptr);
    
    bool loadFromDisk(QString& errorMessage);
    bool updateFromNetwork(const QByteArray& data,
                        const QString& expectedSha1,
                        const QString& expectedSha256,
                        QString& errorMessage);

    const MetaIndex& index() const { return m_index; }
    LoadStatus status() const { return m_loadStatus; }
    bool isLoaded() const { return m_loadStatus != LoadStatus::NotCached; }
    
    QString cacheFilePath() const;
    QString cachedSha256() const { return m_fileSha256; }
    QString cachedSha1() const { return m_fileSha1; }

signals:
    void indexUpdated();

private:
    LoadStatus m_loadStatus = LoadStatus::NotCached;
    QString m_cacheDir;
    QString m_fileSha256;
    QString m_fileSha1;
    MetaIndex m_index;
};
