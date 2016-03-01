#include "gsmtrack.h"
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <cstdio>
#include <cstring>
#include <QDebug>


GsmTrack::GsmTrack(QObject *parent) : QObject(parent)
{
    nextCmd.clear();
    trackTimer = new QTimer(this);
    trackTimer->setInterval(1500);
    connect(trackTimer, SIGNAL(timeout()), SLOT(slUpdateGSM()));
    initStr();

    gsmSerial = new QSerialPort(this);
    connect(gsmSerial, SIGNAL(readyRead()), SLOT(slReadSerial()));

    QString PortName;
    // 假如只有一个设备
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
       QString description = info.description();

#define _GSMTRACK_TEST_XP_x
#ifdef _GSMTRACK_TEST_XP_
       if(description.contains("UI Interface", Qt::CaseInsensitive)){
#else
       if(description.contains("Control", Qt::CaseInsensitive)){
#endif
            PortName = info.portName();
        }
    }

    gsmSerial->setPortName( PortName );
    gsmSerial->setBaudRate( QSerialPort::Baud9600 );
    gsmSerial->setDataBits(QSerialPort::Data8);
    bool isOK = gsmSerial->open(QIODevice::ReadWrite);
    if(isOK){
        qDebug("open successful");
    }else{
        qDebug("open failed");
    }
    trackTimer->start();
}

GsmTrack::~GsmTrack()
{
    if(gsmSerial->isOpen()){
        gsmSerial->close();
    }
}

void GsmTrack::slUpdateGSM()
{
    gsmSerial->flush();
    if(nextCmd.isEmpty()){
        gsmSerial->write( getCmd(strcsq) );
    }else{
        gsmSerial->write( getCmd(nextCmd.takeFirst()) );
    }
}

void GsmTrack::slReadSerial()
{
    QByteArray data = gsmSerial->readAll().trimmed();
    //qDebug() << "the date:: " << data << endl;
    emit sendData(data);
    if(data.contains("+CSQ: ")){
        int csq;
        int csq_tail;
        std::sscanf(data.data(), "+CSQ: %d,%d", &csq, &csq_tail);
        qDebug("the csq is:: %d\n", csq);

        if(csq <= 10 || csq > 31){
            emit sendCsq(0);
            return;
        }
        emit sendCsq(csq);
    }

    // 读取网络类型与网路配置
    if(data.contains("^SYSCFG: ")){
        int type;
        std::sscanf(data.data(), "^SYSCFG: %d", &type);
        if(type == 13){
            emit sendText("2G network it is!!");
        }else if(type == 2){
            emit sendText("3G network it is!!");
        }
    }

    // 基站信息
    // BASE:: "AT+CREG=2;+CREG?"
    // OUT :: +CREG: 2,1,"A5D3","19E420F",2
    if(data.contains("+CREG: ")){
        qDebug() << data << endl;
        QByteArrayList cregList =  data.split(',');
        emit sendBase(QLatin1String( cregList.at(2) + "," + cregList.at(3) ));
    }


}

QByteArray GsmTrack::slSingleCmd(QString cmd)
{
    QByteArray ret;
    if(gsmSerial->isOpen()){
        return ret;
    }else{
        gsmSerial->setBaudRate( QSerialPort::Baud9600 );
        gsmSerial->setDataBits(QSerialPort::Data8);
        gsmSerial->open(QIODevice::ReadWrite);
        const  char* req =  qPrintable( getCmd(cmd) );
        qint64 len = std::strlen(req);
        gsmSerial->flush();

        qint64 nWritten = gsmSerial->write(req, len);
        if(nWritten == len){
            ret = gsmSerial->readAll().trimmed();
        }
        gsmSerial->close();
    }
    return ret;
}

void GsmTrack::slExec(QString cmd)
{
    nextCmd.push_back(cmd);
}

void GsmTrack::initStr()
{
    str2Gmode = QStringLiteral("AT^SYSCFG=13,1,3FFFFFFF,1,2");
    str3Gmode = QStringLiteral("AT^SYSCFG=2,1,3FFFFFFF,1,2");
    str3gnet = QStringLiteral("AT+CGDCONT=1,\"IP\",\"3gnet\"");
    str3gwap = QStringLiteral("AT+CGDCONT=1,\"IP\",\"3gwap\"");
    strcsq = QStringLiteral("AT+CSQ");
    strcfg = QStringLiteral("AT^SYSCFG?");
    strbase = QStringLiteral("AT+CREG=2;+CREG?");
}

QByteArray GsmTrack::getCmd(QString cmd)
{
    QByteArray arr1 = cmd.append("\r\n").toUtf8();

    return arr1;
}

