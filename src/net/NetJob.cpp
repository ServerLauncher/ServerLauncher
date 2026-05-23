#include "NetJob.hpp"
#include <QNetworkRequest>

NetJob::NetJob(const QString& name,
            QNetworkAccessManager* man,
            QObject* parent): ConcurrentTask(name, 6, parent), m_nam(man)
{ }

void NetJob::addRequest(NetRequest::Ptr request) {
    auto task = new NetRequestTask(request, m_nam, this);
    ConcurrentTask::addTask(task);
}