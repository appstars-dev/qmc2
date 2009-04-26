#include "toolexec.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;

ToolExecutor::ToolExecutor(QWidget *parent, QString &command, QStringList &args)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolExecutor::ToolExecutor(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ", ...)");
#endif

  setupUi(this);

  toolCommand = command;
  toolArgs = args;
  toolProc = new QProcess(this);
  connect(toolProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(toolError(QProcess::ProcessError)));
  connect(toolProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(toolFinished(int, QProcess::ExitStatus)));
  connect(toolProc, SIGNAL(readyReadStandardOutput()), this, SLOT(toolReadyReadStandardOutput()));
  connect(toolProc, SIGNAL(readyReadStandardError()), this, SLOT(toolReadyReadStandardError()));
  connect(toolProc, SIGNAL(started()), this, SLOT(toolStarted()));
  connect(toolProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(toolStateChanged(QProcess::ProcessState)));
  QString commandString = toolCommand, s;
  foreach (s, toolArgs)
    commandString += " " + s;
  lineEditCommand->setText(commandString);
  QTimer::singleShot(0, this, SLOT(execute()));
}

ToolExecutor::~ToolExecutor()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolExecutor::~ToolExecutor()");
#endif

}

void ToolExecutor::execute()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolExecutor::execute()");
#endif

  toolProc->start(toolCommand, toolArgs);
}

void ToolExecutor::toolStarted()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolExecutor::toolStarted()");
#endif

  textBrowserToolOutput->append(tr("### tool started, output below ###"));
}

void ToolExecutor::toolFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ToolExecutor::toolFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2)").arg(exitCode).arg(exitStatus));
#endif

  textBrowserToolOutput->append(tr("### tool finished, exit code = %1, exit status = %2 ###").arg(exitCode).arg(exitStatus));
  pushButtonOk->setEnabled(TRUE);
}

void ToolExecutor::toolReadyReadStandardOutput()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolExecutor::toolReadyReadStandardOutput()");
#endif

  QString s = toolProc->readAllStandardOutput();
  QStringList sl = s.split("\n");
  foreach (s, sl)
    if ( !s.isEmpty() )
      textBrowserToolOutput->append(s);
}

void ToolExecutor::toolReadyReadStandardError()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolExecutor::toolReadyReadStandardError()");
#endif

  QString s = toolProc->readAllStandardError();
  QStringList sl = s.split("\n");
  foreach (s, sl)
    if ( !s.isEmpty() )
      textBrowserToolOutput->append(s);
}

void ToolExecutor::toolError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ToolExecutor::toolError(QProcess::ProcessError processError = %1)").arg(processError));
#endif

  textBrowserToolOutput->append(tr("### tool error, process error = %1 ###").arg(processError));
}

void ToolExecutor::toolStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ToolExecutor::toolStateChanged(QProcess::ProcessState processState = %1)").arg(processState));
#endif

}
