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
    void authorized();
    void primaryCalendarCleared();
    void eventInserted();
    void tokenRevoked();

public slots:
    void grant();
    void clearPrimaryCalendar();
    void insertEvent(const QByteArray &eventData);
    void revokeToken();

private slots:
    void authorize(const QUrl &url);
    void urlChanged(const QUrl &url);

private:
    void makeAuthenticateDialog();
    QOAuth2AuthorizationCodeFlow google;

    QDialog *authenticateDialog;
    QWebEngineView *webEngine;
};

#endif // OAUTH2_H
