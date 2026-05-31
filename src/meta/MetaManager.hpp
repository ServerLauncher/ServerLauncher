#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QString>
#include "MetaCache.hpp"
#include "LoadMetaTask.hpp"

class MetaManager : public QObject {
    Q_OBJECT
public:
    explicit MetaManager(const QString& cacheDir,
                         const QString& url,
                         QNetworkAccessManager* nam,
                         QObject* parent = nullptr);

    void init();
    LoadMetaTask* load();

    const MetaIndex& index() const;
    const MetaPlatform* findPlatform(const QString& uid) const;
    bool isLoaded() const;

    MetaCache* cache() const { return m_cache; }

signals:
    void indexUpdated();
    void loadFailed(const QString& error);
private:
    MetaCache* m_cache;
    QString m_url;
    QNetworkAccessManager* m_nam;
};