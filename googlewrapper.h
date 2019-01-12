#ifndef GOOGLEWRAPPER_H
#define GOOGLEWRAPPER_H_H

#include <QObject>
#include <QDialog>
#include <QDebug>
#include <QtNetworkAuth>
#include <QOAuth2AuthorizationCodeFlow>
#include <QDesktopServices>
#include <QWebEngineView>
#include <QGridLayout>

class GoogleWrapper : public QObject
{
    Q_OBJECT
public:
    explicit GoogleWrapper(QObject *parent = nullptr);
    ~GoogleWrapper();

signals:
    void eventLogSignal(const QString &message, const bool &done = false);
    void eventInsertedCount(int count);

public slots:
    void googleStart(bool isPrimary, bool clearBeforeInsert, const QByteArrayList &events, const QString &secondaryName = QString());

private slots:
    void authorizeWithBrowser(const QUrl &url);
    void urlChanged(const QUrl &url);

private:
    void grant();
    void clearCalendar(bool isPrimary);
    void insertEvent(const QByteArray &eventData);
    void revokeToken();
    bool checkIfCalendarExist(const QString &calendarName);
    void stripCalendarNameAndId(const QString &dataString, QStringList &calendarList, QStringList &idList);
    void makeSecondaryCalendar(const QString &calendarName);

    void makeAuthenticateDialog();

    QOAuth2AuthorizationCodeFlow google;

    QDialog *authenticateDialog;
    QWebEngineView *webEngine;
    QString calendarId = "primary";
    int insertEventReplyCount = 0;
};

#endif // OAUTH2_H
