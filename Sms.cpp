#include "Sms.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QSslConfiguration>

//Sms::Sms(int id, int srvIdx):
//    _id(id), _destinations(), _msg(), _srvIdx(srvIdx)
//{}

Sms::Sms(const QJsonObject &json, int srvIdx):
    _id(json[JSON_ID].toInt()),
    _dest(json[JSON_DEST].toString()),
    _msg(json[JSON_MSG].toString()),
    _srvIdx(srvIdx)
{}


SmsServer::SmsServer(const QString &host, bool https, const QString &mobileAPI, const QString &urlGet, const QString &urlNotify, bool checkSSL):
    _urlGetSMS(QString("http%1://%2/%3?mobile=%4").arg(https ? "s" : "").arg(host).arg(urlGet).arg(mobileAPI)),
    _urlNotifyStatusSMS(QString("http%1://%2/%3?mobile=%4").arg(https ? "s" : "").arg(host).arg(urlNotify).arg(mobileAPI)),
    _checkCetificateSSL(checkSSL),
    _reqGetSMS(new QNetworkRequest(urlGet)),
    _reqNotifyStatus(urlNotify.isEmpty() ?  nullptr : new QNetworkRequest(urlNotify))

{
    _init();
}

SmsServer::SmsServer(const QString &urlGet, const QString &urlNotify, bool checkSSL):
    _urlGetSMS(urlGet), _urlNotifyStatusSMS(urlNotify), _checkCetificateSSL(checkSSL),
    _reqGetSMS(new QNetworkRequest(urlGet)),
    _reqNotifyStatus(urlNotify.isEmpty() ?  nullptr : new QNetworkRequest(urlNotify))
{
    _init();
}

SmsServer::~SmsServer()
{
    delete _reqNotifyStatus;
    delete _reqGetSMS;
}

void SmsServer::_init()
{
    _reqGetSMS->setHeader(QNetworkRequest::UserAgentHeader,   "SmsJsonPolling C++ app" );
    _reqGetSMS->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    if (!_checkCetificateSSL)// && _urlGetSMS.scheme() == "https")
    {
        QSslConfiguration conf = QSslConfiguration::defaultConfiguration();
        conf.setPeerVerifyMode(QSslSocket::VerifyNone);
        _reqGetSMS->setSslConfiguration(conf);
    }

    if (_reqNotifyStatus)
    {
        _reqNotifyStatus->setHeader(QNetworkRequest::UserAgentHeader,   "SmsJsonPolling C++ app" );
        _reqNotifyStatus->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        if (!_checkCetificateSSL) // && _urlNotifyStatusSMS.scheme() == "https")
        {
            QSslConfiguration conf = QSslConfiguration::defaultConfiguration();
            conf.setPeerVerifyMode(QSslSocket::VerifyNone);
            _reqNotifyStatus->setSslConfiguration(conf);
        }
    }
}
