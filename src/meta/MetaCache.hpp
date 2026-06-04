#pragma once
#include "MetaInfo.hpp"
#include "tasks/Task.hpp"
#include <QObject>
#include <QString>
#include <QByteArray>

class QNetworkAccessManager;

class MetaCache : public QObject{
    Q_OBJECT
public:
    enum class LoadStatus {
        NotCached,
        Local,
        Remote
    };

    explicit MetaCache(const QString& cacheDir, QObject* parent = nullptr);
    virtual ~MetaCache() = default;

    virtual QString cacheFilePath() const = 0;
    virtual bool parse(const QByteArray& data, QString& errorMessage) = 0;

    Task* loadTask(const QString& url, QNetworkAccessManager* nam);
    
    bool loadFromDisk(QString& errorMessage);
    bool updateFromNetwork(const QByteArray& data,
                        const QString& expectedSha1,
                        const QString& expectedSha256,
                        QString& errorMessage);

    LoadStatus status() const { return m_loadStatus; }
    bool isLoaded() const { return m_loadStatus != LoadStatus::NotCached; }
    QString cachedSha256() const { return m_fileSha256; }
    QString cachedSha1() const { return m_fileSha1; }
    qint64 lastDataSize() const { return m_lastDataSize; }

protected:
    QString m_cacheDir;

private:
    qint64 m_lastDataSize = 0;
    LoadStatus m_loadStatus = LoadStatus::NotCached;
    QString m_fileSha256;
    QString m_fileSha1;
};

class MetaIndexCache : public MetaCache {
    Q_OBJECT
public:
    explicit MetaIndexCache(const QString& cacheDir, QObject* parent = nullptr);

    QString cacheFilePath() const override;
    bool parse(const QByteArray& data, QString& errorMessage) override;

    const MetaIndex& index() const { return m_index; }
signals:
    void indexUpdated();
private:
    MetaIndex m_index;
};

class MetaPackageCache : public MetaCache {
    Q_OBJECT
public:
    explicit MetaPackageCache(const QString& cacheDir, const QString& uid, QObject* parent = nullptr);

    QString cacheFilePath() const override;
    bool parse(const QByteArray& data, QString& errorMessage) override;

    const MetaPackage* package() const { return &m_package; }
    const QString& uid() const { return m_uid; }

signals:
    void packageUpdated();

private:
    QString m_uid;
    MetaPackage m_package;
};