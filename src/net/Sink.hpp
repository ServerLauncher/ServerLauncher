#pragma once
#include "tasks/Task.hpp"
#include "Validator.hpp"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QByteArray>
#include <QString>
#include <vector>
#include <memory>

class Sink {
public:
    Sink() = default;
    virtual ~Sink() = default;

    virtual Task::State init(QNetworkRequest& request) = 0;
    virtual Task::State write(QByteArray* data) = 0;
    virtual Task::State abort() = 0;
    virtual Task::State finalize(QNetworkReply& reply) = 0;

    virtual bool hasLocalData() = 0;

    QString errorMessage() const { return m_errorMessage; }
protected:
    bool initAllValidators(QNetworkRequest& request) {
        for (const auto& validator : m_validators) {
            if (!validator->init(request)) {
                m_errorMessage = "Validator initialization failed";
                return false;
            }
        }
        return true;
    }

    bool writeAllValidators(QByteArray* data) {
        for (const auto& validator : m_validators) {
            if (!validator->write(data)) {
                m_errorMessage = "Validator write failed";
                return false;
            }
        }
        return true;
    }

    bool validateAllValidators(QNetworkReply& reply) {
        for (const auto& validator : m_validators) {
            if (!validator->validate(reply)) {
                m_errorMessage = "Validator validation failed";
                return false;
            }
        }
        return true;
    }

    bool abortAllValidators() {
        bool allAborted = true;
        for (const auto& validator : m_validators) {
            allAborted &= validator->abort();
        }
        return allAborted;
    }
private:
    QString m_errorMessage;
    std::vector<std::unique_ptr<Validator>> m_validators;
};