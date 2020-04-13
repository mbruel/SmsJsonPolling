#ifndef SmsJsonPolling_H
#define SmsJsonPolling_H
#include "Singleton.h"
#include <QQueue>
#include <QUrl>
#include <QTimer>
#include <QTime>
class QNetworkAccessManager;
class Sms;
struct SmsServer;
class SmsSender;

using AtomicBool = QAtomicInteger<unsigned short>; // 16 bit only (faster than using 8 bit variable...)
class SmsJsonPolling : public QObject, public Singleton<SmsJsonPolling>
{
    Q_OBJECT
    friend class Singleton<SmsJsonPolling>;

private:
    static const ushort sDefaultFreqJsonReqInSec = 60;

    QNetworkAccessManager *_netMgr;
    QList<SmsServer*>      _serverUrls;

    QQueue<Sms*>           _smsQueue;
    Sms                   *_sms;                //!< current sms that is sent
    AtomicBool             _isSending;

    SmsSender             *_sender;
    int                    _smsId;

    QTimer                 _jsonPolling;
    ushort                 _freqJsonReqInSec;

    enum class NotificationType : bool {SENT= 0, DELIVERED = 1};


    static constexpr const char *PROPERTY_SERVER_INDEX = "PROPERTY_SERVER_INDEX";

public:
    ~SmsJsonPolling();

    void addServerUrl(const QString &srvGetUrl, const QString &srvRespUrl = "", bool checkCetificateSSL = true);

    Q_INVOKABLE inline int pollingFreqInSec() const;
    Q_INVOKABLE inline void restartPolling(int newFreq);

    Q_INVOKABLE QString firstAPIget() const;
    Q_INVOKABLE QString firstAPInotify() const;

signals:
    void sendNextSms();
    void error(const QString &txt);
    void log(const QString &txt);


public slots:
    void onJsonPolling();


private slots:
    void onJsonReceived();
    void onSendNextSms();

    void onSmsSent(int msgId, bool success);
    void onSmsDelivered(int msgId, bool success);

private:
    SmsJsonPolling(QObject *parent = nullptr);

    void _notifyServer(SmsServer *srv, NotificationType type, bool success);

    inline void _startJsonPollingTimer();
    inline QString _time() const;

    void _smsSenderNotification(NotificationType type, int msgId, bool success);
    inline const char * notificationType(NotificationType type);

    void _log(QString &&txt);
    void _error(QString &&txt);
};

int SmsJsonPolling::pollingFreqInSec() const { return _freqJsonReqInSec; }

void SmsJsonPolling::restartPolling(int newFreq)
{
    if (newFreq > 0)
    {
        _freqJsonReqInSec = static_cast<ushort>(newFreq);
        emit log(tr("new polling frequency: %1").arg(newFreq));
        onJsonPolling();
    }
    else
    {
        emit log("Stop polling!");
        _jsonPolling.stop();
    }

}



void SmsJsonPolling::_startJsonPollingTimer()
{
    _jsonPolling.start(_freqJsonReqInSec*1000);
}

QString SmsJsonPolling::_time() const
{
    return QString("[%1] ").arg(QTime::currentTime().toString("hh:mm:ss.zzz"));
}

const char *SmsJsonPolling::notificationType(SmsJsonPolling::NotificationType type)
{
    return type == NotificationType::SENT ? "Sent" : "Delivered";
}

#endif // GETSMSJSON_H
