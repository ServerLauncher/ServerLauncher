#pragma once
#include "tasks/Task.hpp"
#include "NetRequest.hpp"
#include <QNetworkAccessManager>

class NetRequestTask : public Task {
    Q_OBJECT
public:
    explicit NetRequestTask(
        NetRequest::Ptr request, 
        QNetworkAccessManager* man, 
        QObject* parent = nullptr);
protected:
    void executeTask() override;
    bool abort() override;

private:
    QNetworkAccessManager* m_nam;
    NetRequest::Ptr m_request;
};