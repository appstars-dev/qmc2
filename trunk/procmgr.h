#ifndef __PROCMGR_H__
#define __PROCMGR_H__

#include <QProcess>
#include <QMap>
#include <QString>
#include <QStringList>
#include "macros.h"

class ProcessManager : public QObject
{
  Q_OBJECT

  public:
    QMap<QProcess *, ushort> procMap;
    ushort procCount;
    QString lastCommand;
#if QMC2_USE_PHONON_API
    bool musicWasPlaying;
    bool sentPlaySignal;
#endif

    ProcessManager(QWidget *parent = 0);
    ~ProcessManager();

    int start(QString &, QStringList &, bool autoConnect = TRUE);
    QProcess *process(ushort);
    QString readStandardOutput(QProcess *);
    QString readStandardOutput(ushort);
    QString readStandardError(QProcess *);
    QString readStandardError(ushort);
    void terminate(QProcess *);
    void terminate(ushort);
    void kill(QProcess *);
    void kill(ushort);

  public slots:
    void started();
    void finished(int, QProcess::ExitStatus);
    void readyReadStandardOutput();
    void readyReadStandardError();
    void error(QProcess::ProcessError);
    void stateChanged(QProcess::ProcessState);
};

#endif
