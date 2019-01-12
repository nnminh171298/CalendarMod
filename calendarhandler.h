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
    friend class MainWindow;

public:
    explicit CalendarHandler(QObject *parent = nullptr);

signals:
    void eventLogSignal(const QString &message, const bool &done = false);
    void calendarReady(const QStringList &courseNameList, const QByteArrayList &eventData);

public slots:
    void getCalendar(const QString &groupCode);

private slots:
    void fileDownloaded(QNetworkReply* reply);

private:
    void modifyCalendar();
    void changeDescription();
    void changeSummary();
    void changeLocation();
    void createEventsData(const QStringList &omitCourses = QStringList());
    void insertValue(QJsonObject *eventObject, const QString &key, const QString &value);
    QString getSummary(const QString &event);

    QNetworkAccessManager webCtrl;
    QString dataString;
    QByteArrayList modifiedData;
};

#endif // CALENDARHANDLER_H
