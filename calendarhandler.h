#ifndef CALENDARHANDLER_H
#define CALENDARHANDLER_H

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextCodec>
#include <QJsonObject>
#include <QJsonDocument>

class CalendarHandler : public QObject
{
    Q_OBJECT
public:
    explicit CalendarHandler(QObject *parent = nullptr);

signals:
    void calendarReady();

public slots:
    void getCalendar(const QString &groupCode);
    int getEventCount();
    QByteArray getEventDataAt(int index);

private slots:
    void fileDownloaded(QNetworkReply* reply);

private:
    void modifyCalendar();
    void changeDescription();
    void changeSummary();
    void changeLocation();
    void createEventsData();
    void insertValue(QJsonObject *eventObject, const QString &key, const QString &value);

    QNetworkAccessManager webCtrl;
    QString dataString;
    QByteArrayList modifiedData;
    int eventCount = 0;
};

#endif // CALENDARHANDLER_H
