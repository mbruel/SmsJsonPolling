TEMPLATE = app
TARGET = SmsPolling

QT += quick network

CONFIG += c++11

DEFINES += __SmsSender_SINGALS__
DEFINES -= __POPUP_SMS_STATUS__
DEFINES += __LOG_QT_NATIVE__

android: {
    QT += androidextras
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

    # git clone --depth=1 https://github.com/KDAB/android_openssl
#    include(/home/bruel/android/android_openssl/openssl.pri)

    # OpenSSL_1_1_1d built for arm64 with ndk-r21 and no-asm option
    ANDROID_EXTRA_LIBS += \
        $$PWD/android/libs/libcrypto_1_1.so \
        $$PWD/android/libs/libssl_1_1.so

    SOURCES += native.cpp
}

CONFIG(debug, debug|release) : {
    DEFINES += __DEBUG__
}


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        Sms.cpp \
        SmsJsonPolling.cpp \
        SmsSender.cpp \
        main.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
#QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
#QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Singleton.h \
    Sms.h \
    SmsJsonPolling.h \
    SmsSender.h


OTHER_FILES += \
    android-sources/src/com/mbruel/test/*.java

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/src/com/mbruel/test/MyActivity.java \
    android/src/com/mbruel/test/NativeFunctions.java

