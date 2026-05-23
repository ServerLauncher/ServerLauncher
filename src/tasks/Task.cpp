#include "Task.hpp"

Task::Task(QObject* parent) : QObject(parent)
{ }

Task::State Task::getState() const {
    return m_state;
}

bool Task::isRunning() const {
    return m_state == State::Running;
}

bool Task::isFinished() const {
    return m_state == State::Completed || m_state == State::Failed || m_state == State::Aborted;
}

QString Task::getErrorMessage() const {
    return m_errorMessage;
}

void Task::start() {
        if (m_state != State::Inactive) {
            return;
        }
        m_state = State::Running;
        emit started();

        executeTask();
}

bool Task::abort() { 
    return false; 
}

void Task::emitCompleted() {
        Q_ASSERT(m_state == State::Running);
        m_state = State::Completed;
        emit completed();
}

void Task::emitFailed(const QString& reason) {
        Q_ASSERT(m_state == State::Running);
        m_state = State::Failed;
        m_errorMessage = reason;
        emit failed(reason);
}

void Task::emitAborted() {
        m_state = State::Aborted;
        emit aborted();
}

void Task::emitProgress(qint64 current, qint64 total, const QString& msg) {
        emit progress(current, total, msg);
}

void Task::emitStepChanged(const QString& step) {
        emit stepChanged(step);
}