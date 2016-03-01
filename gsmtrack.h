#ifndef GSMTRACK_H
#define GSMTRACK_H

#include <QObject>
class QTimer;
class QSerialPort;

/**
 * @brief The GsmTrack class
 * @details gsm 探测控制类
 */

class GsmTrack : public QObject
{
    Q_OBJECT
public:
    explicit GsmTrack(QObject *parent = 0);
    ~GsmTrack();

signals:
    void sendCsq(int csq);
    void sendData(QByteArray dat);
    void sendText(QString text);
    void sendBase(QString strBase);
public slots:
    QByteArray slSingleCmd(QString cmd);
    void slExec(QString cmd);

private slots:
    void slUpdateGSM();
    void slReadSerial();


private:
    QTimer* trackTimer;
    QSerialPort* gsmSerial;

    QString str2Gmode;
    QString str3Gmode;
    QString str3gnet;
    QString str3gwap;
    QString strcsq;
    QString strcfg;
    QString strbase;

    /**
     * @brief initStr
     * @details 初始化命令
     */
    void initStr();

    QByteArray getCmd(QString cmd);
    QStringList nextCmd;

};

#endif // GSMTRACK_H
