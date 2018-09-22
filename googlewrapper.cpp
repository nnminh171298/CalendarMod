#include "googlewrapper.h"

// these can be found at https://developers.google.com/calendar/v3/reference/
const QUrl clearPrimaryUrl("https://www.googleapis.com/calendar/v3/calendars/primary/clear");
const QUrl insertEventUrl("https://www.googleapis.com/calendar/v3/calendars/primary/events");
const QString revokeTokenUrl("https://accounts.google.com/o/oauth2/revoke?token=");

GoogleWrapper::GoogleWrapper(QObject *parent) : QObject(parent)
{
    makeAuthenticateDialog();

    QFile file(QDir::currentPath() + "/client_id_CalendarMod.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    file.close();

    const auto object = document.object();
    const auto settingsObject = object["web"].toObject();
    QUrl authUri(settingsObject["auth_uri"].toString());
    const auto clientId = settingsObject["client_id"].toString();
    const QUrl tokenUri(settingsObject["token_uri"].toString());
    const auto clientSecret(settingsObject["client_secret"].toString());
    const auto redirectUris = settingsObject["redirect_uris"].toArray();
    const QUrl redirectUri(redirectUris[0].toString());
    const auto port = static_cast<quint16>(redirectUri.port());

    // Google scopes: https://developers.google.com/identity/protocols/googlescopes
    google.setScope("https://www.googleapis.com/auth/calendar");
    auto replyHandler = new QOAuthHttpServerReplyHandler(port, this);
    google.setReplyHandler(replyHandler);
    google.setAuthorizationUrl(authUri);
    google.setClientIdentifier(clientId);
    google.setAccessTokenUrl(tokenUri);
    google.setClientIdentifierSharedKey(clientSecret);

    connect(&google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &GoogleWrapper::authorize);
    connect(&google, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](QAbstractOAuth::Status status)
    {
        if (status == QAbstractOAuth::Status::Granted)
            emit authorized();
    });
    connect(webEngine, &QWebEngineView::urlChanged, this, &GoogleWrapper::urlChanged);
}

GoogleWrapper::~GoogleWrapper()
{
    delete webEngine;
}

void GoogleWrapper::grant()
{
    google.grant();
}

void GoogleWrapper::clearPrimaryCalendar()
{
    QNetworkReply *reply = google.post(clearPrimaryUrl);
    connect(reply, &QNetworkReply::finished, [=]()
    {
        reply->deleteLater();
        if(reply->error() != QNetworkReply::NoError)
        {
            qCritical() << "Google error:" << reply->errorString();
            return;
        }
        emit primaryCalendarCleared();
    });
}

void GoogleWrapper::insertEvent(const QByteArray &eventData)
{
    google.setContentType(QAbstractOAuth2::ContentType::Json);
    QNetworkReply *reply = google.post(insertEventUrl, eventData);
    connect(reply, &QNetworkReply::finished, [=]()
    {
        reply->deleteLater();
        if(reply->error() != QNetworkReply::NoError)
        {
            qCritical() << "Google error:" << reply->errorString();
            return;
        }
        emit eventInserted();
    });
}

void GoogleWrapper::revokeToken()
{
    google.setContentType(QAbstractOAuth2::ContentType::WwwFormUrlEncoded);
    QNetworkReply *reply = google.post(QUrl(revokeTokenUrl + google.token()));
    connect(reply, &QNetworkReply::finished, [=]()
    {
        reply->deleteLater();
        if(reply->error() != QNetworkReply::NoError)
        {
            qCritical() << "Google error:" << reply->errorString();
            return;
        }
        emit tokenRevoked();
    });
}

void GoogleWrapper::authorize(const QUrl &url)
{
    webEngine->load(url);
    authenticateDialog->exec();
}

void GoogleWrapper::urlChanged(const QUrl &url)
{
    if(url.toString().indexOf("http://localhost:8080/") == 0)
        authenticateDialog->hide();
}

void GoogleWrapper::makeAuthenticateDialog()
{
    authenticateDialog = new QDialog();
    authenticateDialog->setWindowTitle("CalendarMod - User Google authorization");

    webEngine = new QWebEngineView();

    QGridLayout *layoutSettingDialog = new QGridLayout(authenticateDialog);
    layoutSettingDialog->addWidget(webEngine, 0, 0);
}


