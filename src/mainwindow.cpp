#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //windows properties
    this->setWindowTitle("keyboard input bot");
    this->setMinimumSize(650, 250);

    //initializes the UI
    initializeUI();

    //sets chill timer
    timer_chill = new QTimer();
    timer_endChill = new QTimer();
    timer_chillCounter = new QTimer();
    QRandomGenerator randgen(QTime::currentTime().msec());

    chillTimeCounter = randgen.bounded(chillTimeTriggerLower, chillTimeTriggerUpper);
    timer_chill->setInterval(chillTimeCounter);
    timer_chill->setSingleShot(true);
    connect(timer_chill, SIGNAL(timeout()), this, SLOT(chillOut()));

    connect(timer_endChill, SIGNAL(timeout()), this, SLOT(endChill()));

    timer_chillCounter->setInterval(1000);
    connect(timer_chillCounter, SIGNAL(timeout()), this, SLOT(countUntilChill()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeUI()
{
    QWidget *dummyCentralWidget = new QWidget();
    dummyCentralWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setCentralWidget(dummyCentralWidget);

    //shortcut and status
    QGridLayout *grid_shortcut_status = new QGridLayout();

    grpBx_shortcut = new QGroupBox("Start/Stop Shortcut");
    QGridLayout *grid_shortcut = new QGridLayout();
    chkBx_autoReturn = new QCheckBox("Auto insert 'Return' at end of command");
    chkBx_autoReturn->setChecked(true);
    ksq_shortcut = new QKeySequenceEdit(shortcut);
    ksq_shortcut->setFocusPolicy(Qt::ClickFocus);
    connect(ksq_shortcut, SIGNAL(editingFinished()), this, SLOT(redefineShortcut()));
    grid_shortcut->addWidget(ksq_shortcut, 0, 0, Qt::AlignLeft | Qt::AlignTop);
    grid_shortcut->addWidget(chkBx_autoReturn, 0, 2, Qt::AlignLeft | Qt::AlignTop);
    grpBx_shortcut->setLayout(grid_shortcut);

    QGroupBox *grpBx_status = new QGroupBox("Status");
    QGridLayout *grid_status = new QGridLayout();
    status = new QLabel(STOP_MESSAGE);
    lbl_chillTimeLeft = new QLabel("Time until next chill: 0s");
    grid_status->addWidget(status, 0, 0, Qt::AlignLeft | Qt::AlignTop);
    grid_status->addWidget(lbl_chillTimeLeft, 1, 0, Qt::AlignLeft | Qt::AlignTop);
    grpBx_status->setLayout(grid_status);

    grid_shortcut_status->addWidget(grpBx_shortcut, 0, 0, Qt::AlignLeft | Qt::AlignTop);
    grid_shortcut_status->addWidget(grpBx_status, 0, 1, Qt::AlignLeft | Qt::AlignTop);

    //initializes sequences table
    stdModel_sequences = new QStandardItemModel(1, 4);
    QStringList tblVw_sequences_header;
    tblVw_sequences_header.append("Alias");
    tblVw_sequences_header.append("Command");
    tblVw_sequences_header.append("Bottom Timer (in ms)");
    tblVw_sequences_header.append("Upper Timer (in ms)");
    stdModel_sequences->setHorizontalHeaderLabels(tblVw_sequences_header);
    stdModel_sequences->setData(stdModel_sequences->index(0, 0), "Introduction");
    stdModel_sequences->setData(stdModel_sequences->index(0, 1), "Hello, World!");
    stdModel_sequences->setData(stdModel_sequences->index(0, 2), 500);
    stdModel_sequences->setData(stdModel_sequences->index(0, 3), 1000);
    tblVw_sequences = new QTableView();
    tblVw_sequences->setSelectionBehavior(QAbstractItemView::SelectRows);
    tblVw_sequences->setSelectionMode(QAbstractItemView::SingleSelection);
    tblVw_sequences->setModel(stdModel_sequences);
    tblVw_sequences->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    SpinBoxDelegate *spnBxDelegate_bottomTimer = new SpinBoxDelegate();
    SpinBoxDelegate *spnBxDelegate_upperTimer = new SpinBoxDelegate();
    tblVw_sequences->setItemDelegateForColumn(2, spnBxDelegate_bottomTimer);
    tblVw_sequences->setItemDelegateForColumn(3, spnBxDelegate_upperTimer);

    //sequences buttons
    QPushButton *pshBtn_addCommand = new QPushButton();
    pshBtn_addCommand->setText("Add Row");
    connect(pshBtn_addCommand, SIGNAL(clicked()), this, SLOT(addSequenceRow()));
    QPushButton *pshBtn_removeCommand = new QPushButton();
    pshBtn_removeCommand->setText("Remove Row");
    connect(pshBtn_removeCommand, SIGNAL(clicked()), this, SLOT(removeSequenceRow()));
    QGridLayout *grid_addRemoveCommand = new QGridLayout();
    grid_addRemoveCommand->addWidget(pshBtn_addCommand, 0, 0, Qt::AlignLeft | Qt::AlignTop);
    grid_addRemoveCommand->addWidget(pshBtn_removeCommand, 0, 1, Qt::AlignLeft | Qt::AlignTop);

    //sequences box
    grpBx_sequences = new QGroupBox("Sequences");
    QGridLayout *grid_sequences = new QGridLayout();
    grid_sequences->addWidget(tblVw_sequences, 0, 0);
    grid_sequences->addLayout(grid_addRemoveCommand, 1, 0, Qt::AlignRight | Qt::AlignTop);
    grpBx_sequences->setLayout(grid_sequences);

    //inserting everything in the screen
    QGridLayout *mainGrid = new QGridLayout();
    mainGrid->addLayout(grid_shortcut_status, 0, 0, Qt::AlignLeft | Qt::AlignTop);
    mainGrid->addWidget(grpBx_sequences, 1, 0);
    dummyCentralWidget->setLayout(mainGrid);

    //resizes window to its minimum
    adjustSize();

    //creates menu bar
    QMenuBar *menu = new QMenuBar();
    act_save = new QAction("Save Script");
    connect(act_save, SIGNAL(triggered()), this, SLOT(saveScript()));
    act_load = new QAction("Load Script");
    connect(act_load, SIGNAL(triggered()), this, SLOT(loadScript()));

    menu->addAction(act_save);
    menu->addAction(act_load);
    this->setMenuBar(menu);
}

void MainWindow::redefineShortcut()
{
    shortcut = ksq_shortcut->keySequence();
    ksq_shortcut->clearFocus();
}

void MainWindow::addSequenceRow()
{
    int currentRow = stdModel_sequences->rowCount();
    stdModel_sequences->insertRow(currentRow);
    stdModel_sequences->setData(stdModel_sequences->index(currentRow, 0), "NewName");
    stdModel_sequences->setData(stdModel_sequences->index(currentRow, 1), "NewCommand");
    stdModel_sequences->setData(stdModel_sequences->index(currentRow, 2), 500);
    stdModel_sequences->setData(stdModel_sequences->index(currentRow, 3), 1000);
}

void MainWindow::addSequenceRow(QString alias, QString cmd, int btm, int upr)
{
    int currentRow = stdModel_sequences->rowCount();
    stdModel_sequences->insertRow(currentRow);
    stdModel_sequences->setData(stdModel_sequences->index(currentRow, 0), alias);
    stdModel_sequences->setData(stdModel_sequences->index(currentRow, 1), cmd);
    stdModel_sequences->setData(stdModel_sequences->index(currentRow, 2), btm);
    stdModel_sequences->setData(stdModel_sequences->index(currentRow, 3), upr);
}

void MainWindow::removeSequenceRow()
{
    QModelIndexList selectedRows = tblVw_sequences->selectionModel()->selectedRows();
    int rowsAmount = selectedRows.size();
    for(int i = 0; i < rowsAmount; ++i)
        stdModel_sequences->removeRow(selectedRows.at(i).row());
}

void MainWindow::keyPressEvent(QKeyEvent *ke)
{   
    QString modifier;
    QString key;

    if (ke->modifiers() & Qt::ShiftModifier)
        modifier += "Shift+";
    if (ke->modifiers() & Qt::ControlModifier)
        modifier += "Ctrl+";
    if (ke->modifiers() & Qt::AltModifier)
        modifier += "Alt+";
    if (ke->modifiers() & Qt::MetaModifier)
        modifier += "Meta+";

    key = QKeySequence(ke->key()).toString();

    QKeySequence ks(modifier + key);

    if(ks == shortcut)
        shortcutPressed();
}

void MainWindow::shortcutPressed()
{
    shortcutWasPressed = !shortcutWasPressed;
    grpBx_shortcut->setDisabled(shortcutWasPressed);
    grpBx_sequences->setDisabled(shortcutWasPressed);
    act_save->setDisabled(shortcutWasPressed);
    act_load->setDisabled(shortcutWasPressed);
    if(shortcutWasPressed)
    {
        //clears list to avoid duplicates
        for(int i = 0; i < lst_timers.size(); ++i)
        {
            QTimer *timer = lst_timers.at(i);
            timer->stop();
        }
        lst_timers.clear();

        //check if rows are ok
        int rows = stdModel_sequences->rowCount();
        for(int i = 0; i < rows; ++i)
        {
            int lower = stdModel_sequences->data(stdModel_sequences->index(i, 2)).toInt();
            int upper = stdModel_sequences->data(stdModel_sequences->index(i, 3)).toInt();
            if(upper <= lower)
            {
                QString title = "Check your trigger time!";
                QString message = "the upper trigger time for the row number " + QString::number(i+1) +
                        " is lower or equal than the lower trigger, it must be higher";
                QMessageBox::warning(this, title, message);

                shortcutWasPressed = false;
                return;
            }
        }
        //create timers
        for(int i = 0;i < rows; ++i)
        {
            int lower = stdModel_sequences->data(stdModel_sequences->index(i, 2)).toInt();
            int upper = stdModel_sequences->data(stdModel_sequences->index(i, 3)).toInt();
            QRandomGenerator randgen(uint(QTime::currentTime().msec()));
            int triggerTime = randgen.bounded(lower, upper);
            QTimer *timer = new QTimer();
            timer->setInterval(triggerTime);
            timer->setSingleShot(true);
            connect(timer, SIGNAL(timeout()), this, SLOT(callTimer()));
            timer->start();
            lst_timers.append(timer);
        }
        status->setText(START_MESSAGE);
        timer_chill->start();
        timer_chillCounter->start();
        //minimizes the application
        setWindowState(Qt::WindowMinimized);
    }
    else
    {
        int rows = stdModel_sequences->rowCount();
        for(int i = 0;i < rows; ++i)
        {
            QTimer *timer = lst_timers.at(i);
            timer->stop();
        }
        status->setText(STOP_MESSAGE);
        timer_chillCounter->stop();
        timer_chill->stop();
        timer_endChill->stop();
    }
}

void MainWindow::callTimer()
{
    //gets the timer that sent the signal
    QTimer *timer = static_cast<QTimer*>(QObject::sender());
    int timerId = 0;
    int lstSize = lst_timers.size();
    for(int i = 0; i < lstSize; ++i)
    {
        if(timer == lst_timers.at(i))
        {
            timerId = i;
            break;
        }
    }
    QString sequence = stdModel_sequences->data(stdModel_sequences->index(timerId, 1)).toString();

    //sends command
    sendKeysqt(sequence);

    //recalculates trigger time
    int lower = stdModel_sequences->data(stdModel_sequences->index(timerId, 2)).toInt();
    int upper = stdModel_sequences->data(stdModel_sequences->index(timerId, 3)).toInt();
    QRandomGenerator randgen(uint(QTime::currentTime().msec()));
    int triggerTime = randgen.bounded(lower, upper);
    timer->setInterval(triggerTime);
    timer->start();
}

void MainWindow::sendKeysqt(QString bCodeSequence)
{
    // This structure will be used to create the keyboard
    // input event.
    int arraySizeqtt = bCodeSequence.length();
    INPUT ip;

    for (int i=0; i < arraySizeqtt; i++)
    {
        // Set up a generic keyboard event.
        ip.type = INPUT_KEYBOARD;
        ip.ki.time = 0;
        ip.ki.dwExtraInfo = 0;

        // Press the key
        ip.ki.dwFlags = KEYEVENTF_UNICODE;
        ip.ki.wScan = bCodeSequence.at(i).unicode();
        ip.ki.wVk = 0;
        SendInput(1, &ip, sizeof(INPUT));

        // Release the key
        ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
        SendInput(1, &ip, sizeof(INPUT));
    }

    if(chkBx_autoReturn)
    {
        // Press the key
        ip.ki.dwFlags = 0;
        ip.ki.wScan = 0;
        ip.ki.wVk = VK_RETURN;
        SendInput(1, &ip, sizeof(INPUT));

        // Release the key
        ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
        SendInput(1, &ip, sizeof(INPUT));
    }
}

void MainWindow::saveScript()
{
    //gets save path and file
    QString filename = QFileDialog::getSaveFileName(this, "", "", "*.xml");

    if(filename == "")
            return;

    //open file and sets it
    QFile f(filename);
    f.open(QIODevice::WriteOnly);
    QXmlStreamWriter xmlWriter;
    xmlWriter.setDevice(&f);
    xmlWriter.setAutoFormatting(true);
    //writes header <xmlversion="1.0" ...
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement("SaveDocument"); //starts SaveDocument

    xmlWriter.writeStartElement("Configs"); // starts Configs
    xmlWriter.writeAttribute("Shortcut", ksq_shortcut->keySequence().toString());
    xmlWriter.writeAttribute("AutoReturn", QString::number(chkBx_autoReturn->isChecked()));
    xmlWriter.writeAttribute("TableSize", QString::number(stdModel_sequences->rowCount()));
    xmlWriter.writeEndElement(); //closes Configs

    xmlWriter.writeStartElement("TableContent"); //starts TableContent
    int tableSize = stdModel_sequences->rowCount();
    for(int i = 0; i < tableSize; ++i)
    {
        QString rowTag = "row" + QString::number(i);
        QString alias = stdModel_sequences->data(stdModel_sequences->index(i, 0)).toString();
        QString cmd = stdModel_sequences->data(stdModel_sequences->index(i, 1)).toString();
        QString bottom = stdModel_sequences->data(stdModel_sequences->index(i, 2)).toString();
        QString upper = stdModel_sequences->data(stdModel_sequences->index(i, 3)).toString();
        xmlWriter.writeStartElement(rowTag);
        xmlWriter.writeAttribute("alias", alias);
        xmlWriter.writeAttribute("cmd", cmd);
        xmlWriter.writeAttribute("bottom", bottom);
        xmlWriter.writeAttribute("upper", upper);
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement(); //closes TableContent

    xmlWriter.writeEndElement(); //closes SaveDocument

    //closes the document
    f.close();

}

void MainWindow::loadScript()
{
    //gets save path and file
    QString filename = QFileDialog::getOpenFileName(this, "", "", "*.xml");

    if(filename == "")
            return;

    QFile *f = new QFile(filename);
    if(f == Q_NULLPTR)
        return;

    //clears list to avoid duplicates
    for(int i = 0; i < lst_timers.size(); ++i)
    {
        QTimer *timer = lst_timers.at(i);
        timer->stop();
    }
    lst_timers.clear();
    shortcutWasPressed = false;
    grpBx_shortcut->setDisabled(false);
    grpBx_sequences->setDisabled(false);
    act_save->setDisabled(false);
    act_load->setDisabled(false);


    //clears model
    stdModel_sequences->clear();
    QStringList tblVw_sequences_header;
    tblVw_sequences_header.append("Alias");
    tblVw_sequences_header.append("Command");
    tblVw_sequences_header.append("Bottom Timer (in ms)");
    tblVw_sequences_header.append("Upper Timer (in ms)");
    stdModel_sequences->setHorizontalHeaderLabels(tblVw_sequences_header);

    QVariantList shortc = GetXmlAttribute(f, "Configs", "Shortcut");
    QVariantList autoreturn = GetXmlAttribute(f, "Configs", "AutoReturn");
    QVariantList tableSize = GetXmlAttribute(f, "Configs", "TableSize");
    ksq_shortcut->setKeySequence(QKeySequence(shortc.at(0).toString()));
    shortcut = QKeySequence(shortc.at(0).toString());
    chkBx_autoReturn->setChecked(autoreturn.at(0).toBool());
    int rows = tableSize.at(0).toInt();

    for(int i = 0; i < rows; ++i)
    {
        QString row = "row" + QString::number(i);
        QVariantList tableAlias = GetXmlAttribute(f, row, "alias");
        QVariantList tableCmd = GetXmlAttribute(f, row, "cmd");
        QVariantList tableBottom = GetXmlAttribute(f, row, "bottom");
        QVariantList tableUpper = GetXmlAttribute(f, row, "upper");

        addSequenceRow(tableAlias.at(0).toString(), tableCmd.at(0).toString(), tableBottom.at(0).toInt(), tableUpper.at(0).toInt());
    }
}

QVariantList MainWindow::GetXmlAttribute(QFile *file, QString tag, QString att)
{
    //Get the value of an attribute on a tag, returning it as a list of found values
    if(!file->isOpen())
        file->open(QFile::ReadOnly);

    QXmlStreamReader reader(file->readAll());
    QVariantList fndAtt;

    file->close();
    while(!reader.atEnd())
    {
        reader.readNext();
        if (reader.isStartElement())
        {
            if (reader.name() == tag)
            {
                for(int i = 0; i < reader.attributes().size(); ++i)
                {
                    if(reader.attributes().at(i).name().toString() == att)
                    {
                        fndAtt.append(reader.attributes().value(att).toString());
                        break;
                    }
                }
            }
        }
    }

    return fndAtt;
}

void MainWindow::chillOut()
{
    //stops timers
    int rows = stdModel_sequences->rowCount();
    for(int i = 0;i < rows; ++i)
    {
        QTimer *timer = lst_timers.at(i);
        timer->stop();
    }
    status->setText(CHILL_MESSAGE);

    //stop control timers
    timer_chill->stop();

    //gets chill time
    QRandomGenerator randgen(QTime::currentTime().msec());
    chillTimeCounter = randgen.bounded(chillTimeLower, chillTimeUpper);
    timer_endChill->setInterval(chillTimeCounter);
    timer_endChill->setSingleShot(true);
    timer_endChill->start();
    disconnect(timer_chillCounter, SIGNAL(timeout()), this, SLOT(countUntilChill()));
    connect(timer_chillCounter, SIGNAL(timeout()), this, SLOT(countChillTime()));
    timer_chillCounter->start();
}

void MainWindow::endChill()
{
    //stops timers
    int rows = stdModel_sequences->rowCount();
    for(int i = 0;i < rows; ++i)
    {
        //recalculates trigger time
        int lower = stdModel_sequences->data(stdModel_sequences->index(i, 2)).toInt();
        int upper = stdModel_sequences->data(stdModel_sequences->index(i, 3)).toInt();
        QRandomGenerator randgen(uint(QTime::currentTime().msec()));
        int triggerTime = randgen.bounded(lower, upper);
        QTimer *timer = lst_timers.at(i);
        timer->setInterval(triggerTime);
        timer->start();
    }
    status->setText(START_MESSAGE);

    //stops control timers
    timer_endChill->stop();

    //resume chill timer
    QRandomGenerator randgen(QTime::currentTime().msec());
    chillTimeCounter = randgen.bounded(chillTimeTriggerLower, chillTimeTriggerUpper);
    timer_chill->setInterval(chillTimeCounter);
    timer_chill->start();
    disconnect(timer_chillCounter, SIGNAL(timeout()), this, SLOT(countChillTime()));
    connect(timer_chillCounter, SIGNAL(timeout()), this, SLOT(countUntilChill()));
    timer_chillCounter->start();

}

void MainWindow::countChillTime()
{
    chillTimeCounter -= 1000;
    if(chillTimeCounter < 0)
        chillTimeCounter = 0;
    lbl_chillTimeLeft->setText("Chill time left: " + QString::number(chillTimeCounter/1000) + "s");
}

void MainWindow::countUntilChill()
{
    chillTimeCounter -= 1000;
    if(chillTimeCounter < 0)
        chillTimeCounter = 0;
    lbl_chillTimeLeft->setText("Time until next chill: " + QString::number(chillTimeCounter/1000) + "s");
}

