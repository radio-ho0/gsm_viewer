#ifndef GSMVIEWER_H
#define GSMVIEWER_H

#include <QWidget>
class GsmTrack;
class BaseQuery;

namespace Ui {
class gsmViewer;
}

class gsmViewer : public QWidget
{
    Q_OBJECT

public:
    explicit gsmViewer(QWidget *parent = 0);
    ~gsmViewer();

signals:
    void execCmd(QString cmd);


private slots:
    void on_btExit_clicked();
    void slgetNum(int val);
    void slViewData(QByteArray dat);
    void slGetText();
    void slEnterPressed();

    void slGetUrl();

    /**
     * @brief slGetPos    获取经纬度
     */
    void slGetPos();

    /**
     * @brief slGetBase
     * @details 获取基站信息
     */
    void slRequestBase();

    void slGotBase(QString strBase);

    // 得到经纬度
    void  slGotCoor(bool bOK, const QString &lat, const QString &lon, const QString &address);

private:
    Ui::gsmViewer *ui;
    GsmTrack* mTracker;

    QString ValLongitude;
    QString ValLatitude;
    QString ValParentvar;
    QString RequestUrl;

    BaseQuery* m_baseQuery;
};

#endif // GSMVIEWER_H
