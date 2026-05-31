#include "LoadMetaTask.hpp"
#include "net/NetRequest.hpp"
#include "net/ByteArraySink.hpp"

LoadMetaTask::LoadMetaTask(MetaCache* cache,
                           const QString& url,
                           QNetworkAccessManager* nam,
                           QObject* parent)
    : Task(parent), m_cache(cache), m_url(url), m_nam(nam)
{}

bool LoadMetaTask::abort() {
    if (m_netTask) {
        m_netTask->abort();
    }
    return true;
}

void LoadMetaTask::executeTask() {
    if (m_cache->status() == MetaCache::LoadStatus::Remote) {
        emitCompleted();
        return;
    }

    auto request = std::make_shared<NetRequest>();
    request->url = QUrl(m_url);

    auto buffer = std::make_shared<QByteArray>();
    request->sink = std::make_unique<ByteArraySink>(buffer.get());

    m_netTask = std::make_shared<NetRequestTask>(request, m_nam, this);

    connect(m_netTask.get(), &Task::failed, this, &LoadMetaTask::emitFailed);
    connect(m_netTask.get(), &Task::aborted, this, &LoadMetaTask::emitAborted);
    connect(m_netTask.get(), &Task::progress, this, [this](qint64 current, qint64 total, const QString& msg) {
        emitProgress(current, total, msg);
    });
    connect(m_netTask.get(), &Task::completed, this, [this, buffer]() {
        QString errorMessage;
        if (!m_cache->updateFromNetwork(*buffer, {}, {}, errorMessage)) {
            emitFailed(errorMessage);
            return;
        }
        emitCompleted();
    });

    m_netTask->start();
}