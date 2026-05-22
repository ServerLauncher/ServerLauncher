#include <NetJob.hpp>
#include <QNetworkRequest>

NetJob::NetJob(const QString& name,
            QNetworkAccessManager* man,
            QObject* parent = nullptr): Task(parent), m_nam(man), m_name(name)
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

    QNetworkReply* reply = m_nam->get(netReq);
    m_replies[index] = reply;
    m_running++;

    connect(reply, &QNetworkReply::downloadProgress, this,
        [this, index](qint64 recive, qint64 total) {
            emit progress(m_done + recive, m_requests.size() * total,
                          m_requests[index]->url.fileName());
        }
    );

    connect(reply, &QNetworkReply::finished, this,
        [this, index, reply](){
            reply->deleteLater();
            m_replies[index] = nullptr;
            m_running--;

            auto& req = m_requests[index];

            if(reply->error() != QNetworkReply::NoError) {
                req->state = NetRequest::State::Failed;
                req->onFailed(reply);
                m_failed++;
            }
            else {
                req->state = NetRequest::State::Completed;
                req->onSucceeded(reply);
                m_done++;
            }

            emitProgress(m_done, m_requests.size());

            startNextRequests();

             if (m_done == m_requests.size()) {
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