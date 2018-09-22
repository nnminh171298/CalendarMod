#include "mainwindow.h"
#include "calendarhandler.h"
#include "googlewrapper.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    qDebug() << QSslSocket::sslLibraryBuildVersionString();

    QCoreApplication::setOrganizationName("nomi2208");
    QCoreApplication::setOrganizationDomain("nomi2208.com");
    QCoreApplication::setApplicationName("CalendarMod");

    makeView();

    calendarHandler = new CalendarHandler(this);
    googleWrapper = new GoogleWrapper(this);

    connect(startButton, &QPushButton::clicked,                     this, &MainWindow::startButtonClicked);
    connect(calendarHandler, &CalendarHandler::calendarReady,       this, &MainWindow::calendarReady);
    connect(googleWrapper, &GoogleWrapper::authorized,              this, &MainWindow::authorized);
    connect(googleWrapper, &GoogleWrapper::primaryCalendarCleared,  this, &MainWindow::primaryCalendarCleared);
    connect(googleWrapper, &GoogleWrapper::eventInserted,           this, &MainWindow::eventInserted);
    connect(googleWrapper, &GoogleWrapper::tokenRevoked,            this, &MainWindow::tokenRevoked);

    connect(this, &MainWindow::getCalendar, calendarHandler, &CalendarHandler::getCalendar);
    connect(this, &MainWindow::authorize,               googleWrapper, &GoogleWrapper::grant);
    connect(this, &MainWindow::clearPrimaryCalendar,    googleWrapper, &GoogleWrapper::clearPrimaryCalendar);
    connect(this, &MainWindow::insertEvent,             googleWrapper, &GoogleWrapper::insertEvent);
    connect(this, &MainWindow::revokeToken,             googleWrapper, &GoogleWrapper::revokeToken);
}

MainWindow::~MainWindow()
{

}

void MainWindow::startButtonClicked()
{
    startButton->setDisabled(true);
    QSettings settings;
    settings.setValue("groupCode", groupCodeEdit->text());
    emit getCalendar(groupCodeEdit->text());
}

void MainWindow::calendarReady()
{
    calendarReadyCheckBox->setChecked(true);
    eventCount = calendarHandler->getEventCount();
    insertEventProgressBar->setMaximum(eventCount);
    emit authorize();
}

void MainWindow::authorized()
{
    authorizedCheckBox->setChecked(true);
    emit clearPrimaryCalendar();
}

void MainWindow::primaryCalendarCleared()
{
    clearCalendarCheckBox->setChecked(true);
    for(int i=0; i<eventCount; i++)
        emit insertEvent(calendarHandler->getEventDataAt(i));
}

void MainWindow::eventInserted()
{
    insertEventProgressBar->setValue(insertEventProgressBar->value()+1);
    if(insertEventProgressBar->value() == eventCount)
    {
        insertEventCheckBox->setChecked(true);
        emit revokeToken();
    }
}

void MainWindow::tokenRevoked()
{
    revokeTokenCheckBox->setChecked(true);
}

void MainWindow::makeView()
{
    groupBox = new QGroupBox(this);
    QSettings settings;

    QLabel *groupCodeLabel = new QLabel("Group code");
    groupCodeEdit = new QLineEdit;
    groupCodeEdit->setText(settings.value("groupCode", QString()).toString());
    calendarReadyCheckBox = new QCheckBox("Calendar modified");
    calendarReadyCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents);

    QLabel *authLabel = new QLabel("User Google authorization");
    authorizedCheckBox = new QCheckBox("Authorized");
    authorizedCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents);

    QLabel *clearCalendarLabel = new QLabel("Clear primary calendar");
    clearCalendarCheckBox = new QCheckBox("Cleared");
    clearCalendarCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents);

    QLabel *insertEventLabel = new QLabel("Insert events progress");
    insertEventCheckBox = new QCheckBox("All inserted");
    insertEventCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents);
    insertEventProgressBar = new QProgressBar;
    insertEventProgressBar->setMinimum(0);
    insertEventProgressBar->setTextVisible(true);
    insertEventProgressBar->setValue(0);

    QLabel *revokeTokenLabel = new QLabel("Revoke token");
    revokeTokenCheckBox = new QCheckBox("Revoked");
    revokeTokenCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents);

    startButton = new QPushButton("Start");

    QGridLayout *horizontalLayout_1 = new QGridLayout;
    horizontalLayout_1->addWidget(groupCodeLabel,   0, 0);
    horizontalLayout_1->addWidget(groupCodeEdit,    0, 1);
    QGridLayout *horizontalLayout_2 = new QGridLayout;
    horizontalLayout_2->addWidget(insertEventLabel,         0, 0);
    horizontalLayout_2->addWidget(insertEventProgressBar,   0, 1);

    QGridLayout *verticalLayout_1 = new QGridLayout;
    verticalLayout_1->setSpacing(20);
    verticalLayout_1->addLayout(horizontalLayout_1, 0, 0);
    verticalLayout_1->addWidget(authLabel,          1, 0);
    verticalLayout_1->addWidget(clearCalendarLabel, 2, 0);
    verticalLayout_1->addLayout(horizontalLayout_2, 3, 0);
    verticalLayout_1->addWidget(revokeTokenLabel,   4, 0);
    QGridLayout *verticalLayout_2 = new QGridLayout;
    verticalLayout_2->setSpacing(20);
    verticalLayout_2->addWidget(calendarReadyCheckBox,  0, 0);
    verticalLayout_2->addWidget(authorizedCheckBox,     1, 0);
    verticalLayout_2->addWidget(clearCalendarCheckBox,  2, 0);
    verticalLayout_2->addWidget(insertEventCheckBox,    3, 0);
    verticalLayout_2->addWidget(revokeTokenCheckBox,    4, 0);

    QGridLayout *layout = new QGridLayout(groupBox);
    layout->setVerticalSpacing(20);
    layout->addLayout(verticalLayout_1, 0, 0);
    layout->addLayout(verticalLayout_2, 0, 1);
    layout->addWidget(startButton,      1, 1);
    layout->setColumnMinimumWidth(0, 300);

    setCentralWidget(groupBox);
}
