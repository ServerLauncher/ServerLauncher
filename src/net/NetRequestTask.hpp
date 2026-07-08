#pragma once
#include "tasks/Task.hpp"
#include "NetRequest.hpp"
#include <QNetworkAccessManager>
#include <QPointer>

class NetRequestTask : public Task {
    Q_OBJECT
public:
    explicit NetRequestTask(
        NetRequest::Ptr request, 
        QNetworkAccessManager* man, 
        QObject* parent = nullptr);

    bool abort() override;    

protected:
    void executeTask() override;

private:
    QNetworkAccessManager* m_nam;
    NetRequest::Ptr m_request;
    QPointer<QNetworkReply> m_reply;
};