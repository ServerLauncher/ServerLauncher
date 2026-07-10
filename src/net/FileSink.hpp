#pragma once
#include "Sink.hpp"
#include <QSaveFile>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <memory>

class FileSink : public Sink {
public:
    explicit FileSink(const QString& destPath)
        : m_destPath(destPath)
    { }

    Task::State init(QNetworkRequest& request) override {
        QDir().mkpath(QFileInfo(m_destPath).absolutePath());

        m_file = std::make_unique<QSaveFile>(m_destPath);
        if (!m_file->open(QIODevice::WriteOnly)) {
            setErrorMessage(QString("Cannot open file for writing: %1").arg(m_file->errorString()));
            return Task::State::Failed;
        }

        if (!initAllValidators(request)) {
            return Task::State::Failed;
        }
        return Task::State::Running;
    }

    Task::State write(QByteArray* data) override {
        if (!writeAllValidators(data)) {
            return Task::State::Failed;
        }

        const qint64 written = m_file->write(*data);
        if (written == -1) {
            setErrorMessage(QString("Write failed: %1").arg(m_file->errorString()));
            return Task::State::Failed;
        }
        if (written != data->size()) {
            setErrorMessage(QString("Partial write: %1 of %2 bytes")
                .arg(written).arg(data->size()));
            return Task::State::Failed;
        }

        return Task::State::Running;
    }

    Task::State finalize(QNetworkReply& reply) override {
        if (!validateAllValidators(reply)) {
            m_file->cancelWriting();
            return Task::State::Failed;
        }

        if (!m_file->commit()) {
            setErrorMessage(QString("Commit failed: %1").arg(m_file->errorString()));
            return Task::State::Failed;
        }

        return Task::State::Completed;
    }

    Task::State abort() override {
        abortAllValidators();
        if (m_file) {
            m_file->cancelWriting();
        }
        return Task::State::Aborted;
    }

    bool hasLocalData() override {
        return QFileInfo::exists(m_destPath);
    }
    
private:
    QString m_destPath;
    std::unique_ptr<QSaveFile> m_file;
};