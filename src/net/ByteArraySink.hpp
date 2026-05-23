#pragma once
#include "Sink.hpp"
#include <QByteArray>

class ByteArraySink : public Sink {
public:
    ByteArraySink(QByteArray* target): m_target(target) 
    { }

    Task::State init(QNetworkRequest& request) override {
        return Task::State::Running;
    }
    Task::State write(QByteArray* data) override {
        m_target->append(*data);
        return Task::State::Running;
    }
    Task::State abort() override {
        m_target->clear();
        return Task::State::Failed;
    }
    Task::State finalize(QNetworkReply& reply) override {
        return Task::State::Completed;
    }

    bool hasLocalData() override {
        return false;
    }
private:
    QByteArray* m_target;
};