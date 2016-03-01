#ifndef BASEQUERY_H
#define BASEQUERY_H
  
#include <QObject>  
#include <QNetworkAccessManager>
class QNetworkReply;

/**
 * @brief The BaseQuery class 获取基站位置
 */

class BaseQuery : public QObject
{  
    Q_OBJECT  
public:  
    BaseQuery(QObject *parent = 0);
    ~BaseQuery();

    /**
     * @brief query         查询基站位置
     * @param strUrl        查找路径
     * @param strPrefix     前置路径 （经纬度所在的Json对象）
     * @param strLat        经度名称
     * @param strLon        纬度名称
     */
    void query(const QString &strUrl, const QString& strPrefix, const QString& strLat, const QString& strLon);
signals:  
    //   发送最终基站位置
    void getBaseFinished(bool bOK, const QString &lat, const QString &lon, const QString &address);
  
protected slots:  

    void onReplyFinished(QNetworkReply *reply);  
  
private:  
    QNetworkAccessManager m_nam;  
    QString m_strPrefix;
    QString m_strlat;
    QString m_strLon;

    /**
     * @brief ParseJsonValue  获取字符串(无论 double型 和 字符型)
     * @return
     */
    QString parseJsonValue(const QJsonValue &val);
};  
#endif  
