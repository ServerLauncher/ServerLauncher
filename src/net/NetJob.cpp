#include "NetJob.hpp"
#include <QNetworkRequest>

NetJob::NetJob(const QString& name,
            QNetworkAccessManager* man,
            QObject* parent): Task(parent), m_nam(man), m_name(name)
{ }

void NetJob::addRequest(NetRequest::Ptr request) {
    m_requests.append(request);
    m_replies.append(nullptr);
}

void NetJob::executeTask() {
    if (m_requests.isEmpty()) {
        emitCompleted();
        return;
    }
    startNextRequests();
}

void NetJob::startNextRequests() {
    while (m_running < MAX_CONCURRENT && m_next < m_requests.size()) {
        startRequest(m_next++);
    }

}

void NetJob::startRequest(int index) {
    auto request = m_requests[index];
    request->state = NetRequest::State::Running;

    QNetworkRequest netReq(request->url);
    netReq.setHeader(QNetworkRequest::UserAgentHeader, "ServerLauncher/1.0");
    netReq.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy
    );

    auto initState = request->sink->init(netReq);
    if (initState == Task::State::Failed) {
        request->state = NetRequest::State::Failed;
        m_failed++;
        if(m_aborted + m_failed + m_done == m_requests.size()) {
            emitFailed(QString("%1 request(s) failed").arg(m_failed));
        }
        else {
            startNextRequests();
        }
        return;
    }
    if (initState == Task::State::Aborted) {
        request->state = NetRequest::State::Failed;
        m_aborted++;
        if(m_aborted + m_failed + m_done == m_requests.size()) {
            emitFailed(QString("%1 request(s) failed").arg(m_failed));
        }
        else {
            startNextRequests();
        }
        return;
    }
    if (initState == Task::State::Completed) {
        request->state = NetRequest::State::Completed;
        m_done++;
        if(m_aborted + m_failed + m_done == m_requests.size()) {
            emitFailed(QString("%1 request(s) failed").arg(m_failed));
        }
        else {
            startNextRequests();
        }
        return;
    }


    QNetworkReply* reply = m_nam->get(netReq);
    m_replies[index] = reply;
    m_running++;

    connect(reply, &QNetworkReply::readyRead, this,
        [req = request.get(), reply](){
            auto data = reply->readAll();
            auto state = req->sink->write(&data);
            if (state == Task::State::Failed) 
                reply->abort(); 
        }
    );

    connect(reply, &QNetworkReply::downloadProgress, this,
        [this, index](qint64 received, qint64 total) {
            emit progress(received, total, m_requests[index]->url.fileName());
        }
    );

    connect(reply, &QNetworkReply::finished, this,
        [this, index, reply](){
            reply->deleteLater();
            m_replies[index] = nullptr;
            m_running--;

            auto& req = m_requests[index];

            bool isAbort = (reply->error() == QNetworkReply::OperationCanceledError);

            if(!isAbort && reply->error() != QNetworkReply::NoError) {
                req->state = NetRequest::State::Failed;
                req->sink->abort();
                m_failed++;
            }
            else if(isAbort) {
                req->state = NetRequest::State::Failed;
                req->sink->abort();
                m_aborted++;
            }
            else {
                auto finalizeState = req->sink->finalize(*reply);
                if (finalizeState == Task::State::Completed) {
                    req->state = NetRequest::State::Completed;
                    m_done++;
                }
                else {
                    req->state = NetRequest::State::Failed;
                    m_failed++;
                }
            }
            
            emitProgress(m_done, m_requests.size());

            startNextRequests();

             if (m_done + m_failed + m_aborted == m_requests.size()) {
                if (m_failed > 0)
                    emitFailed(QString("%1 request(s) failed").arg(m_failed));
                else
                    emitCompleted();
            }
        });
}

bool NetJob::abort() {
    for (auto reply : m_replies) {
        if (reply) {
            reply->abort();
        }
    }
    emitAborted();
    return true;
}