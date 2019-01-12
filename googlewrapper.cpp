#include "googlewrapper.h"

// these can be found at https://developers.google.com/calendar/v3/reference/
const QUrl getCalendarListUrl("https://www.googleapis.com/calendar/v3/users/me/calendarList");
const QUrl calendarUrl("https://www.googleapis.com/calendar/v3/calendars");
const QString calendarUrlString("https://www.googleapis.com/calendar/v3/calendars");
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

    connect(&google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &GoogleWrapper::authorizeWithBrowser);
    connect(webEngine, &QWebEngineView::urlChanged, this, &GoogleWrapper::urlChanged);
}

GoogleWrapper::~GoogleWrapper()
{
    if(google.status() != QAbstractOAuth::Status::Granted)
        revokeToken();
    delete webEngine;
}

void GoogleWrapper::googleStart(bool isPrimary, bool clearBeforeInsert, const QByteArrayList &events, const QString &secondaryName)
{
    google.grant();

    QEventLoop waitForAuth;
    connect(&google, &QOAuth2AuthorizationCodeFlow::statusChanged, [&](QAbstractOAuth::Status status)
    {
        if(status == QAbstractOAuth::Status::Granted)
            waitForAuth.quit();
    });
    waitForAuth.exec();

    if(isPrimary)
    {
        if(clearBeforeInsert)
            clearCalendar(true);
    }
    else
    {
        bool exist = checkIfCalendarExist(secondaryName);
        if(clearBeforeInsert)
            if(exist)
            {
                clearCalendar(false);   // delete calendar
                makeSecondaryCalendar(secondaryName);
            }
        if(!exist)
            makeSecondaryCalendar(secondaryName);
    }

    emit eventLogSignal("Insert events");
    int eventCount = events.length();
    for(int i=0; i<eventCount; i++)
        insertEvent(events.at(i));
    emit eventLogSignal("Insert events", true);

    revokeToken();
}

void GoogleWrapper::authorizeWithBrowser(const QUrl &url)
{
    webEngine->load(url);
    authenticateDialog->exec();
}

void GoogleWrapper::urlChanged(const QUrl &url)
{
    if(url.toString().indexOf("http://localhost:8080/") == 0)
        authenticateDialog->hide();
}

void GoogleWrapper::clearCalendar(bool isPrimary)
{
    emit eventLogSignal("Clear calendar");

    google.setContentType(QAbstractOAuth2::ContentType::WwwFormUrlEncoded);
    QNetworkReply *reply;

    if(isPrimary)
        reply = google.post(QUrl(calendarUrlString + QString("/primary/clear")));
    else
        reply = google.deleteResource(QUrl(calendarUrlString + QString("/%1").arg(calendarId)));

    QEventLoop waitForReply;
    connect(reply, &QNetworkReply::finished, [&, this]()
    {
        reply->deleteLater();
        if(reply->error() != QNetworkReply::NoError)
            qCritical() << "Google error:" << reply->errorString();

        emit eventLogSignal("Clear calendar", true);
        waitForReply.quit();
    });
    waitForReply.exec();
}

void GoogleWrapper::insertEvent(const QByteArray &eventData)
{
    QEventLoop waitForReply;
    google.setContentType(QAbstractOAuth2::ContentType::Json);
    QNetworkReply *reply = google.post(QUrl(calendarUrlString + QString("/%1/events").arg(calendarId)), eventData);

    connect(reply, &QNetworkReply::finished, [&, this]()
    {
        reply->deleteLater();
        if(reply->error() != QNetworkReply::NoError)
            qCritical() << "Google error:" << reply->errorString();
        else
            emit eventInsertedCount(++insertEventReplyCount);
        waitForReply.quit();
    });
    waitForReply.exec();
}

void GoogleWrapper::revokeToken()
{
    emit eventLogSignal("Revoke token");

    google.setContentType(QAbstractOAuth2::ContentType::WwwFormUrlEncoded);
    QNetworkReply *reply = google.post(QUrl(revokeTokenUrl + google.token()));

    QEventLoop waitForReply;
    connect(reply, &QNetworkReply::finished, [&, this]()
    {
        reply->deleteLater();
        if(reply->error() != QNetworkReply::NoError)
            qCritical() << "Google error:" << reply->errorString();

        emit eventLogSignal("Revoke token", true);
        waitForReply.quit();
    });
    waitForReply.exec();
}

bool GoogleWrapper::checkIfCalendarExist(const QString &calendarName)
{
    emit eventLogSignal("Check calendar existance");

    bool returnValue;
    QNetworkReply *reply = google.get(getCalendarListUrl);

    QEventLoop waitForReply;
    connect(reply, &QNetworkReply::finished, [&, this]()
    {
        reply->deleteLater();
        if(reply->error() != QNetworkReply::NoError)
            qCritical() << "Google error:" << reply->errorString();

        QByteArray data = reply->readAll();
        QString dataString = QString::fromLatin1(data);
        QStringList calendarList;
        QStringList idList;
        stripCalendarNameAndId(dataString, calendarList, idList);

        returnValue = calendarList.contains(calendarName);
        if(returnValue)
            calendarId = idList.at(calendarList.indexOf(calendarName));

        emit eventLogSignal("Check calendar existance", true);
        waitForReply.quit();
    });
    waitForReply.exec();

    return returnValue;
}

void GoogleWrapper::stripCalendarNameAndId(const QString &dataString, QStringList &calendarList, QStringList &idList)
{
    QStringList dataList = dataString.split("\n");
    for(int i=0; i<dataList.length(); i++)
        if(dataList.at(i).contains("summaryOverride"))
            calendarList.replace(calendarList.length()-1, dataList.at(i).split("\"").at(3));
        else if(dataList.at(i).contains("summary"))
            calendarList.append(dataList.at(i).split("\"").at(3));
        else if(dataList.at(i).contains("id"))
            idList.append(dataList.at(i).split("\"").at(3));
}

void GoogleWrapper::makeSecondaryCalendar(const QString &calendarName)
{
    emit eventLogSignal("Add calendar");

    QJsonObject jsonObject;
    jsonObject.insert("summary", calendarName);
    QJsonDocument jsonDoc(jsonObject);
    QByteArray dataArray = jsonDoc.toJson(QJsonDocument::Compact);

    google.setContentType(QAbstractOAuth2::ContentType::Json);
    QNetworkReply *reply = google.post(calendarUrl, dataArray);

    QEventLoop waitForReply;
    connect(reply, &QNetworkReply::finished, [&, this]()
    {
        reply->deleteLater();
        if(reply->error() != QNetworkReply::NoError)
            qCritical() << "Google error:" << reply->errorString();

        QByteArray data = reply->readAll();
        QString dataString = QString::fromLatin1(data);
        QStringList calendarList;
        QStringList idList;
        stripCalendarNameAndId(dataString, calendarList, idList);

        calendarId = idList.first();

        emit eventLogSignal("Add calendar", true);
        waitForReply.quit();
    });
    waitForReply.exec();
}

void GoogleWrapper::makeAuthenticateDialog()
{
    authenticateDialog = new QDialog();
    authenticateDialog->setWindowTitle("CalendarMod - User Google authorization");

    webEngine = new QWebEngineView();

    QGridLayout *layoutSettingDialog = new QGridLayout(authenticateDialog);
    layoutSettingDialog->addWidget(webEngine, 0, 0);
}


