#include "keyseqscan.h"
#include "macros.h"
#include "options.h"
#include "qmc2main.h"

// external global variables
extern MainWindow *qmc2MainWindow;

KeySequenceScanner::KeySequenceScanner(QWidget *parent, bool special)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: KeySequenceScanner::KeySequenceScanner(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
#endif

  setupUi(this);

  specialKey = special;
  if ( specialKey ) {
    labelStatus->setText(tr("Scanning special key"));
    setWindowTitle(tr("Scanning special key"));
  } else {
    labelStatus->setText(tr("Scanning shortcut"));
    setWindowTitle(tr("Scanning shortcut"));
  }
  keySequence = 0;
  animSeq = 0;
  animationTimeout();
  connect(&animTimer, SIGNAL(timeout()), this, SLOT(animationTimeout()));
  animTimer.start(QMC2_ANIMATION_TIMEOUT);
  grabKeyboard();
}

KeySequenceScanner::~KeySequenceScanner()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: KeySequenceScanner::~KeySequenceScanner()");
#endif

  releaseKeyboard();
}

void KeySequenceScanner::animationTimeout()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: KeySequenceScanner::animationTimeout()");
#endif

  switch ( animSeq ) {
    case 0:
      labelKeySequence->setText("<   >");
      break;
    case 1:
      labelKeySequence->setText("< <   > >");
      break;
    case 2:
      labelKeySequence->setText("< < <   > > >");
      break;
    case 3:
      labelKeySequence->setText("< < < <   > > > >");
      break;
    case 4:
      labelKeySequence->setText("< < < < <   > > > > >");
      break;
    case 5:
      labelKeySequence->setText("> > > > >   < < < < <");
      break;
    case 6:
      labelKeySequence->setText("> > > >   < < < <");
      break;
    case 7:
      labelKeySequence->setText("> > >   < < <");
      break;
    case 8:
      labelKeySequence->setText("> >   < <");
      break;
    case 9:
      labelKeySequence->setText(">   <");
      break;
  }
  animSeq++;
  if ( animSeq > 9 )
    animSeq = 0;
}

void KeySequenceScanner::keyPressEvent(QKeyEvent *event)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: KeySequenceScanner::keyPressEvent(QKeyEvent *event = %1)").arg((qulonglong)event));
#endif

  keySequence = event->key();
  seqModifiers = event->modifiers();
  if ( keySequence != 0 && keySequence != Qt::Key_unknown ) {
    animTimer.stop();
    keySequence += seqModifiers;

    QString keySeqString(QKeySequence(keySequence).toString().toAscii());
    currentKeySequence = keySeqString;
    QStringList words = keySeqString.split("+");
    keySeqString = "";
    int i;
    for (i = 0; i < words.count(); i++) {
      if ( i > 0 ) keySeqString += "+";
      keySeqString += QObject::tr(words[i].toAscii());
    }
    labelKeySequence->setText(keySeqString);

    if ( specialKey ) {
      if ( (words.count() != 1 && currentKeySequence != "+") || labelKeySequence->text().endsWith("??") ) {
        pushButtonOk->setEnabled(FALSE);
        animSeq = 0;
        animTimer.start(QMC2_ANIMATION_TIMEOUT);
      } else {
        pushButtonOk->setEnabled(TRUE);
      }
    } else if ( labelKeySequence->text().endsWith("??") ) {
      pushButtonOk->setEnabled(FALSE);
      if ( labelKeySequence->text() == "??" ) {
        animSeq = 0;
        animTimer.start(QMC2_ANIMATION_TIMEOUT);
      }
    } else
      pushButtonOk->setEnabled(TRUE);
  } else {
    pushButtonOk->setEnabled(FALSE);
    animationTimeout();
    animTimer.start(QMC2_ANIMATION_TIMEOUT);
  }

  event->accept();
}

void KeySequenceScanner::keyReleaseEvent(QKeyEvent *event)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: KeySequenceScanner::keyReleaseEvent(QKeyEvent *event = %1)").arg((qulonglong)event));
#endif

  keySequence = event->key();
  if ( seqModifiers != event->modifiers() && labelKeySequence->text().endsWith("??") ) {
    seqModifiers = event->modifiers();
    animTimer.stop();
    keySequence += seqModifiers;

    QString keySeqString(QKeySequence(keySequence).toString().toAscii());
    currentKeySequence = keySeqString;
    QStringList words = keySeqString.split("+");
    keySeqString = "";
    int i;
    for (i = 0; i < words.count(); i++) {
      if ( i > 0 ) keySeqString += "+";
      keySeqString += QObject::tr(words[i].toAscii());
    }
    labelKeySequence->setText(keySeqString);

    if ( specialKey ) {
      if ( (words.count() != 1 && currentKeySequence != "+") || labelKeySequence->text().endsWith("??") ) {
        pushButtonOk->setEnabled(FALSE);
        animSeq = 0;
        animTimer.start(QMC2_ANIMATION_TIMEOUT);
      } else {
        pushButtonOk->setEnabled(TRUE);
      }
    } else if ( labelKeySequence->text().endsWith("??") ) {
      pushButtonOk->setEnabled(FALSE);
      if ( labelKeySequence->text() == "??" ) {
        animSeq = 0;
        animTimer.start(QMC2_ANIMATION_TIMEOUT);
      }
    } else
      pushButtonOk->setEnabled(TRUE);
  }

  event->accept();
}
