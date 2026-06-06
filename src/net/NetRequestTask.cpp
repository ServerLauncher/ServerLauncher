#include "NetRequestTask.hpp"

NetRequestTask::NetRequestTask(NetRequest::Ptr request, QNetworkAccessManager* man, QObject* parent)
    : Task(parent), m_request(request), m_nam(man) 
{ }

void NetRequestTask::executeTask() {
    QNetworkRequest netReq(m_request->url);
    netReq.setHeader(QNetworkRequest::UserAgentHeader, "ServerLauncher/1.0");
     netReq.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy
    );

    auto initState = m_request->sink->init(netReq);
    if (initState == Task::State::Failed) {
        emitFailed(m_request->sink->errorMessage().isEmpty()
            ? "Sink init failed"
            : m_request->sink->errorMessage());
        return;
    }
    if (initState == Task::State::Aborted) { emitAborted(); return; }
    if (initState == Task::State::Completed) { emitCompleted(); return; }

    QNetworkReply* reply = m_nam->get(netReq);

    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        auto data = reply->readAll();
        if (m_request->sink->write(&data) == Task::State::Failed)
            reply->abort();
    });

    connect(reply, &QNetworkReply::downloadProgress, this,
        [this](qint64 received, qint64 total) {
            emitProgress(received, total, m_request->url.fileName());
        }
    );

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        bool isAbort = (reply->error() == QNetworkReply::OperationCanceledError);

        if (isAbort) {
            m_request->sink->abort();
            emitAborted();
        }
        else if (reply->error() != QNetworkReply::NoError) {
            m_request->sink->abort();
            emitFailed(reply->errorString());
        }
        else {
            auto finalizeState = m_request->sink->finalize(*reply);
            if (finalizeState == Task::State::Completed)
                emitCompleted();
            else
                emitFailed(m_request->sink->errorMessage().isEmpty()
                    ? "Sink finalize failed"
                    : m_request->sink->errorMessage());
        }
    });
}

bool NetRequestTask::abort() {
    return true;
}