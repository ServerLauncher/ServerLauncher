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

    void addValidator(std::unique_ptr<Validator> validator){
        if(validator) 
            m_validators.push_back(std::move(validator));
    }

    virtual auto init(QNetworkRequest& request) -> Task::State = 0;
    virtual auto write(QByteArray* data) -> Task::State = 0;
    virtual auto abort() -> Task::State = 0;
    virtual auto finalize(QNetworkReply& reply) -> Task::State = 0;

    virtual bool hasLocalData() = 0;

    QString errorMessage() const { return m_errorMessage; }

protected:
    void setErrorMessage(const QString& errorMessage) { m_errorMessage = errorMessage; }

    bool initAllValidators(QNetworkRequest& request) {
        for (const auto& validator : m_validators) {
            if (!validator->init(request)) {
                m_errorMessage = validator->errorMessage();
                return false;
            }
        }
        return true;
    }

    bool writeAllValidators(QByteArray* data) {
        for (const auto& validator : m_validators) {
            if (!validator->write(data)) {
                m_errorMessage = validator->errorMessage();
                return false;
            }
        }
        return true;
    }

    bool validateAllValidators(QNetworkReply& reply) {
        for (const auto& validator : m_validators) {
            if (!validator->validate(reply)) {
                m_errorMessage = validator->errorMessage();
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