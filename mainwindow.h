#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QSettings>
#include <QGroupBox>
#include <QProgressBar>
#include <QRadioButton>
#include <QButtonGroup>
#include <QStandardItemModel>
#include <QTreeView>
#include <QHeaderView>

QT_BEGIN_NAMESPACE
class GoogleWrapper;
class CalendarHandler;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void getCalendar(const QString &groupCode);
    void googleStart(bool isPrimary, bool clearBeforeInsert, const QByteArrayList &events, const QString &secondaryName = QString());

private slots:
    void getCalendarButtonClicked();
    void changeColumnButtonClicked();
    void updateEventButtonClicked();
    void googleStartButtonClicked();

    void calendarReady(const QStringList &courseNameList, const QByteArrayList &eventData);

    void updateEventView(const QString &message, const bool &done = false);

private:
    void makeGetCalendarBox();
    void makeModCalendarBox();
    void makeEventLog();
    void makeAddGoogleBox();
    void getLastSessionSettings();

    CalendarHandler *calendarHandler;
    GoogleWrapper *googleWrapper;
    int eventCount = 0;
    int googleChainCounter = 0;

    QGroupBox *getCalendarBox;
    QLineEdit *groupCodeEdit;
    QProgressBar *insertEventProgressBar;
    QPushButton *getCalendarButton;
    QByteArrayList eventData;

    QGroupBox *modCalendarBox;
    QStandardItemModel *insertCourseModel;
    QTreeView *insertCourseView;
    QPushButton *changeColumnButton;
    QPushButton *updateEventButton;
    QCheckBox *statusCheckBox;

    QGroupBox *eventBox;
    QStandardItemModel *eventModel;
    QTreeView *eventView;

    QGroupBox *googleBox;
    QButtonGroup *choosingCalendarButtonGroup;
    QRadioButton *primaryCalendarRadio;
    QRadioButton *secondaryCalendarRadio;
    QLineEdit *secondaryCalendarEdit;
    QCheckBox *clearEventsCheckBox;
    QPushButton *googleStartButton;
};

#endif // MAINWINDOW_H
