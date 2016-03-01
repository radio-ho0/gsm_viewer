#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QJsonArray>
#include <cstdio>
#include <cstring>
#include <QDebug>

#include "basequery.h"
#include "gsmviewer.h"
#include "ui_gsmviewer.h"
#include "gsmtrack.h"

gsmViewer::gsmViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::gsmViewer)
{
    ui->setupUi(this);
    mTracker = new GsmTrack(this);
    connect(mTracker, SIGNAL(sendCsq(int)), SLOT(slgetNum(int)));
    connect(mTracker, SIGNAL(sendData(QByteArray)), SLOT(slViewData(QByteArray)));
    connect(mTracker, SIGNAL(sendText(QString)), ui->label_2, SLOT(setText(QString)));
    connect(ui->btOK, SIGNAL(clicked(bool)), SLOT(slGetText()));
    connect(ui->leCmd, SIGNAL(returnPressed()), SLOT(slEnterPressed()));
    connect(ui->btBase, SIGNAL(clicked(bool)), SLOT(slRequestBase()));


    connect(mTracker, SIGNAL(sendBase(QString)), SLOT(slGotBase(QString)));
    connect( ui->btGetUrl, SIGNAL(clicked(bool)), SLOT(slGetUrl()));
    connect( ui->btGetPos, SIGNAL(clicked(bool)),SLOT(slGetPos()));

    qDebug("bbq");

    QString str1 = "+CREG: 2,1,\"A5D3\",\"19E420F\",2";
    QStringList list1 = str1.split(",");
    qDebug() << list1 << "  " << list1.at(2)  << endl;

    // 关闭回显
    mTracker->slExec("ate 0");

    QString strcfg = "^SYSCFG: 13,1,3FFFFFFF,1,2";
    QByteArray arrcfg = strcfg.toUtf8();
    if(arrcfg.contains("^SYSCFG: ")){
        qDebug("Yeah, it has cfg");
        int type;
        std::sscanf(arrcfg.data(), "^SYSCFG: %d,", &type);
        qDebug("the type is:: %d", type);
    }
    QString s1 = "111";
    QString s2 = "222";
    QString s3 = "333";

    QString s4 = QString("_%3_%2__%1__").arg(s1).arg(s2).arg(s3);

    qDebug() << "s4:: "  << s4 <<endl;

    QString strNum = "+CNUM: \"\",\"+8618575632907\",145";

    m_baseQuery = new BaseQuery(this);
    connect(m_baseQuery, SIGNAL(getBaseFinished(bool,QString,QString,QString)), SLOT(slGotCoor(bool,QString,QString,QString)));

}

gsmViewer::~gsmViewer()
{
    delete ui;
}

void gsmViewer::on_btExit_clicked()
{
    close();
}

void gsmViewer::slgetNum(int val)
{
    ui->lineEdit->setText( QString::number(val) );
}

void gsmViewer::slViewData(QByteArray dat)
{
    ui->textEdit->setText(dat);
}

void gsmViewer::slGetText()
{
    mTracker->slExec(ui->leCmd->text());
}

void gsmViewer::slEnterPressed()
{
    slGetText();
}

void gsmViewer::slGetUrl()
{
    qDebug() << "slGetUrl" << endl;
    // create custom temporary event loop on stack
    QEventLoop eventLoop;

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    // the HTTP request
    //QNetworkRequest req( QUrl( QString("http://time.jsontest.com/") ) );
    QString strSource = ui->leSource->text().trimmed();
    QUrl srcUrl = QUrl(strSource);
    QNetworkRequest req( srcUrl );
    //QNetworkRequest req( QUrl( QString("http://localhost/gpsapi.json") ) );
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec(); // blocks stack until "finished()" has been called

    if (reply->error() == QNetworkReply::NoError) {
        QString strReply = (QString)reply->readAll();
        //parse json
        qDebug() << "Response: " << strReply << endl;

        QJsonParseError err;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8(),&err);
        if(err.error != QJsonParseError::NoError)
        {
            qDebug() << "gsmViewer!, json error - " << err.errorString();
            return;
        }
        QJsonObject jsonObj = jsonResponse.object();

        RequestUrl = jsonObj.value("gpsapi").toString();
        ValParentvar = jsonObj.value("parentvar").toString();
        ValLatitude = jsonObj["latitude"].toString();
        ValLongitude = jsonObj["longitude"].toString();


        qDebug() << "____ longitude: " << ValLongitude;
        qDebug() << "____ latitude: " << ValLatitude;
        qDebug() << "____ parentvar: " << ValParentvar;
        qDebug() << "__ address:: " << RequestUrl;

        ui->leRequest->setText(RequestUrl);
        ui->lePrefix->setText(ValParentvar);
        ui->leLon->setText(ValLongitude);
        ui->leLat->setText(ValLatitude);

        delete reply;
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
        delete reply;
    }
}

/**
 * @brief slGetPos    获取经纬度
 */
void gsmViewer::slGetPos()
{
        // to dec

    QString strLacID = ui->leLac->text().trimmed();
    QString strCellID = ui->leCellid->text().trimmed();


    uint lacID;
    uint cellID;
    std::sscanf( qPrintable(strLacID), "%x", &lacID);
    std::sscanf( qPrintable(strCellID), "%x", &cellID);

    QString strQuery = RequestUrl
            .replace("#MCC#","460").replace("#MNC#", "01").replace("#LAC#", QString::number(lacID))
            .replace("#CID#", QString::number(cellID));
    m_baseQuery->query(strQuery, ValParentvar,ValLatitude, ValLongitude);
    qDebug() << "Q U E R Y :: " << strQuery << endl;
}

void gsmViewer::slRequestBase()
{
    mTracker->slExec("AT+CREG=2;+CREG?");
}

void gsmViewer::slGotBase(QString strBase)
{
    QString str2 =  strBase.replace("\"","");
    QStringList list = str2.split(",");
    ui->leLac->setText(list.at(0));
    ui->leCellid->setText( list.at(1) );
}

void gsmViewer::slGotCoor(bool bOK, const QString &lat, const QString &lon, const QString &address)
{
    if(bOK){
        Q_UNUSED(address);

        ui->leCoor->setText( lat + "," + lon);
    }

}

