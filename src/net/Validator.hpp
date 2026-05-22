#pragma once  
#include <QNetworkRequest>  
#include <QNetworkReply>  
#include <QByteArray>

class Validator {
public:
    Validator() = default;
    virtual ~Validator() = default;

    virtual bool init(QNetworkRequest& request) = 0;
    virtual bool write(QByteArray* data) = 0;
    virtual bool abort() = 0;
    virtual bool validate(QNetworkReply& reply) = 0;
};