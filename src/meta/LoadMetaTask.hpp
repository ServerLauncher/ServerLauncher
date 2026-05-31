#pragma once
#include <QString>
#include <QByteArray>
#include "tasks/Task.hpp"
#include "net/NetRequestTask.hpp"
#include "MetaCache.hpp"

class LoadMetaTask : public Task {
    Q_OBJECT
public:
    explicit LoadMetaTask(MetaCache* cache,
                          const QString& url,
                          QNetworkAccessManager* nam,
                          QObject* parent = nullptr);

    bool abort() override;    

protected:
    void executeTask() override;

private:
    MetaCache* m_cache;
    QString m_url;
    QNetworkAccessManager* m_nam;
    std::shared_ptr<NetRequestTask> m_netTask;
};