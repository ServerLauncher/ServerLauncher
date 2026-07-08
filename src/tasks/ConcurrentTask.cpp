#include "ConcurrentTask.hpp"

ConcurrentTask::ConcurrentTask(const QString& name, int max_concurrent, QObject* parent)
    : Task(parent), m_name(name), m_maxConcurrent(max_concurrent)
{ }

void ConcurrentTask::addTask(Task* task) {
    m_queue.enqueue(task);
}

ConcurrentTask::~ConcurrentTask() {
    for (auto task : m_doing)
        task->disconnect(this);
}

void ConcurrentTask::prioritizeTask(Task* task) {
    if (m_doing.contains(task))
        return;

    if (m_done.contains(task))
        return;

    QQueue<Task*> newQueue;
    newQueue.enqueue(task);
    for (auto t : m_queue){
        if (t != task)
            newQueue.enqueue(t);
    }
    m_queue = newQueue;
}

void ConcurrentTask::executeTask() {
    if(m_queue.isEmpty()) {
        emitCompleted();
        return;
    }

    for (int i = 0; i < m_maxConcurrent; i++) {
        QMetaObject::invokeMethod(this, &ConcurrentTask::executeNextSubTask, Qt::QueuedConnection);
    }
}

void ConcurrentTask::executeNextSubTask() {
    if(!isRunning())
        return;
    if(m_doing.size() >= m_maxConcurrent)
        return;
    if(m_queue.isEmpty()) {
        if(m_doing.isEmpty()){
            if(m_failed.isEmpty())
                emitCompleted();
            else
                emitFailed(QString("%1 subtask(s) failed").arg(m_failed.size()));
        }
        return;
    }

    auto task = m_queue.dequeue();
    qDebug() << "dequeued task:" << task;
    startSubTask(task);
}

void ConcurrentTask::startSubTask(Task* task) {
    if (!task) {
        qDebug() << "startSubTask: task is nullptr!";
        QMetaObject::invokeMethod(this, &ConcurrentTask::executeNextSubTask, Qt::QueuedConnection);
        return;
    }

    connect(task, &Task::completed, this, [this, task](){ subTaskSucceeded(task); });
    connect(task, &Task::failed, this, [this, task](const QString& msg){ subTaskFailed(task, msg); });
    connect(task, &Task::progress, this, [this, task](qint64 current, qint64 total, const QString& msg){ emitProgress(current, total, msg); });

    m_doing.insert(task, task);

    updateState();

    QMetaObject::invokeMethod(task, &Task::start, Qt::QueuedConnection);
}

void ConcurrentTask::subTaskSucceeded(Task* task) {
    m_doing.remove(task);
    m_done.insert(task, task);
    disconnect(task, nullptr, this, nullptr);

    emitProgress(m_done.size(), totalSize());
    QMetaObject::invokeMethod(this, &ConcurrentTask::executeNextSubTask, Qt::QueuedConnection);
}

void ConcurrentTask::subTaskFailed(Task* task, const QString& msg) {
    m_doing.remove(task);
    m_done.insert(task, task);
    m_failed.insert(task, task);
    disconnect(task, nullptr, this, nullptr);

    emitProgress(m_done.size(), totalSize());
    QMetaObject::invokeMethod(this, &ConcurrentTask::executeNextSubTask, Qt::QueuedConnection);
}

bool ConcurrentTask::abort() {
    m_queue.clear();
    if(m_doing.isEmpty()){
        emitAborted();
        return true;
    }

    for (auto task : m_doing) {
        task->disconnect(this);
        task->abort();
    }
    m_doing.clear();

    emitAborted();
    return true;
}

void ConcurrentTask::updateState() {
    emitProgress(m_done.size(), totalSize());
    emitStepChanged(
        QString("Running %1/%2, done %3")
            .arg(m_doing.size())
            .arg(totalSize())
            .arg(m_done.size())
    );
}

void ConcurrentTask::clear() {
    Q_ASSERT(!isRunning());
    m_queue.clear();
    m_doing.clear();
    m_done.clear();
    m_failed.clear();
}

int ConcurrentTask::totalSize() const {
    return m_queue.size() + m_doing.size() + m_done.size();
}