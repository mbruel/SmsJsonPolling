#include "SmsJsonPolling.h"
#include "Sms.h"
#include "SmsSender.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>

SmsJsonPolling::SmsJsonPolling(QObject *parent):
    QObject(parent),
    _netMgr(new QNetworkAccessManager()),
    _serverUrls(),
    _smsQueue(), _sms(nullptr), _isSending(0x0),
    _sender(SmsSender::getInstance()),
    _jsonPolling(new QTimer()), _freqJsonReqInSec(sDefaultFreqJsonReqInSec)
{
    // QueuedConnection so we don't need a Mutex to protect the queue _smsQueue
    connect(this,    &SmsJsonPolling::sendNextSms, this, &SmsJsonPolling::onSendNextSms, Qt::QueuedConnection);

    connect(_sender, &SmsSender::sigSmsSent,       this, &SmsJsonPolling::onSmsSent,      Qt::QueuedConnection);
    connect(_sender, &SmsSender::sigSmsDelivered,  this, &SmsJsonPolling::onSmsDelivered, Qt::QueuedConnection);


    connect(&_jsonPolling, &QTimer::timeout, this, &SmsJsonPolling::onJsonPolling);    
    _startJsonPollingTimer();
}



SmsJsonPolling::~SmsJsonPolling()
{
    qDeleteAll(_serverUrls);
    _jsonPolling.stop();
    qDeleteAll(_smsQueue);
    delete _netMgr;
}

void SmsJsonPolling::addServerUrl(const QString &srvGetUrl, const QString &srvRespUrl, bool checkCetificateSSL)
{
    _serverUrls << new SmsServer(srvGetUrl, srvRespUrl, checkCetificateSSL);
}



#include <QUrlQuery>
void SmsJsonPolling::_notifyServer(SmsServer *srv, NotificationType type, bool success)
{
    qDebug() << QString("[SmsJsonPolling::_notifyServer] SMS #%1 %2: %3").arg(
                    _sms->id()).arg(notificationType(type)).arg(success ? "ok" : "KO");

//    QByteArray postData;
//    postData.append(QString("msgId=%1&res=%2&type=%3").arg(msgId).arg(success).arg(
//                        static_cast<int>(type)));
    QString data = QString("msgId=%1&type=%2&status=%3").arg(
                _sms->id()).arg(static_cast<int>(type)).arg(success);

    const QNetworkRequest &req = srv->reqNotifyStatusSMS();

    QNetworkReply *reply = _netMgr->post(req, data.toLocal8Bit());
    QObject::connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);
}



void SmsJsonPolling::onJsonReceived()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    int serverIdx = reply->property(PROPERTY_SERVER_INDEX).toInt();

    if (reply->error() == QNetworkReply::NoError)
    {
        QVariant contentType = reply->header(QNetworkRequest::ContentTypeHeader);
//        qDebug() << _time()  << "contentType: " << contentType;
        if (!contentType.isValid() || !contentType.toString().startsWith("application/json"))
        {
            _error(QString("[SmsJsonPolling::onJsonReceived] Invalid contentType from url: %1 : %2").arg(
                   reply->url().toString()).arg(contentType.toString()));
            return;
        }
        QString jsonTxt = reply->readAll();
        qDebug() << _time()  << "Json from " << reply->url().toString() << " : " << jsonTxt;

        QJsonDocument jsonDoc  = QJsonDocument::fromJson(jsonTxt.toUtf8());
        QJsonObject   json     = jsonDoc.object();
        if (json.contains("error"))
        {
            _error(QString("[SmsJsonPolling::onJsonReceived] host: %1 has sent an error: %2").arg(
                       reply->url().host()).arg(json["error"].toString()));
            return;
        }
        if (!json.contains("sms") || json["sms"].type() != QJsonValue::Array)
        {
            _error(QString("[SmsJsonPolling::onJsonReceived] host: %1 sent a unexpected JSON").arg(reply->url().host()));
            return;

        }

        QJsonArray smsArray = json["sms"].toArray();
        _log(QString("We received %1 SMS to send from %2").arg(smsArray.count()).arg(reply->url().host()));

        for (auto it = smsArray.cbegin(), itEnd = smsArray.cend(); it != itEnd ; ++it)
        {
            Sms *sms = new Sms(it->toObject(), serverIdx);
            _smsQueue << sms;
            qDebug() << _time()  << sms->str();
        }
    }
    else
        _error(QString("Error getting sms from %1: %2").arg(reply->url().host()).arg(reply->errorString()));


    reply->deleteLater();

    if (_smsQueue.size())
        emit sendNextSms();
    else
    {
        qDebug() << _time()  << "[SmsJsonPolling::onJsonReceived] no SMS => schedule next Polling";
        _startJsonPollingTimer();
    }
}

void SmsJsonPolling::onSendNextSms()
{
    if (_isSending.testAndSetOrdered(0x0, 0x1))
    {
        if (!_sms)
            _sms = _smsQueue.dequeue();

        _log(QString("Sending SMS #%1 to %2...").arg(_sms->id()).arg(_sms->dest()));
        _smsId = _sender->sendText(_sms->dest(), _sms->msg());
    }
    else
        qDebug() << _time()  << "[SmsJsonPolling::onSendNextSms] we're already sending a SMS...";

#if defined(__DEBUG__) && !defined(__ANDROID__)
    emit _sender->sigSmsSent(_smsId, true);
#endif
}

void SmsJsonPolling::onSmsSent(int msgId, bool success)
{
    _smsSenderNotification(NotificationType::SENT, msgId, success);

#if defined(__DEBUG__) && !defined(__ANDROID__)
    emit _sender->sigSmsDelivered(_smsId, true);
#endif
}

void SmsJsonPolling::onSmsDelivered(int msgId, bool success)
{
    _smsSenderNotification(NotificationType::DELIVERED, msgId, success);
}

void SmsJsonPolling::_smsSenderNotification(NotificationType type, int msgId, bool success)
{   
    if (_smsId != msgId)
    {
        qCritical() << "Unexpected sms delivered: " << msgId << " but we were expecting " << _smsId;
        return;
    }

    _log(QString("SMS #%1 %2 %3").arg(
             _sms->id()).arg(
             notificationType(type)).arg(
             success ? "ok" : "KO"));

    SmsServer *srv = _serverUrls.at(_sms->srvIdx());
    if (srv->notifyServer())
        _notifyServer(srv, type, success);

    // if the SMS is Delivered or if there is an issue during the sending
    // we can switch to the next SMS
    if (type == NotificationType::DELIVERED || !success)
    {
        delete _sms;
        _sms = nullptr;

        _isSending = 0x0;
        if (_sms || _smsQueue.size())
            emit sendNextSms();
    }
}

void SmsJsonPolling::_log(QString &&txt)
{
    txt.prepend(_time());
    qDebug() << txt;
    emit log(txt);
}

void SmsJsonPolling::_error(QString &&txt)
{
    txt.prepend(_time());
    qCritical() << txt;
    emit error(txt);
}


void SmsJsonPolling::onJsonPolling()
{    
    qDebug() << _time() << "[SmsJsonPolling::onJsonPolling] polling triggered (current _sms: " << (_sms ? _sms->id() : -1)
             << ", _smsQueue.size: " << _smsQueue.size() << ")";
    if (!_sms && _smsQueue.isEmpty())
    {
        _log("Polling triggered");
        int serverIdx = 0;
        for (SmsServer *srv : _serverUrls)
        {
            const QNetworkRequest &req = srv->reqGetSMS();
            QNetworkReply *reply = _netMgr->get(req);
            reply->setProperty(PROPERTY_SERVER_INDEX, serverIdx++);
            QObject::connect(reply, &QNetworkReply::finished, this, &SmsJsonPolling::onJsonReceived);
        }
    }
    else
    {
        _log("Polling postponed as we still have some SMS to send");
        _startJsonPollingTimer();
    }
}

QString SmsJsonPolling::firstAPIget() const
{
    if (_serverUrls.isEmpty())
        return "NOT_SET...";
    else
        return _serverUrls.first()->urlGet().toString();
}

QString SmsJsonPolling::firstAPInotify() const
{
    if (_serverUrls.isEmpty())
        return "NOT_SET...";
    else
    {
        const QUrl &url = _serverUrls.first()->urlNotify();
        if (url.isEmpty())
            return "NONE";
        else
            return url.toString();
    }
}
