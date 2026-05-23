#pragma once
#include <QObject>
#include <QString>

class Task : public QObject {
    Q_OBJECT
public:
    explicit Task(QObject* parent = nullptr);

    enum class State {
        Inactive,
        Running,
        Completed,
        Failed,
        Aborted
    };

    State getState() const;
    bool isRunning() const;
    bool isFinished() const;

    QString getErrorMessage() const;

public slots:
    void start();

    virtual bool abort();
signals:
    void started();
    void progress(qint64 current, qint64 total, const QString& msg);
    void completed();
    void failed(const QString& errorMessage);
    void aborted();
    void stepChanged(QString step);

protected:
    virtual void executeTask() = 0;

    void emitCompleted();
    void emitFailed(const QString& reason);
    void emitAborted();
    void emitProgress(qint64 current, qint64 total, const QString& msg = {});
    
    void emitStepChanged(const QString& step);

private:
    State m_state = State::Inactive;
    QString m_errorMessage;
};