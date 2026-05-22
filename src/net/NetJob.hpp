#include <QNetworkAccessManager>
#include <NetRequest.hpp>
#include <QString>
#include <QList>
#include "Task.hpp"
#include "Sink.hpp"

class NetJob : public Task {
    Q_OBJECT
public:
    explicit NetJob(
            const QString& name,
            QNetworkAccessManager* man,
            QObject* parent = nullptr);
    
    void addRequest(NetRequest::Ptr request);
    bool abort() override;

protected:
    void executeTask() override;

public slots:
    void startNextRequests();
    void onRequestFinished();
    void onRequestProgress(qint64 received, qint64 total);
          
private:
    void startRequest(int index);
    
    QString m_name;
    QNetworkAccessManager* m_nam; 
    QList<NetRequest::Ptr> m_requests;
    QList<QNetworkReply*> m_replies;

    int m_done = 0;
    int m_failed = 0;
    int m_aborted = 0;

    static constexpr int MAX_CONCURRENT = 6;
    int m_running = 0;
    int m_next = 0;
};