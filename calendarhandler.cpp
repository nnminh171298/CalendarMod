#include "calendarhandler.h"
#include <QDebug>

const QString calendarUrl("http://www.bet.puv.fi/schedule/ical/");

CalendarHandler::CalendarHandler(QObject *parent) : QObject(parent)
{
    connect(&webCtrl, &QNetworkAccessManager::finished, this, &CalendarHandler::fileDownloaded);
}

// get the .ics file from the Url
void CalendarHandler::getCalendar(const QString &groupCode)
{
    QNetworkRequest request(calendarUrl + groupCode + ".ics");
    webCtrl.get(request);
}

int CalendarHandler::getEventCount()
{
    return eventCount;
}

QByteArray CalendarHandler::getEventDataAt(int index)
{
    return modifiedData.at(index);
}

void CalendarHandler::fileDownloaded(QNetworkReply *reply)
{
    QByteArray downloadedData = reply->readAll();
    dataString = QString(downloadedData);
    if(!dataString.isEmpty())
        modifyCalendar();
}

void CalendarHandler::modifyCalendar()
{
    changeDescription();
    changeSummary();
    changeLocation();

    createEventsData();

    emit calendarReady();
}

void CalendarHandler::changeDescription()
{
    int searchBeginIndexFrom = 0;
    int searchEndIndexFrom;
    while(true)
    {
        // search indexes of the first and last character
        int beginIndex = dataString.indexOf("DESCRIPTION:", searchBeginIndexFrom);
        if(beginIndex == -1)
            break;
        searchEndIndexFrom = beginIndex;
        int endIndex = dataString.indexOf("\r\n", searchEndIndexFrom);
        int length = endIndex - beginIndex;

        // create a new description line
        QString line = dataString.mid(beginIndex, length);
        QStringList strList = line.split(":");
        QString teacherInitial = strList.at(strList.length()-2).split(" ").last();
        QStringList newStrList;
        newStrList.append(strList.first());
        newStrList.append(teacherInitial);
        newStrList.append(strList.last());
        QString newLine = newStrList.join(":");

        dataString.replace(beginIndex, length, newLine);
        searchBeginIndexFrom = beginIndex+1;
    }
}

void CalendarHandler::changeSummary()
{
    int searchBeginIndexFrom = 0;
    int searchEndIndexFrom;
    while(true)
    {
        // search indexes of the first and last character
        int beginIndex = dataString.indexOf("SUMMARY:", searchBeginIndexFrom);
        if(beginIndex == -1)
            break;
        searchEndIndexFrom = beginIndex;
        int endIndex = dataString.indexOf("\r\n", searchEndIndexFrom);
        int length = endIndex - beginIndex;

        // create a new summary line
        QString line = dataString.mid(beginIndex, length);
        QStringList strList = line.split(":");
        QStringList newStrList;
        newStrList.append(strList.first());
        newStrList.append(strList.last());
        QString newLine = newStrList.join(":");

        dataString.replace(beginIndex, length, newLine);
        searchBeginIndexFrom = beginIndex+1;
    }
}

void CalendarHandler::changeLocation()
{
    int searchBeginIndexFrom = 0;
    int searchEndIndexFrom;
    while(true)
    {
        // search indexes of the first and last character
        int beginIndex = dataString.indexOf("LOCATION:", searchBeginIndexFrom);
        if(beginIndex == -1)
            break;
        searchEndIndexFrom = beginIndex;
        int endIndex = dataString.indexOf("\r\n", searchEndIndexFrom);
        int length = endIndex - beginIndex;

        // create a new summary line
        QString line = dataString.mid(beginIndex, length);
        QStringList strList = line.split(":");
        QStringList newStrList;
        newStrList.append(strList.at(0));
        newStrList.append(strList.at(1));
        QString newLine = newStrList.join(":");

        dataString.replace(beginIndex, length, newLine);
        searchBeginIndexFrom = beginIndex+1;
    }
}

void CalendarHandler::createEventsData()
{
    int searchBeginIndexFrom = 0;
    int searchEndIndexFrom;
    while(true)
    {
        // search indexes of the first and last character
        int beginIndex = dataString.indexOf("BEGIN:VEVENT", searchBeginIndexFrom);
        searchEndIndexFrom = beginIndex+1;
        int endIndex = dataString.indexOf("BEGIN:VEVENT", searchEndIndexFrom);
        int length = endIndex - beginIndex;

        // create event data
        QString eventString = dataString.mid(beginIndex, length);
        // omit last "\r\n"
        eventString.chop(4);
        QStringList lineList = eventString.split("\r\n");

        QJsonObject eventObject;
        for(int i=0; i<lineList.length(); i++)
        {
            QString line = lineList.at(i);
            QStringList subStringList = line.split(":");
            QString key = subStringList.takeFirst();
            QString value = subStringList.join(":");
            insertValue(&eventObject, key, value);
        }
        QJsonDocument jsonDoc(eventObject);
        QByteArray dataArray = jsonDoc.toJson(QJsonDocument::Compact);
        modifiedData.append(dataArray);

        eventCount++;
        if(endIndex == -1)
            break;
        searchBeginIndexFrom = endIndex;
    }
}

void CalendarHandler::insertValue(QJsonObject *eventObject, const QString &key, const QString &value)
{
    if(key == "DTSTART" || key == "DTEND")
    {
        QJsonObject subObject;
        QString subKey = "dateTime";
        QString subValue = value;
        subValue.insert(4, "-");
        subValue.insert(7, "-");
        subValue.insert(13, ":");
        subValue.insert(16, ":");
        subObject.insert(subKey, subValue);

        eventObject->insert(key.right(key.size()-2).toLower(), subObject);
        return;
    }
    if(key == "LOCATION" || key == "DESCRIPTION" || key == "SUMMARY")
    {
        eventObject->insert(key.toLower(), value);
        return;
    }
}
