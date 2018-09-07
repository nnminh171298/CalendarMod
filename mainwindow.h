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
    void authorize();
    void clearPrimaryCalendar();
    void insertEvent(const QByteArray &eventData);
    void revokeToken();

private slots:
    void startButtonClicked();
    void calendarReady();
    void authorized();
    void primaryCalendarCleared();
    void eventInserted();
    void tokenRevoked();

private:
    void makeView();

    CalendarHandler *calendarHandler;
    GoogleWrapper *googleWrapper;

    QGroupBox *groupBox;
    QLineEdit *groupCodeEdit;
    QCheckBox *calendarReadyCheckBox;
    QCheckBox *authorizedCheckBox;
    QCheckBox *clearCalendarCheckBox;
    QCheckBox *insertEventCheckBox;
    QProgressBar *insertEventProgressBar;
    QCheckBox *revokeTokenCheckBox;
    QPushButton *startButton;

    int eventCount;
};

#endif // MAINWINDOW_H
