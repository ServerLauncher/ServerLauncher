#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <functional>

class MetaFetcher : public QObject {
    Q_OBJECT

public:
    explicit MetaFetcher(QObject *parent = nullptr);

    static constexpr const char* BASE_URL = "https://serverlauncher.github.io/meta-launcher/";
    void fetchJson(
        const QString& releativePath,
        std::function<void(QJsonDocument, QString)> onSuccess,
        std::function<void(QString)> onError
    );

    void downloadFile(
        const QString& url,
        const QString& savePath,
        std::function<void(qint64, qint64)> onProgress,
        std::function<void()> onSuccess,
        std::function<void(QString)> onError
    );

private:
    QNetworkAccessManager* manager;
};