#pragma once
#include <QObject>
#include <QString>
#include <QNetworkReply>
#include <QUrl>
#include <memory>
#include "Sink.hpp"

class NetRequest : public QObject {
    Q_OBJECT
public:
    using Ptr = std::shared_ptr<NetRequest>;
    
    enum class State {
        Inactive,
        Running,
        Completed,
        Failed
    };

    QUrl url;
    State state = State::Inactive;
    QString errorMessage;

    std::unique_ptr<Sink> sink;

signals:
    void progress(qint64 current, qint64 total);
    void completed();
    void failed(const QString& errorMessage);
};