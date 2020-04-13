import QtQuick 2.12
import QtQuick.Controls 2.14

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("SMS Json Polling (REST APIs)")

    Rectangle {
        id: titleRect
        width: titleTxt.paintedWidth + 20
        height: titleTxt.paintedHeight + 20
        color: "lightgray"
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter

        Text {
            id: titleTxt
            text: "SMS Polling"
            color: "darkblue"
            font.pointSize: 24
            font.bold: true
            anchors.centerIn: parent
        }
    }

    Column
    {
        id: apiInfo
        anchors.left: parent.left
        anchors.top : titleRect.bottom
        anchors.topMargin: 20
        height : 80

        Text {
            wrapMode: Text.Wrap
            text: "Get API: "+cppSmsPolling.firstAPIget()
        }
        Text {
            wrapMode: Text.Wrap
            text: "Notify API: "+cppSmsPolling.firstAPInotify()
        }
    }

    GroupBox {
        id: txtBox
        title: qsTr("Logs")

        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.topMargin: 20
        width: parent.width
        contentWidth: width
        height: parent.height - titleRect.height - apiInfo.height - 50
        contentHeight: logScroll.implicitHeight
        padding: 10

        // https://stackoverflow.com/questions/46579359/styling-groupbox-in-qml-2
        background: Rectangle {
            y: txtBox.topPadding - txtBox.padding
            width: parent.width
            height: parent.height - txtBox.topPadding + txtBox.padding
            color: "transparent"
            border.color: "#21be2b"
            border.width: 3
            radius: 10
        }

        label: Rectangle {
            anchors.left: parent.left
            anchors.leftMargin: 25
            anchors.bottom: parent.top
            anchors.bottomMargin: -height/2

            color: "white"
            width: logTitle.paintedWidth + 40
            height: logTitle.font.pixelSize
            Text {
                id: logTitle
                text: txtBox.title
                anchors.centerIn: parent
                font.pixelSize: 16
            }
        }

        ScrollView {
            id: logScroll
            width: txtBox.width - 20
            height: txtBox.height - 20
            anchors.top: parent.top

            TextArea {
                id: logTxt
//                text: qsTr("Enter your text here...")

                textFormat: TextEdit.RichText
                wrapMode: Text.WordWrap
//                selectByMouse: true
                readOnly: true

                Connections {
                    target: cppSmsSender
                    onLog:   log(txt);
                    onError: error(txt);
                }
                Connections {
                    target: cppSmsPolling
                    onLog:   log(txt);
                    onError: error(txt);
                }
            }
        }
    }

    MyButton {
        id: clearMsg
        anchors.right: parent.right
        anchors.bottom: txtBox.top
        anchors.margins: 5

        buttonText: qsTr("Clear Log")

        onClicked: {
//            console.log("Clear Log!");
            logTxt.clear();
        }
    }





    Row {
        anchors.left:  parent.left
        anchors.bottom: txtBox.top
        anchors.margins: 5

        spacing : 5

        Label {
            id: label
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Poll freq (sec)")
        }

        SpinBox {
            id: spinBox
            anchors.verticalCenter: parent.verticalCenter
            value: cppSmsPolling.pollingFreqInSec()
            stepSize: 5
            width:  120
            height: 30
        }

        MyButton {
            id: launchPolling
            anchors.verticalCenter: parent.verticalCenter
            buttonText: qsTr("ReStart Polling")

            onClicked: {
                cppSmsPolling.restartPolling(spinBox.value);
            }

        }
    }



    function log(txt){
        logTxt.append(txt);
    }

    function error(txt){
        logTxt.append("<font color='darkred'>"+txt+"</font>");
    }

}


