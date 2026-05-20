#include "MetaFetcher.hpp"
#include <QNetworkRequest>
#include <QFile>

MetaFetcher::MetaFetcher(QObject *parent): 
    QObject(parent), 
    manager(new QNetworkAccessManager(this)) 
        {}

void MetaFetcher::fetchJson(
    const QString& releativePath,
    std::function<void(QJsonDocument, QString)> onSuccess,
    std::function<void(QString)> onError) {
        QString url = QString(BASE_URL) + releativePath;
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader, "ServerLauncher/1.0");

        QNetworkReply* reply = manager->get(request);

        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();

            if(reply->error() != QNetworkReply::NoError) {
                onError(reply->errorString());
                return;
            }

            QByteArray data = reply->readAll();
            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);

            if(parseError.error != QJsonParseError::NoError) {
                onError(parseError.errorString());
                return;
            }

            onSuccess(jsonDoc, QString(data));
        });
    }

void MetaFetcher::downloadFile(
    const QString& url,
    const QString& savePath,
    std::function<void(qint64, qint64)> onProgress,
    std::function<void()> onSuccess,
    std::function<void(QString)> onError
) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "ServerLauncher/1.0");
    request.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy
    );

    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::downloadProgress, this, [=](qint64 recived, qint64 total) {
        onProgress(recived, total);
    });

    connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();

            if(reply->error() != QNetworkReply::NoError) {
                onError(reply->errorString());
                return;
            }

            QFile file(savePath);
            if(!file.open(QIODevice::WriteOnly)) {
                onError(file.errorString());
                return;
            }
            file.write(reply->readAll());
            file.close();

            onSuccess();
        }
    );
}