#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "SmsSender.h"
#include "SmsJsonPolling.h"
#ifdef __DEBUG__
#include <QSslSocket>
#endif


#define __LOCAL_DB__ 1

#ifdef __LOCAL_DB__
  #define SRV_HOST   "http://192.168.1.42"
  #define SSL_CHECK  1

  #define STATIC_JSON "/sms.json"
#else
  #define SRV_HOST   "https://xxx.xxx.xxx.xxx"
  #define SSL_CHECK  0
#endif

#define URL_GET    "/smsGet.cgi"
#define URL_NOTIFY "/smsNotify.cgi"
#define MOBILE_API "c171c0acb21adaa3fb34f1f05e9467e3"

int main(int argc, char *argv[])
{

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

#ifdef __DEBUG__
    qDebug() << QString("SSL support: %1, build version: %2, system version: %3").arg(QSslSocket::supportsSsl() ? "yes" : "no").arg(
             QSslSocket::sslLibraryBuildVersionString()).arg(QSslSocket::sslLibraryVersionString());
#endif

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    SmsJsonPolling *smsPolling = SmsJsonPolling::getInstance();

#ifdef STATIC_JSON
    smsPolling->addServerUrl(QString("%1%2").arg(SRV_HOST).arg(STATIC_JSON));
#else
    smsPolling->addServerUrl(
                QString("%1%2?mobile=%3").arg(SRV_HOST).arg(URL_GET).arg(MOBILE_API),
                QString("%1%2?mobile=%3").arg(SRV_HOST).arg(URL_NOTIFY).arg(MOBILE_API),
                SSL_CHECK
                );
#endif

    smsPolling->onJsonPolling();

#ifdef __DEBUG__
    engine.rootContext()->setContextProperty("cppSmsSender",  SmsSender::getInstance());
#endif

    engine.rootContext()->setContextProperty("cppSmsPolling", smsPolling);

    engine.load(url); 

    return app.exec();
}
