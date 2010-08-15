#ifndef _DEMOMODE_H_
#define _DEMOMODE_H_

#include <QProcess>
#include "ui_demomode.h"

class DemoModeDialog : public QDialog, public Ui::DemoModeDialog
{
  Q_OBJECT

  public:
    QStringList selectedGames;
    QProcess *emuProcess;
    bool demoModeRunning;

    DemoModeDialog(QWidget *parent = 0);
    ~DemoModeDialog();

  public slots:
    void adjustIconSizes();
    void on_pushButtonRunDemo_clicked();
    void emuFinished(int, QProcess::ExitStatus);
    void emuStarted();
    void startNextEmu();
    void setStatus(QString);
    void clearStatus() { setStatus(QString()); }

  protected:
    void closeEvent(QCloseEvent *);
    void hideEvent(QHideEvent *) { closeEvent(NULL); };
    void showEvent(QShowEvent *);
};

#endif
