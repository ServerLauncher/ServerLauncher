#pragma once
#include <QObject>
#include <QString>

class Task : public QObject {
    Q_OBJECT
public:
    explicit Task(QObject* parent = nullptr) : QObject(parent) {}

    enum class State {
        Inactive,
        Running,
        Completed,
        Failed,
        Aborted
    };

    State getState() const { return m_state; }
    bool isRunning() const { return m_state == State::Running; }
    bool isFinished() const { return m_state == State::Completed || m_state == State::Failed || m_state == State::Aborted; }

    QString getErrorMessage() const { return m_errorMessage; }

public slots:
    void start() {
        if (m_state != State::Inactive) {
            return;
        }
        m_state = State::Running;
        emit started();

        executeTask();
    }

    virtual bool abort() { return false; }
signals:
    void started();
    void progress(qint64 current, qint64 total, const QString& msg);
    void completed();
    void failed(const QString& errorMessage);
    void aborted();
    void stepChanged(QString step);

protected:
    virtual void executeTask() = 0;

    void emitCompleted() {
        m_state = State::Completed;
        emit completed();
    }

    void emitFailed(const QString& reason) {
        m_state = State::Failed;
        m_errorMessage = reason;
        emit failed(reason);
    }

    void emitAborted() {
        m_state = State::Aborted;
        emit aborted();
    }

    void emitProgress(qint64 current, qint64 total, const QString& msg = {}) {
        emit progress(current, total, msg);
    }

private:
    State m_state = State::Inactive;
    QString m_errorMessage;
};