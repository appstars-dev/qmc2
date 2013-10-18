#ifndef SCRIPTWIDGET_H
#define SCRIPTWIDGET_H

#include <QtGui>

#include "projectwidget.h"
#include "scriptengine.h"

class ScriptEngine;

namespace Ui {
class ScriptWidget;
}

class ScriptWidget : public QWidget
{
    Q_OBJECT

    friend class ScriptEngine;

public:
    bool askFileName;

    explicit ScriptWidget(QWidget *parent = 0);
    virtual ~ScriptWidget();

    ScriptEngine *engine() { return scriptEngine; }

public slots:
    // Callbacks
    void on_toolButtonRun_clicked();
    void on_toolButtonStop_clicked();
    void on_progressBar_valueChanged(int);

    // Other
    void adjustFonts();
    void log(QString);
    void load(const QString &fileName = QString(), QString *buffer = NULL);
    void save(QString *buffer = NULL);
    void saveAs(const QString &fileName = QString(), QString *buffer = NULL);
    QString toString();
    void fromString(QString);
    void triggerSaveAs();
    void resetProgressBar();

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::ScriptWidget *ui;
    ScriptEngine *scriptEngine;
    QLinearGradient linearGradient;
};

#endif // SCRIPTWIDGET_H
