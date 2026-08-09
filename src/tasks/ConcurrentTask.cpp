#include "ConcurrentTask.hpp"

ConcurrentTask::ConcurrentTask(const QString& name, int max_concurrent, QObject* parent)
    : Task(parent), m_name(name), m_maxConcurrent(max_concurrent)
{}

ConcurrentTask::~ConcurrentTask() {
    abort();
}

void ConcurrentTask::addTask(Task* task) {
    if (!task) return;
    
    if (!task->parent())
        task->setParent(this);
        
    m_queue.enqueue(task);
}

void ConcurrentTask::prioritizeTask(Task* task) {
    if (!task || m_doing.contains(task) || m_done.contains(task))
        return;

    if (!task->parent())
        task->setParent(this);

    QQueue<Task*> newQueue;
    newQueue.enqueue(task);
    for (auto t : m_queue) {
        if (t != task)
            newQueue.enqueue(t);
    }
    m_queue = newQueue;

    if (isRunning() && m_doing.size() < m_maxConcurrent) {
        QMetaObject::invokeMethod(this, &ConcurrentTask::executeNextSubTask, Qt::QueuedConnection);
    }
}

void ConcurrentTask::executeTask() {
    if (m_queue.isEmpty()) {
        emitCompleted();
        return;
    }

    for (int i = 0; i < m_maxConcurrent; i++) {
        QMetaObject::invokeMethod(this, &ConcurrentTask::executeNextSubTask, Qt::QueuedConnection);
    }
}

void ConcurrentTask::executeNextSubTask() {
    if (!isRunning())
        return;
    if (m_doing.size() >= m_maxConcurrent)
        return;
    if (m_queue.isEmpty()) {
        if (m_doing.isEmpty()) {
            if (m_failed.isEmpty())
                emitCompleted();
            else
                emitFailed(QString("%1 subtask(s) failed").arg(m_failed.size()));
        }
        return;
    }

    auto task = m_queue.dequeue();
    startSubTask(task);
}

void ConcurrentTask::startSubTask(Task* task) {
    if (!task) {
        QMetaObject::invokeMethod(this, &ConcurrentTask::executeNextSubTask, Qt::QueuedConnection);
        return;
    }

    connect(task, &Task::completed, this, [this, task]() { subTaskSucceeded(task); });
    connect(task, &Task::failed, this, [this, task](const QString& msg) { subTaskFailed(task, msg); });
    
    connect(task, &Task::stepChanged, this, [this](const QString& msg) {
        emitStepChanged(msg);
    });

    m_doing.insert(task, task);
    updateState();

    QMetaObject::invokeMethod(task, &Task::start, Qt::QueuedConnection);
}

void ConcurrentTask::subTaskSucceeded(Task* task) {
    m_doing.remove(task);
    m_done.insert(task, task);
    disconnect(task, nullptr, this, nullptr);

    updateState();
    
    if (task->parent() == this)
        task->deleteLater();

    QMetaObject::invokeMethod(this, &ConcurrentTask::executeNextSubTask, Qt::QueuedConnection);
}

void ConcurrentTask::subTaskFailed(Task* task, const QString& msg) {
    Q_UNUSED(msg);
    m_doing.remove(task);
    m_done.insert(task, task);
    m_failed.insert(task, task);
    disconnect(task, nullptr, this, nullptr);

    updateState();

    if (task->parent() == this)
        task->deleteLater();

    QMetaObject::invokeMethod(this, &ConcurrentTask::executeNextSubTask, Qt::QueuedConnection);
}

bool ConcurrentTask::abort() {
    while (!m_queue.isEmpty()) {
        auto task = m_queue.dequeue();
        if (task->parent() == this)
            task->deleteLater();
    }

    for (auto task : m_doing) {
        task->disconnect(this);
        task->abort();
        if (task->parent() == this)
            task->deleteLater();
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

    while (!m_queue.isEmpty()) {
        auto task = m_queue.dequeue();
        if (task->parent() == this)
            task->deleteLater();
    }

    for (auto task : m_done.keys()) {
        if (task->parent() == this)
            task->deleteLater();
    }

    m_doing.clear();
    m_done.clear();
    m_failed.clear();
}

int ConcurrentTask::totalSize() const {
    return m_queue.size() + m_doing.size() + m_done.size();
}