#include <QNetworkAccessManager>
#include <QString>
#include <QList>
#include "tasks/ConcurrentTask.hpp"
#include "Sink.hpp"
#include "NetRequestTask.hpp"

class NetJob : public ConcurrentTask {
    Q_OBJECT
public:
    explicit NetJob(const QString& name, QNetworkAccessManager* man, QObject* parent = nullptr);

    void addRequest(NetRequest::Ptr request);

private:
    QNetworkAccessManager* m_nam;
};