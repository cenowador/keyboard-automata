#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGroupBox>
#include <QGridLayout>
#include <QKeySequenceEdit>
#include <QTableView>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QMessageBox>

#include <QDebug>
#include <QKeyEvent>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QHeaderView>
#include <QList>
#include <QTimer>
#include <QTime>
#include <QRandomGenerator>
#include <QFile>
#include <QXmlStreamReader>
#include <QFileDialog>
#include "spinboxdelegate.h"

#include <windows.h>

//defines minimum windows version
#define WINVER 0x0500

//messages
#define STOP_MESSAGE "Stopped"
#define START_MESSAGE "Running"
#define CHILL_MESSAGE "Chilling"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    void initializeUI();
    void shortcutPressed();
    void sendKeysqt(QString bCodeSequence);
    QVariantList GetXmlAttribute(QFile *xmlFile, QString tag, QString att);

private:
    bool shortcutWasPressed = false;
    QKeySequence shortcut = Qt::Key_F1;
    QKeySequenceEdit *ksq_shortcut;
    QCheckBox *chkBx_autoReturn;
    QTableView *tblVw_sequences;
    QStandardItemModel *stdModel_sequences;
    QList<QTimer*> lst_timers;
    QLabel *status;
    QLabel *lbl_chillTimeLeft;

private:
    QGroupBox *grpBx_shortcut;
    QGroupBox *grpBx_sequences;
    QAction *act_save;
    QAction *act_load;
    unsigned int chillTimeTriggerLower = 10*60000; //starts between 10 mins and
    unsigned int chillTimeTriggerUpper = 15*60000; //15 mins
    unsigned int chillTimeLower = 18*60000; //chills between 18 mins and
    unsigned int chillTimeUpper = 25*60000; //25 mins
    int chillTimeCounter = 0; //counting how much chilling is left
    QTimer *timer_chill, *timer_endChill, *timer_chillCounter;

private slots:
    void redefineShortcut();
    void addSequenceRow();
    void addSequenceRow(QString alias, QString cmd, int btm, int upr);
    void removeSequenceRow();
    void callTimer();
    void saveScript();
    void loadScript();
    void chillOut();
    void endChill();
    void countChillTime();
    void countUntilChill();

protected:
    void keyPressEvent(QKeyEvent *e);
};
#endif // MAINWINDOW_H
