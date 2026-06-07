#pragma once
#include "Task.hpp"
#include <QHash>
#include <QQueue>
#include <QDebug>

class ConcurrentTask : public Task {
    Q_OBJECT
public:
    using Ptr = std::shared_ptr<ConcurrentTask>;

    explicit ConcurrentTask(const QString& task_name = "", int max_concurrent = 6, QObject* parent = nullptr);   
    ~ConcurrentTask() override;
    
    void addTask(Task* task);
    void prioritizeTask(Task* task);
    bool abort() override;
    void clear();

protected:
    void executeTask() override;

private slots:
    void executeNextSubTask();
    void subTaskSucceeded(Task* task);
    void subTaskFailed(Task* task, const QString& msg);

private:
    void startSubTask(Task* task);
    void updateState();
    int totalSize() const;

    QString m_name;
    int m_maxConcurrent;

    QQueue<Task*> m_queue;
    QHash<Task*, Task*> m_doing;
    QHash<Task*, Task*> m_done;
    QHash<Task*, Task*> m_failed;
};