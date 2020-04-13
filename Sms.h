#ifndef SMS_H
#define SMS_H
#include <QUrl>
class QNetworkRequest;

class QJsonObject;

class SmsServer{
private:
    const QUrl _urlGetSMS;
    const QUrl _urlNotifyStatusSMS;
    const bool _checkCetificateSSL;

    QNetworkRequest *_reqGetSMS;
    QNetworkRequest *_reqNotifyStatus;

public:
    SmsServer(const QString &host, bool https, const QString &mobileAPI, const QString &urlGet, const QString &urlNotify = "", bool checkSSL = true);

    SmsServer(const QString &urlGet, const QString &urlNotify = "", bool checkSSL = true);
    ~SmsServer();

    inline QString server() const;
    inline bool notifyServer() const;

    inline const QNetworkRequest &reqGetSMS() const;
    inline const QNetworkRequest &reqNotifyStatusSMS() const;

    inline const QUrl &urlGet() const;
    inline const QUrl &urlNotify() const;

private:
    void _init();

};

QString SmsServer::server() const { return _urlGetSMS.host(); }
bool SmsServer::notifyServer() const { return _reqNotifyStatus != nullptr; }

const QNetworkRequest &SmsServer::reqGetSMS() const { return *_reqGetSMS; }
const QNetworkRequest &SmsServer::reqNotifyStatusSMS() const{ return *_reqNotifyStatus; }
const QUrl &SmsServer::urlGet() const { return _urlGetSMS; }
const QUrl &SmsServer::urlNotify() const { return _urlNotifyStatusSMS; }



class Sms {
public:
//    Sms(int id, int srvIdx = 0);
    Sms(const QJsonObject &json, int srvIdx = 0);

    ~Sms() = default;

    static constexpr const char *JSON_ID   = "id";
    static constexpr const char *JSON_DEST = "dest";
    static constexpr const char *JSON_MSG  = "msg";

    inline int id() const;
    inline const QString &dest() const;
    inline const QString &msg() const;
    inline int srvIdx() const;

    inline QString str() const;

private:
    const int      _id;
    const QString  _dest;
    const QString  _msg;
    const int      _srvIdx; //!< in case we need to send a response upon delivery
};

int Sms::id() const { return _id; }
const QString &Sms::dest() const { return _dest; }
const QString &Sms::msg()  const { return _msg; }
int Sms::srvIdx() const { return _srvIdx; }

QString Sms::str() const
{
    return QString("Sms #%1 to %2 : %3").arg(_id).arg(_dest).arg(_msg);
}


#endif // SMS_H
