#include "mainwindow.h"
#include "calendarhandler.h"
#include "googlewrapper.h"
#include <QSizePolicy>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    qDebug() << QSslSocket::sslLibraryBuildVersionString();

    QCoreApplication::setOrganizationName("nomi2208");
    QCoreApplication::setOrganizationDomain("nomi2208.com");
    QCoreApplication::setApplicationName("CalendarMod");

    makeGetCalendarBox();
    makeModCalendarBox();
    makeEventLog();
    makeAddGoogleBox();

    QWidget *centralWidget = new QWidget;
    QGridLayout *layout = new QGridLayout(centralWidget);
    layout->addWidget(getCalendarBox,   0, 0);
    layout->addWidget(modCalendarBox,   1, 0);
    layout->addWidget(googleBox,        2, 0);
    layout->addWidget(eventBox,         0, 1, -1, 1);
    layout->setColumnMinimumWidth(0, 800);
    layout->setColumnMinimumWidth(1, 400);
    layout->setRowMinimumHeight(1, 500);
    setCentralWidget(centralWidget);

    calendarHandler = new CalendarHandler(this);
    googleWrapper = new GoogleWrapper(this);

    connect(calendarHandler, &CalendarHandler::calendarReady, this, &MainWindow::calendarReady);
    connect(calendarHandler, &CalendarHandler::eventLogSignal, this, &MainWindow::updateEventView);
    connect(this, &MainWindow::getCalendar, calendarHandler, &CalendarHandler::getCalendar);

    connect(googleWrapper, &GoogleWrapper::eventLogSignal, this, &MainWindow::updateEventView);
    connect(googleWrapper, &GoogleWrapper::eventInsertedCount, insertEventProgressBar, &QProgressBar::setValue);
    connect(this, &MainWindow::googleStart, googleWrapper, &GoogleWrapper::googleStart);

    connect(getCalendarButton, &QPushButton::clicked, this, &MainWindow::getCalendarButtonClicked);
    connect(changeColumnButton, &QPushButton::clicked, this, &MainWindow::changeColumnButtonClicked);
    connect(updateEventButton, &QPushButton::clicked, this, &MainWindow::updateEventButtonClicked);
    connect(googleStartButton, &QPushButton::clicked, this, &MainWindow::googleStartButtonClicked);

    getLastSessionSettings();
}

MainWindow::~MainWindow()
{

}

void MainWindow::getCalendarButtonClicked()
{
    getCalendarButton->setDisabled(true);
    QSettings settings;
    settings.setValue("groupCode", groupCodeEdit->text());
    emit getCalendar(groupCodeEdit->text());
}

void MainWindow::changeColumnButtonClicked()
{
    QModelIndex selectedIndex = insertCourseView->selectionModel()->currentIndex();
    if(selectedIndex.isValid())
    {
        int selectedRowIndex = selectedIndex.row();
        QString col_0_text = insertCourseModel->item(selectedRowIndex, 0)->text();
        QString col_1_text = insertCourseModel->item(selectedRowIndex, 1)->text();
        insertCourseModel->item(selectedRowIndex, 0)->setText(col_1_text);
        insertCourseModel->item(selectedRowIndex, 1)->setText(col_0_text);
    }
}

void MainWindow::updateEventButtonClicked()
{
    statusCheckBox->setChecked(false);
    updateEventView("Update events");

    QStringList omitList;
    for(int i=0; i<insertCourseModel->rowCount(); i++)
    {
        QString col_0_text = insertCourseModel->item(i, 0)->text();
        if(col_0_text.isEmpty())
            continue;
        if(!omitList.contains(col_0_text))
            omitList.append(col_0_text);
    }

    QSettings settings;
    settings.remove("omitCourses");
    settings.beginWriteArray("omitCourses");
    for(int i=0; i<omitList.length(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("course", omitList.at(i));
    }
    settings.endArray();

    calendarHandler->createEventsData(omitList);
}

void MainWindow::googleStartButtonClicked()
{
    if(eventCount == 0)
    {
        updateEventView("No event to start");
        updateEventView("No event to start", true);
        return;
    }
    if(secondaryCalendarRadio->isChecked())
    {
        QString secondaryName = secondaryCalendarEdit->text();
        secondaryName = secondaryName.simplified();
        secondaryName.replace(" ", "");
        if(secondaryName.isEmpty())
        {
            updateEventView("Calendar name missing");
            updateEventView("Calendar name missing", true);
            return;
        }
    }

    googleStartButton->setDisabled(true);
    primaryCalendarRadio->setDisabled(true);
    secondaryCalendarRadio->setDisabled(true);
    secondaryCalendarEdit->setDisabled(true);
    clearEventsCheckBox->setDisabled(true);

    QSettings settings;
    settings.setValue("primary", primaryCalendarRadio->isChecked());
    settings.setValue("clear", clearEventsCheckBox->isChecked());
    settings.setValue("secondaryName", secondaryCalendarEdit->text().simplified());

    insertEventProgressBar->setMaximum(eventCount);
    emit googleStart(primaryCalendarRadio->isChecked(),
                     clearEventsCheckBox->isChecked(),
                     eventData,
                     secondaryCalendarEdit->text().simplified());
}

void MainWindow::calendarReady(const QStringList &courseNameList, const QByteArrayList &eventData)
{
    updateEventView("Update events", true);
    updateEventButton->setEnabled(true);
    statusCheckBox->setChecked(true);
    this->eventData = eventData;
    this->eventCount = eventData.length();
    statusCheckBox->setText(QString("%1 events").arg(eventCount));

    if(insertCourseModel->rowCount() == 0)
    {
        QStringList omitList;

        QSettings settings;
        int size = settings.beginReadArray("omitCourses");
        for(int i=0; i<size; i++)
        {
            settings.setArrayIndex(i);
            omitList.append(settings.value("course").toString());
        }
        settings.endArray();

        for(int i=0; i<courseNameList.length(); i++)
            if(omitList.contains(courseNameList.at(i)))
                insertCourseModel->appendRow({new QStandardItem(courseNameList.at(i)), new QStandardItem(QString())});
            else
                insertCourseModel->appendRow({new QStandardItem(QString()), new QStandardItem(courseNameList.at(i))});

        if(!omitList.isEmpty())
            updateEventButton->click();
    }
}

void MainWindow::updateEventView(const QString &message, const bool &done)
{
    qDebug() << message;
    if(!done)
        eventModel->appendRow({new QStandardItem(message), new QStandardItem("In progress")});
    else
        eventModel->item(eventModel->rowCount()-1, 1)->setText("Done");
}

void MainWindow::makeGetCalendarBox()
{
    getCalendarBox = new QGroupBox("Get calendar", this);

    QLabel *groupCodeLabel = new QLabel("Group code");
    groupCodeEdit = new QLineEdit;

    getCalendarButton = new QPushButton("Get");

    QGridLayout *layout = new QGridLayout(getCalendarBox);
    layout->addWidget(groupCodeLabel,   0, 0);
    layout->addWidget(groupCodeEdit,    0, 1);
    layout->addWidget(getCalendarButton,      0, 2);
}

void MainWindow::makeModCalendarBox()
{
    modCalendarBox = new QGroupBox("Choose courses", this);

    insertCourseModel = new QStandardItemModel;
    insertCourseModel->setHorizontalHeaderLabels({"Omit", "Include"});

    insertCourseView = new QTreeView;
    insertCourseView->setModel(insertCourseModel);
    insertCourseView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    insertCourseView->header()->setSectionResizeMode(QHeaderView::Stretch);

    updateEventButton = new QPushButton("Update");
    updateEventButton->setEnabled(false);
    changeColumnButton = new QPushButton("Change column");

    statusCheckBox = new QCheckBox("0 events");
    statusCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents);

    QGridLayout *horizontalLayout = new QGridLayout;
    horizontalLayout->addWidget(changeColumnButton, 0, 0);
    horizontalLayout->addWidget(updateEventButton,  0, 1);
    QGridLayout *layout = new QGridLayout(modCalendarBox);
    layout->addWidget(statusCheckBox,           0, 0, Qt::AlignRight);
    layout->addWidget(insertCourseView,         1, 0);
    layout->addLayout(horizontalLayout,         2, 0);
}

void MainWindow::makeEventLog()
{
    eventBox = new QGroupBox("Event log", this);

    eventModel = new QStandardItemModel;
    eventModel->setHorizontalHeaderLabels({"Event", "State"});

    eventView = new QTreeView;
    eventView->setModel(eventModel);
    eventView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    eventView->header()->setSectionResizeMode(QHeaderView::Stretch);

    QGridLayout *layout = new QGridLayout(eventBox);
    layout->addWidget(eventView, 0, 0);
}

void MainWindow::makeAddGoogleBox()
{
    googleBox = new QGroupBox("Google upload", this);

    QLabel *radioLabel = new QLabel("Choose a calendar to insert events");

    insertEventProgressBar = new QProgressBar;
    insertEventProgressBar->setMinimum(0);
    insertEventProgressBar->setTextVisible(true);
    insertEventProgressBar->setValue(0);

    primaryCalendarRadio = new QRadioButton("Primary (your current main calendar)");
    primaryCalendarRadio->setChecked(true);
    secondaryCalendarRadio = new QRadioButton("Secondary (one will be made if not exist)");
    choosingCalendarButtonGroup = new QButtonGroup(this);
    choosingCalendarButtonGroup->addButton(primaryCalendarRadio, 0);
    choosingCalendarButtonGroup->addButton(secondaryCalendarRadio, 1);

    QLabel *nameLabel = new QLabel(" - Name: ");
    secondaryCalendarEdit = new QLineEdit;

    clearEventsCheckBox = new QCheckBox("Clear all events before inserting");
    clearEventsCheckBox->setChecked(true);

    QLabel *buttonLabel = new QLabel("A popup for signing in will be displayed");
    googleStartButton = new QPushButton("Start");

    QSpacerItem *spaceItem_1 = new QSpacerItem(20, 1);
    QSpacerItem *spaceItem_2 = new QSpacerItem(20, 1);
    QGridLayout *horizontalLayout_1 = new QGridLayout;
    horizontalLayout_1->addWidget(buttonLabel,          0, 0);
    horizontalLayout_1->addWidget(googleStartButton,    0, 1);
    QGridLayout *horizontalLayout_2 = new QGridLayout;
    horizontalLayout_2->addWidget(secondaryCalendarRadio,   0, 0);
    horizontalLayout_2->addWidget(nameLabel,                0, 1);
    horizontalLayout_2->addWidget(secondaryCalendarEdit,    0, 2);
    QGridLayout *horizontalLayout_3 = new QGridLayout;
    horizontalLayout_3->addWidget(radioLabel,               0, 0);
    horizontalLayout_3->addWidget(insertEventProgressBar,   0, 1, Qt::AlignRight);
    QGridLayout *layout = new QGridLayout(googleBox);
    layout->setVerticalSpacing(15);
    layout->addLayout(horizontalLayout_3,       0, 0, 1, 2);
    layout->addItem(spaceItem_1,                1, 0);
    layout->addWidget(primaryCalendarRadio,     1, 1);
    layout->addItem(spaceItem_2,                2, 0);
    layout->addLayout(horizontalLayout_2,       2, 1);
    layout->addWidget(clearEventsCheckBox,      3, 0, 1, 2);
    layout->addLayout(horizontalLayout_1,       4, 0, 1, 2);
}

void MainWindow::getLastSessionSettings()
{
    QSettings settings;
    groupCodeEdit->setText(settings.value("groupCode", QString()).toString());
    if(settings.value("primary", true).toBool())
        primaryCalendarRadio->setChecked(true);
    else
        secondaryCalendarRadio->setChecked(true);
    clearEventsCheckBox->setChecked(settings.value("clear", true).toBool());
    secondaryCalendarEdit->setText(settings.value("secondaryName", QString()).toString());
}
