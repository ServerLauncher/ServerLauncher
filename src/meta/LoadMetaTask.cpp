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
    if (m_netTask) 
        return m_netTask->abort();
    return Task::abort();
}

void LoadMetaTask::executeTask() {
    if (m_cache->status() == MetaCache::LoadStatus::Remote) {
        emitCompleted();
        return;
    }

    if (m_cache->status() == MetaCache::LoadStatus::Local) {
        const QString expectedSha256 = m_cache->expectedSha256();
        const QString expectedSha1 = m_cache->expectedSha1();

        bool allExpectedMatch = true;
        bool anyExpected = false;

        if (!expectedSha256.isEmpty()) {
            anyExpected = true;
            allExpectedMatch &= (m_cache->cachedSha256() == expectedSha256.toLower());
        }
        if (!expectedSha1.isEmpty()) {
            anyExpected = true;
            allExpectedMatch &= (m_cache->cachedSha1() == expectedSha1.toLower());
        }

        if (anyExpected && allExpectedMatch) {
            emitCompleted();
            return;
        }
    }

    auto request = std::make_shared<NetRequest>();
    request->url = QUrl(m_url);

    auto buffer = std::make_shared<QByteArray>();
    auto sink = std::make_unique<ByteArraySink>(buffer.get());

    // Add SHA256 validator if expected hash is available
    const QString expectedSha256 = m_cache->expectedSha256();
    if (!expectedSha256.isEmpty()) {
        sink->addValidator(std::make_unique<HashValidator>(
            QCryptographicHash::Sha256, expectedSha256.toLower()));
    }

    // Add SHA1 validator if expected hash is available
    const QString expectedSha1 = m_cache->expectedSha1();
    if (!expectedSha1.isEmpty()) {
        sink->addValidator(std::make_unique<HashValidator>(
            QCryptographicHash::Sha1, expectedSha1.toLower()));
    }

    request->sink = std::move(sink);

    m_netTask = std::make_shared<NetRequestTask>(request, m_nam, this);

    connect(m_netTask.get(), &Task::failed, this, &LoadMetaTask::emitFailed);
    connect(m_netTask.get(), &Task::aborted, this, &LoadMetaTask::emitAborted);
    connect(m_netTask.get(), &Task::progress, this, [this](qint64 current, qint64 total, const QString& msg) {
        emitProgress(current, total, msg);
    });
    connect(m_netTask.get(), &Task::completed, this, [this, buffer]() {
        QString errorMessage;
        if (!m_cache->updateFromNetwork(*buffer, errorMessage)) {
            emitFailed(errorMessage);
            return;
        }
        emitCompleted();
    });

    m_netTask->start();
}