#include <QJsonDocument>  
#include <QByteArray>  
#include <QHostAddress>  
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>  
#include <QJsonArray>  
#include <QTextCodec>  
#include <cstdio>
#include <QDebug>  

#include "basequery.h"
  
BaseQuery::BaseQuery(QObject *parent)
    : QObject(parent)  
    , m_nam(this)  
{  
    connect(&m_nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReplyFinished(QNetworkReply*)));
}  
  
BaseQuery::~BaseQuery()
{  
  
}  
  
/**
 * @brief query         查询基站位置
 * @param strUrl        查找路径
 * @param strPrefix     前置路径 （经纬度所在的Json对象）
 * @param strLat        经度名称
 * @param strLon        纬度名称
 */
void BaseQuery::query(const QString &strUrl, const QString &strPrefix, const QString &strLat, const QString &strLon)
{  

    QUrl url(strUrl);
    QNetworkRequest req(url);  
    req.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");  
    req.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.114 Safari/537.36");  
    QNetworkReply *reply = m_nam.get(req);  

    Q_UNUSED(reply);
    m_strPrefix = strPrefix;
    m_strlat = strLat;
    m_strLon  = strLon;
    //reply->setProperty("lon", strlac);
}  
  
QString BaseQuery::parseJsonValue(const QJsonValue& val)
{
    QString strResult;
    if(val.isDouble()){
            double dval = val.toDouble();
            char strVal[32] = {0};
            std::sprintf(strVal, "%.7f", dval);
            strResult = QString(strVal);
    }else{
        strResult = val.toString();
    }
    return strResult;
}

  
void BaseQuery::onReplyFinished(QNetworkReply *reply)
{  

    reply->deleteLater();  
    if(reply->error() != QNetworkReply::NoError)  
    {  
        qDebug() << "BaseQuery, error - " << reply->errorString();
        emit getBaseFinished(false, "", "", "");
        return;  
    }  
  
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();  
    //qDebug() << "IpQuery, status - " << status ;  
    if(status != 200)  
    {  
        emit getBaseFinished(false, "", "", "");
        return;  
    }  
  
    QByteArray data = reply->readAll();  
    QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();  
    //qDebug() << "contentType - " << contentType;  
    int charsetIndex = contentType.indexOf("charset=");  
    if(charsetIndex > 0)  
    {  
        charsetIndex += 8;  
        QString charset = contentType.mid(charsetIndex).trimmed().toLower();  
        if(charset.startsWith("gbk") || charset.startsWith("gb2312"))  
        {  
            QTextCodec *codec = QTextCodec::codecForName("GBK");  
            if(codec)  
            {  
                data = codec->toUnicode(data).toUtf8();  
            }  
        }  
    }  
  
    int parenthesisLeft = data.indexOf('(');  
    int parenthesisRight = data.lastIndexOf(')');  
    if(parenthesisLeft >=0 && parenthesisRight >=0)  
    {  
        parenthesisLeft++;  
        data = data.mid(parenthesisLeft, parenthesisRight - parenthesisLeft);  
    }  
    QJsonParseError err;  
    QJsonDocument json = QJsonDocument::fromJson(data, &err);  
    if(err.error != QJsonParseError::NoError)  
    {  
        qDebug() << "BaseQuery, json error - " << err.errorString();
        emit getBaseFinished(false, "", "", "");
        return;  
    }  
    QJsonObject obj = json.object();  

    QString strLat;
    QString strLon;
    QString address;

    if(m_strPrefix.isEmpty()){
        strLat  = parseJsonValue( obj.value(m_strlat) );
        strLon  = parseJsonValue( obj.value(m_strLon) );
        address = QStringLiteral("Earth");
        //QString address = obj.value("address").toString(QStringLiteral("EARTH"));

    }else{
        QJsonValue val = obj.value("m_strPrefix");
        if(val.isObject()){
            QJsonObject resultObj = val.toObject();
            strLat  = parseJsonValue( resultObj.value(m_strlat) );
            strLon  = parseJsonValue( resultObj.value(m_strLon) );
            address = QStringLiteral("Earth");
        }
    }   // m_strPrefix.isEmpty()

    // 准备上传
    if( strLat.isEmpty() || strLon.isEmpty() ){
        return;
    }

    qDebug("++++ baseQuery %s, %s", qPrintable(strLat), qPrintable(strLon) );
    emit getBaseFinished(true, strLat, strLon, address);
}

