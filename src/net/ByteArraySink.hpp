#pragma once
#include "Sink.hpp"
#include <QByteArray>

class ByteArraySink : public Sink {
public:
    explicit ByteArraySink(QByteArray* target)
        : m_target(target)
    { }

    Task::State init(QNetworkRequest& request) override {
        if (!m_target) {
            return Task::State::Failed;
        }
        m_target->clear();
        if (!initAllValidators(request)) {
            return Task::State::Failed;
        }
        return Task::State::Running;
    }

    Task::State write(QByteArray* data) override {
        if (!writeAllValidators(data)) {
            return Task::State::Failed;
        }
        m_target->append(*data);
        return Task::State::Running;
    }

    Task::State finalize(QNetworkReply& reply) override {
        if (!validateAllValidators(reply)) {
            m_target->clear();
            return Task::State::Failed;
        }

        return Task::State::Completed;
    }

    Task::State abort() override {
        abortAllValidators();

        if (m_target) {
            m_target->clear();
        }
        return Task::State::Aborted;
    }

    bool hasLocalData() override {
        return m_target && !m_target->isEmpty();
    }
private:
    QByteArray* m_target = nullptr;
};