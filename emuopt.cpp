#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QHeaderView>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QTreeWidgetItem>
#include <QScrollBar>

#include <stdlib.h>
#include "emuopt.h"
#include "options.h"
#include "gamelist.h"
#include "itemselect.h"
#include "qmc2main.h"
#include "fileeditwidget.h"
#include "floateditwidget.h"
#include "direditwidget.h"
#include "comboeditwidget.h"
#include "macros.h"
#if defined(QMC2_EMUTYPE_MAME)
#include "demomode.h"
#endif

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern QSettings *qmc2Config;
extern EmulatorOptions *qmc2GlobalEmulatorOptions;
extern EmulatorOptions *qmc2EmulatorOptions;
extern bool qmc2GuiReady;
extern QTreeWidgetItem *qmc2CurrentItem;
extern Gamelist *qmc2Gamelist;
extern bool qmc2ReloadActive;
#if defined(QMC2_EMUTYPE_MAME)
extern DemoModeDialog *qmc2DemoModeDialog;
#endif

QMap<QString, QList<EmulatorOption> > EmulatorOptions::templateMap;
QMap<QString, bool> EmulatorOptions::optionExpansionMap;
QMap<QString, bool> EmulatorOptions::sectionExpansionMap;
QMap<QString, QTreeWidgetItem *> EmulatorOptions::sectionItemMap;
int EmulatorOptions::horizontalScrollPosition = 0;
int EmulatorOptions::verticalScrollPosition = 0;

QString optionDescription = "";
int optionType = QMC2_EMUOPT_TYPE_UNKNOWN;
int optionDecimals = QMC2_EMUOPT_DFLT_DECIMALS;
QStringList optionChoices;
QString optionPart = "";
EmulatorOptions *emulatorOptions = NULL;

EmulatorOptionDelegate::EmulatorOptionDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptionDelegate::EmulatorOptionDelegate(QObject *parent = %1)").arg((qulonglong)parent));
#endif

}

void EmulatorOptionDelegate::dataChanged()
{
  QWidget *widget = (QWidget *)sender();

  if ( widget ) {
    emit commitData(widget);
    if ( qmc2GlobalEmulatorOptions && parent() == qmc2GlobalEmulatorOptions )
      qmc2GlobalEmulatorOptions->changed = true;
  }
}

QWidget *EmulatorOptionDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  #define _MIN (-2 * 1024 * 1024)
  #define _MAX (2 * 1024 * 1024)

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptionDelegate::createEditor(QWidget *parent = %1, const QStyleOptionViewItem &option, const QModelIndex &index)").arg((qulonglong)parent));
#endif

  switch ( optionType ) {
    case QMC2_EMUOPT_TYPE_BOOL: {
      QCheckBox *checkBoxEditor = new QCheckBox(parent);
      checkBoxEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      checkBoxEditor->setAccessibleName("checkBoxEditor");
      if ( !optionDescription.isEmpty() )
        checkBoxEditor->setToolTip(optionDescription);
      connect(checkBoxEditor, SIGNAL(toggled(bool)), this, SLOT(dataChanged()));
      return checkBoxEditor;
    }

    case QMC2_EMUOPT_TYPE_INT: {
      QSpinBox *spinBoxEditor = new QSpinBox(parent);
      spinBoxEditor->setRange(_MIN, _MAX);
      spinBoxEditor->setSingleStep(1);
      spinBoxEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      spinBoxEditor->setAccessibleName("spinBoxEditor");
      if ( !optionDescription.isEmpty() )
        spinBoxEditor->setToolTip(optionDescription);
      connect(spinBoxEditor, SIGNAL(valueChanged(int)), this, SLOT(dataChanged()));
      return spinBoxEditor;
    }

    case QMC2_EMUOPT_TYPE_FLOAT: {
      QDoubleSpinBox *doubleSpinBoxEditor = new QDoubleSpinBox(parent);
      doubleSpinBoxEditor->setRange(_MIN, _MAX);
      doubleSpinBoxEditor->setSingleStep(0.1);
      doubleSpinBoxEditor->setDecimals(optionDecimals);
      doubleSpinBoxEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      doubleSpinBoxEditor->setAccessibleName("doubleSpinBoxEditor");
      if ( !optionDescription.isEmpty() )
        doubleSpinBoxEditor->setToolTip(optionDescription);
      connect(doubleSpinBoxEditor, SIGNAL(valueChanged(double)), this, SLOT(dataChanged()));
      return doubleSpinBoxEditor;
    }

    case QMC2_EMUOPT_TYPE_FLOAT2: {
      FloatEditWidget *float2Editor = new FloatEditWidget(2, ",", parent);
      float2Editor->doubleSpinBox0->setRange(_MIN, _MAX);
      float2Editor->doubleSpinBox0->setSingleStep(0.1);
      float2Editor->doubleSpinBox0->setDecimals(optionDecimals);
      float2Editor->doubleSpinBox1->setRange(_MIN, _MAX);
      float2Editor->doubleSpinBox1->setSingleStep(0.1);
      float2Editor->doubleSpinBox1->setDecimals(optionDecimals);
      float2Editor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      float2Editor->setAccessibleName("float2Editor");
      if ( !optionDescription.isEmpty() )
        float2Editor->setToolTip(optionDescription);
      connect(float2Editor->doubleSpinBox0, SIGNAL(valueChanged(double)), this, SLOT(dataChanged()));
      connect(float2Editor->doubleSpinBox1, SIGNAL(valueChanged(double)), this, SLOT(dataChanged()));
      return float2Editor;
    }

    case QMC2_EMUOPT_TYPE_FLOAT3: {
      FloatEditWidget *float3Editor = new FloatEditWidget(3, ",", parent);
      float3Editor->doubleSpinBox0->setRange(_MIN, _MAX);
      float3Editor->doubleSpinBox0->setSingleStep(0.1);
      float3Editor->doubleSpinBox0->setDecimals(optionDecimals);
      float3Editor->doubleSpinBox1->setRange(_MIN, _MAX);
      float3Editor->doubleSpinBox1->setSingleStep(0.1);
      float3Editor->doubleSpinBox1->setDecimals(optionDecimals);
      float3Editor->doubleSpinBox2->setRange(_MIN, _MAX);
      float3Editor->doubleSpinBox2->setSingleStep(0.1);
      float3Editor->doubleSpinBox2->setDecimals(optionDecimals);
      float3Editor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      float3Editor->setAccessibleName("float3Editor");
      if ( !optionDescription.isEmpty() )
        float3Editor->setToolTip(optionDescription);
      connect(float3Editor->doubleSpinBox0, SIGNAL(valueChanged(double)), this, SLOT(dataChanged()));
      connect(float3Editor->doubleSpinBox1, SIGNAL(valueChanged(double)), this, SLOT(dataChanged()));
      connect(float3Editor->doubleSpinBox2, SIGNAL(valueChanged(double)), this, SLOT(dataChanged()));
      return float3Editor;
    }

    case QMC2_EMUOPT_TYPE_FILE: {
      FileEditWidget *fileEditor = new FileEditWidget("", tr("All files (*)"), optionPart, parent);
      fileEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      fileEditor->setAccessibleName("fileEditor");
      if ( !optionDescription.isEmpty() ) {
        fileEditor->lineEditFile->setToolTip(optionDescription);
        fileEditor->toolButtonBrowse->setToolTip(tr("Browse: ") + optionDescription);
      }
      connect(fileEditor, SIGNAL(dataChanged(QWidget *)), this, SLOT(dataChanged()));
      return fileEditor;
    }

    case QMC2_EMUOPT_TYPE_DIRECTORY: {
      DirectoryEditWidget *directoryEditor = new DirectoryEditWidget("", parent);
      directoryEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      directoryEditor->setAccessibleName("directoryEditor");
      if ( !optionDescription.isEmpty() ) {
        directoryEditor->lineEditDirectory->setToolTip(optionDescription);
        directoryEditor->toolButtonBrowse->setToolTip(tr("Browse: ") + optionDescription);
      }
      connect(directoryEditor, SIGNAL(dataChanged(QWidget *)), this, SLOT(dataChanged()));
      return directoryEditor;
    }

    case QMC2_EMUOPT_TYPE_COMBO: {
      ComboBoxEditWidget *comboEditor = new ComboBoxEditWidget(optionChoices, "", parent);
      comboEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      comboEditor->setAccessibleName("comboEditor");
      if ( !optionDescription.isEmpty() )
        comboEditor->comboBoxValue->setToolTip(optionDescription);
      connect(comboEditor, SIGNAL(dataChanged(QWidget *)), this, SLOT(dataChanged()));
      return comboEditor;
    }

    case QMC2_EMUOPT_TYPE_STRING:
    default: {
      QLineEdit *lineEditEditor = new QLineEdit(parent);
      lineEditEditor->installEventFilter(const_cast<EmulatorOptionDelegate*>(this));
      lineEditEditor->setAccessibleName("lineEditEditor");
      if ( !optionDescription.isEmpty() )
        lineEditEditor->setToolTip(optionDescription);
      connect(lineEditEditor, SIGNAL(textEdited(const QString &)), this, SLOT(dataChanged()));
      return lineEditEditor;
    }
  }
}

void EmulatorOptionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptionDelegate::setEditorData(QWidget *editor = %1, const QModelIndex &index)").arg((qulonglong)editor));
#endif

  if ( editor->accessibleName() == "checkBoxEditor" ) {
    bool value = index.model()->data(index, Qt::EditRole).toBool();
    QCheckBox *checkBoxEditor = static_cast<QCheckBox *>(editor);
    checkBoxEditor->setChecked(value);
  } else if ( editor->accessibleName() == "spinBoxEditor" ) {
    int value = index.model()->data(index, Qt::EditRole).toInt();
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    int cPos = 0;
    QLineEdit *lineEdit = spinBox->findChild<QLineEdit *>();
    if ( lineEdit ) cPos = lineEdit->cursorPosition();
    spinBox->setValue(value);
    if ( lineEdit ) lineEdit->setCursorPosition(cPos);
  } else if ( editor->accessibleName() == "doubleSpinBoxEditor" ) {
    double value = index.model()->data(index, Qt::EditRole).toDouble();
    QDoubleSpinBox *doubleSpinBox = static_cast<QDoubleSpinBox *>(editor);
    int cPos = 0;
    QLineEdit *lineEdit = doubleSpinBox->findChild<QLineEdit *>();
    if ( lineEdit ) cPos = lineEdit->cursorPosition();
    doubleSpinBox->setValue(value);
    if ( lineEdit ) lineEdit->setCursorPosition(cPos);
  } else if ( editor->accessibleName() == "float2Editor" ) {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    FloatEditWidget *float2Editor = static_cast<FloatEditWidget *>(editor);
    QStringList subValues = value.split(",");
    int cPos = 0;
    if ( subValues.count() > 0 ) {
      QLineEdit *lineEdit = float2Editor->doubleSpinBox0->findChild<QLineEdit *>();
      if ( lineEdit ) cPos = lineEdit->cursorPosition();
      float2Editor->doubleSpinBox0->setValue(subValues[0].toDouble());
      if ( lineEdit ) lineEdit->setCursorPosition(cPos);
    }
    if ( subValues.count() > 1 ) {
      QLineEdit *lineEdit = float2Editor->doubleSpinBox1->findChild<QLineEdit *>();
      if ( lineEdit ) cPos = lineEdit->cursorPosition();
      float2Editor->doubleSpinBox1->setValue(subValues[1].toDouble());
      if ( lineEdit ) lineEdit->setCursorPosition(cPos);
    }
  } else if ( editor->accessibleName() == "float3Editor" ) {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    FloatEditWidget *float3Editor = static_cast<FloatEditWidget *>(editor);
    QStringList subValues = value.split(",");
    int cPos = 0;
    if ( subValues.count() > 0 ) {
      QLineEdit *lineEdit = float3Editor->doubleSpinBox0->findChild<QLineEdit *>();
      if ( lineEdit ) cPos = lineEdit->cursorPosition();
      float3Editor->doubleSpinBox0->setValue(subValues[0].toDouble());
      if ( lineEdit ) lineEdit->setCursorPosition(cPos);
    }
    if ( subValues.count() > 1 ) {
      QLineEdit *lineEdit = float3Editor->doubleSpinBox1->findChild<QLineEdit *>();
      if ( lineEdit ) cPos = lineEdit->cursorPosition();
      float3Editor->doubleSpinBox1->setValue(subValues[1].toDouble());
      if ( lineEdit ) lineEdit->setCursorPosition(cPos);
    }
    if ( subValues.count() > 2 ) {
      QLineEdit *lineEdit = float3Editor->doubleSpinBox2->findChild<QLineEdit *>();
      if ( lineEdit ) cPos = lineEdit->cursorPosition();
      float3Editor->doubleSpinBox2->setValue(subValues[2].toDouble());
      if ( lineEdit ) lineEdit->setCursorPosition(cPos);
    }
  } else if ( editor->accessibleName() == "fileEditor" ) {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    FileEditWidget *fileEditor = static_cast<FileEditWidget *>(editor);
    int cPos = fileEditor->lineEditFile->cursorPosition();
    fileEditor->lineEditFile->setText(value);
    fileEditor->lineEditFile->setCursorPosition(cPos);
  } else if ( editor->accessibleName() == "directoryEditor" ) {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    DirectoryEditWidget *directoryEditor = static_cast<DirectoryEditWidget *>(editor);
    int cPos = directoryEditor->lineEditDirectory->cursorPosition();
    directoryEditor->lineEditDirectory->setText(value);
    directoryEditor->lineEditDirectory->setCursorPosition(cPos);
  } else if ( editor->accessibleName() == "comboEditor" ) {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    ComboBoxEditWidget *comboEditor = static_cast<ComboBoxEditWidget *>(editor);
    int cPos = comboEditor->comboBoxValue->lineEdit()->cursorPosition();
    comboEditor->comboBoxValue->lineEdit()->setText(value);
    comboEditor->comboBoxValue->lineEdit()->setCursorPosition(cPos);
    int itemIndex = comboEditor->comboBoxValue->findText(value);
    if ( itemIndex >= 0 )
      comboEditor->comboBoxValue->setCurrentIndex(itemIndex);
  } else {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    int cPos = lineEdit->cursorPosition();
    lineEdit->setText(value);
    lineEdit->setCursorPosition(cPos);
  }
}

void EmulatorOptionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptionDelegate::setModelData(QWidget *editor = %1, QAbstractItemModel *model = %2, const QModelIndex &index)").arg((qulonglong)editor).arg((qulonglong)model));
#endif

  if ( editor->accessibleName() == "checkBoxEditor" ) {
    optionType = QMC2_EMUOPT_TYPE_BOOL;
    QCheckBox *checkBoxEditor = static_cast<QCheckBox*>(editor);
    bool v = checkBoxEditor->isChecked();
    model->setData(index, QColor(0, 0, 0, 0), Qt::ForegroundRole);
    model->setData(index, QFont("Helvetiva", 1), Qt::FontRole);
    model->setData(index, v);
  } else if ( editor->accessibleName() == "spinBoxEditor" ) {
    optionType = QMC2_EMUOPT_TYPE_INT;
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int v = spinBox->value();
    model->setData(index, v);
  } else if ( editor->accessibleName() == "doubleSpinBoxEditor" ) {
    optionType = QMC2_EMUOPT_TYPE_FLOAT;
    QDoubleSpinBox *doubleSpinBox = static_cast<QDoubleSpinBox*>(editor);
    doubleSpinBox->interpretText();
    double v = doubleSpinBox->value();
    model->setData(index, v);
  } else if ( editor->accessibleName() == "float2Editor" ) {
    optionType = QMC2_EMUOPT_TYPE_FLOAT2;
    FloatEditWidget *float2Editor = static_cast<FloatEditWidget*>(editor);
    float2Editor->doubleSpinBox0->interpretText();
    float2Editor->doubleSpinBox1->interpretText();
    QLocale cLoc(QLocale::C);
    QString v = cLoc.toString(float2Editor->doubleSpinBox0->value()) + "," + cLoc.toString(float2Editor->doubleSpinBox1->value());
    model->setData(index, v);
  } else if ( editor->accessibleName() == "float3Editor" ) {
    optionType = QMC2_EMUOPT_TYPE_FLOAT3;
    FloatEditWidget *float3Editor = static_cast<FloatEditWidget*>(editor);
    float3Editor->doubleSpinBox0->interpretText();
    float3Editor->doubleSpinBox1->interpretText();
    float3Editor->doubleSpinBox2->interpretText();
    QLocale cLoc(QLocale::C);
    QString v = cLoc.toString(float3Editor->doubleSpinBox0->value()) + "," + cLoc.toString(float3Editor->doubleSpinBox1->value()) + "," + cLoc.toString(float3Editor->doubleSpinBox2->value());
    model->setData(index, v);
  } else if ( editor->accessibleName() == "fileEditor" ) {
    optionType = QMC2_EMUOPT_TYPE_FILE;
    FileEditWidget *fileEditor = static_cast<FileEditWidget*>(editor);
    QString v = fileEditor->lineEditFile->text();
    model->setData(index, v);
  } else if ( editor->accessibleName() == "directoryEditor" ) {
    optionType = QMC2_EMUOPT_TYPE_DIRECTORY;
    DirectoryEditWidget *directoryEditor = static_cast<DirectoryEditWidget*>(editor);
    QString v = directoryEditor->lineEditDirectory->text();
    model->setData(index, v);
  } else if ( editor->accessibleName() == "comboEditor" ) {
    optionType = QMC2_EMUOPT_TYPE_COMBO;
    ComboBoxEditWidget *comboEditor = static_cast<ComboBoxEditWidget*>(editor);
    QString v = comboEditor->comboBoxValue->lineEdit()->text();
    model->setData(index, v);
  } else {
    optionType = QMC2_EMUOPT_TYPE_STRING;
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    QString v = lineEdit->text();
    model->setData(index, v);
  }
}

void EmulatorOptionDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptionDelegate::updateEditorGeometry(QWidget *editor = %1, const QStyleOptionViewItem &option, const QModelIndex &index)").arg((qulonglong)editor));
#endif

  editor->setGeometry(option.rect);
  QFontMetrics fm(QApplication::font());
  QSize iconSize(fm.height() - 2, fm.height() - 2);
  if ( editor->accessibleName() == "fileEditor" ) {
    FileEditWidget *fileEditWidget = static_cast<FileEditWidget *>(editor);
    fileEditWidget->toolButtonBrowse->setIconSize(iconSize);
    fileEditWidget->toolButtonClear->setIconSize(iconSize);
  } else if ( editor->accessibleName() == "directoryEditor" ) {
    DirectoryEditWidget *directoryEditor = static_cast<DirectoryEditWidget *>(editor);
    directoryEditor->toolButtonBrowse->setIconSize(iconSize);
  }
}

EmulatorOptions::EmulatorOptions(QString group, QWidget *parent)
  : QTreeWidget(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptions::EmulatorOptions(QString group = %1, QWidget *parent = %2").arg(group).arg((qulonglong)parent));
#endif

  // initialize
  templateVersion = tr("unknown");
  connect(&searchTimer, SIGNAL(timeout()), this, SLOT(searchTimeout()));
  lineEditSearch = NULL;
  if ( !group.contains("Global") ) {
    emulatorOptions = NULL;
#if defined(QMC2_EMUTYPE_MAME)
    setStatusTip(tr("Game specific emulator configuration"));
#elif defined(QMC2_EMUTYPE_MESS)
    setStatusTip(tr("Machine specific emulator configuration"));
#endif
  } else {
    emulatorOptions = this;
    setStatusTip(tr("Global emulator configuration"));
  }
  loadActive = changed = false;
  settingsGroup = group;
  delegate = new EmulatorOptionDelegate(this);
  setColumnCount(2);  
  setItemDelegateForColumn(QMC2_EMUOPT_COLUMN_VALUE, delegate);
  setAlternatingRowColors(true);
  headerItem()->setText(0, tr("Option / Attribute"));
  headerItem()->setText(1, tr("Value"));
  header()->setClickable(false);
  if ( templateMap.count() == 0 )
    createTemplateMap();
  createMap();
}

EmulatorOptions::~EmulatorOptions()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::~EmulatorOptions()");
#endif

  if ( delegate )
    delete delegate;
}

void EmulatorOptions::pseudoConstructor()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::pseudoConstructor()");
#endif

  if ( emulatorOptions == this ) {
    header()->resizeSections(QHeaderView::Custom);
    header()->resizeSection(0, qmc2Config->value(settingsGroup + "/OptionColumnWidth", 200).toInt());
    header()->restoreState(qmc2Config->value(settingsGroup + "/HeaderState").toByteArray());
  } else {
    header()->setMovable(false);
  }
}

void EmulatorOptions::pseudoDestructor()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::pseudoDestructor()");
#endif

  if ( emulatorOptions == this ) {
    if ( header()->sectionSize(0) != 200 )
      qmc2Config->setValue(settingsGroup + "/OptionColumnWidth", header()->sectionSize(0));
    else
      qmc2Config->remove(settingsGroup + "/OptionColumnWidth");
    qmc2Config->setValue(settingsGroup + "/HeaderState", header()->saveState());
  }
}

void EmulatorOptions::load(bool overwrite)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::load(bool overwrite = " + QString(overwrite ? "true" : "false") + ")");
#endif

  loadActive = true;
  qmc2Config->beginGroup(settingsGroup);
  QString sectionTitle;
  foreach (sectionTitle, optionsMap.keys()) {
    EmulatorOption option;
    bool ok;
    int i;
    if ( qmc2GlobalEmulatorOptions != this ) {
      QTreeWidgetItem *item = sectionItemMap[sectionTitle];
      if ( item )
        if ( sectionExpansionMap[sectionTitle] )
          expandItem(item);
    }
    for (i = 0; i < optionsMap[sectionTitle].count(); i++) {
      option = optionsMap[sectionTitle][i];
      if ( qmc2GlobalEmulatorOptions != this )
        if ( optionExpansionMap[option.name] )
          expandItem(option.item);
      switch ( option.type ) {
        case QMC2_EMUOPT_TYPE_INT: {
          optionType = QMC2_EMUOPT_TYPE_INT;
          optionChoices.clear();
	  optionPart.clear();
          int v;
          if ( qmc2GlobalEmulatorOptions != this ) {
            if ( overwrite )
              v = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value.toInt();
            else
              v = qmc2Config->value(option.name, qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value.toInt()).toInt(&ok);
          } else
            v = qmc2Config->value(option.name, option.dvalue.toInt()).toInt(&ok);
          if ( ok ) {
            optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, v);
            optionsMap[sectionTitle][i].value.sprintf("%d", v);
            optionsMap[sectionTitle][i].valid = true;
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_FLOAT: {
          optionType = QMC2_EMUOPT_TYPE_FLOAT;
	  optionDecimals = option.decimals;
          optionChoices.clear();
	  optionPart.clear();
          double v;
          if ( qmc2GlobalEmulatorOptions != this ) {
            if ( overwrite )
              v = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value.toDouble();
            else
              v = qmc2Config->value(option.name, qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value.toDouble()).toDouble(&ok);
          } else
            v = qmc2Config->value(option.name, option.dvalue.toDouble()).toDouble(&ok);
          if ( ok ) {
            optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, v);
            optionsMap[sectionTitle][i].value.setNum(v, 'f', option.decimals);
            optionsMap[sectionTitle][i].valid = true;
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_FLOAT2: {
          optionType = QMC2_EMUOPT_TYPE_FLOAT2;
	  optionDecimals = option.decimals;
          optionChoices.clear();
	  optionPart.clear();
          QString v;
          if ( qmc2GlobalEmulatorOptions != this ) {
            if ( overwrite )
              v = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value;
            else
              v = qmc2Config->value(option.name, qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value).toString();
          } else
            v = qmc2Config->value(option.name, option.dvalue).toString();
          optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, v);
          optionsMap[sectionTitle][i].value = v;
          optionsMap[sectionTitle][i].valid = true;
          break;
        }

        case QMC2_EMUOPT_TYPE_FLOAT3: {
          optionType = QMC2_EMUOPT_TYPE_FLOAT3;
	  optionDecimals = option.decimals;
          optionChoices.clear();
	  optionPart.clear();
          QString v;
          if ( qmc2GlobalEmulatorOptions != this ) {
            if ( overwrite )
              v = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value;
            else
              v = qmc2Config->value(option.name, qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value).toString();
          } else
            v = qmc2Config->value(option.name, option.dvalue).toString();
          optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, v);
          optionsMap[sectionTitle][i].value = v;
          optionsMap[sectionTitle][i].valid = true;
          break;
        }

        case QMC2_EMUOPT_TYPE_BOOL: {
          optionType = QMC2_EMUOPT_TYPE_BOOL;
          optionChoices.clear();
	  optionPart.clear();
          bool v;
          if ( qmc2GlobalEmulatorOptions != this ) {
            if ( overwrite )
              v = EmulatorOptionDelegate::stringToBool(qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value);
            else
              v = EmulatorOptionDelegate::stringToBool(qmc2Config->value(option.name, qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value).toString());
          } else
            v = EmulatorOptionDelegate::stringToBool(qmc2Config->value(option.name, option.dvalue).toString());
          optionsMap[sectionTitle][i].value = EmulatorOptionDelegate::boolToString(v);
          optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::ForegroundRole, QColor(0, 0, 0, 0));
          optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::FontRole, QFont("Helvetiva", 1));
          optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, v);
          optionsMap[sectionTitle][i].valid = true;
          break;
        }

        case QMC2_EMUOPT_TYPE_COMBO:
        case QMC2_EMUOPT_TYPE_FILE:
        case QMC2_EMUOPT_TYPE_DIRECTORY:
        case QMC2_EMUOPT_TYPE_STRING:
        default: {
          optionType = option.type;
          if ( optionType == QMC2_EMUOPT_TYPE_COMBO )
            optionChoices = option.choices;
          else
            optionChoices.clear();
          if ( optionType == QMC2_EMUOPT_TYPE_FILE )
            optionPart = option.part;
          else
            optionPart.clear();
          QString v;
          if ( qmc2GlobalEmulatorOptions != this ) {
            if ( overwrite )
              v = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value;
            else
              v = qmc2Config->value(option.name, qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].value).toString();
          } else
            v = qmc2Config->value(option.name, option.dvalue).toString();
          optionsMap[sectionTitle][i].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, v);
          optionsMap[sectionTitle][i].value = v;
          optionsMap[sectionTitle][i].valid = true;
          break;
        }
      }
    }
  }

  qmc2Config->endGroup();

  loadActive = changed = false;
}

void EmulatorOptions::save()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::save()");
#endif

  if ( loadActive )
    return;

#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2DemoModeDialog )
    if ( qmc2DemoModeDialog->demoModeRunning )
      return;
#endif

  if ( qmc2GlobalEmulatorOptions != this ) {
    horizontalScrollPosition = horizontalScrollBar()->sliderPosition();
    verticalScrollPosition = verticalScrollBar()->sliderPosition();
  }

  qmc2Config->beginGroup(settingsGroup);
  QString sectionTitle;
  foreach (sectionTitle, optionsMap.keys()) {
    QString vs;
    int i;
    if ( qmc2GlobalEmulatorOptions != this ) {
      QTreeWidgetItem *item = sectionItemMap[sectionTitle];
      if ( item )
        sectionExpansionMap[sectionTitle] = item->isExpanded();
    }
    for (i = 0; i < optionsMap[sectionTitle].count(); i++) {
      if ( qmc2GlobalEmulatorOptions != this )
        optionExpansionMap[optionsMap[sectionTitle][i].name] = optionsMap[sectionTitle][i].item->isExpanded();
      switch ( optionsMap[sectionTitle][i].type ) {
        case QMC2_EMUOPT_TYPE_INT: {
          int v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toInt();
          int gv = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toInt();
          vs.sprintf("%d", v);
          if ( qmc2GlobalEmulatorOptions == this ) {
            if ( v != optionsMap[sectionTitle][i].dvalue.toInt() ) {
              optionsMap[sectionTitle][i].value = vs;
              qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
            } else {
              qmc2Config->remove(optionsMap[sectionTitle][i].name);
            }
          } else if ( v != gv && optionsMap[sectionTitle][i].valid ) {
            optionsMap[sectionTitle][i].value = vs;
            qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
          } else {
            qmc2Config->remove(optionsMap[sectionTitle][i].name);
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_FLOAT: {
          double v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toDouble();
          double gv = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toDouble();
          vs.setNum(v, 'f', optionsMap[sectionTitle][i].decimals);
          if ( qmc2GlobalEmulatorOptions == this ) {
            if ( v != optionsMap[sectionTitle][i].dvalue.toDouble() ) {
              optionsMap[sectionTitle][i].value = vs;
              qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
            } else {
              qmc2Config->remove(optionsMap[sectionTitle][i].name);
            }
          } else if ( v != gv && optionsMap[sectionTitle][i].valid ) {
            optionsMap[sectionTitle][i].value = vs;
            qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
          } else {
            qmc2Config->remove(optionsMap[sectionTitle][i].name);
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_FLOAT2:
        case QMC2_EMUOPT_TYPE_FLOAT3: {
          QString vs = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toString();
          QString gv = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toString();
          if ( qmc2GlobalEmulatorOptions == this ) {
            if ( vs != optionsMap[sectionTitle][i].dvalue ) {
              optionsMap[sectionTitle][i].value = vs;
              qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
            } else {
              qmc2Config->remove(optionsMap[sectionTitle][i].name);
            }
          } else if ( vs != gv && optionsMap[sectionTitle][i].valid ) {
            optionsMap[sectionTitle][i].value = vs;
            qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
          } else {
            qmc2Config->remove(optionsMap[sectionTitle][i].name);
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_BOOL: {
          bool v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toBool();
          bool gv = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toBool();
          if ( qmc2GlobalEmulatorOptions == this ) {
            if ( v != EmulatorOptionDelegate::stringToBool(optionsMap[sectionTitle][i].dvalue) ) {
              optionsMap[sectionTitle][i].value = EmulatorOptionDelegate::boolToString(v);
              qmc2Config->setValue(optionsMap[sectionTitle][i].name, EmulatorOptionDelegate::boolToString(v));
            } else {
              qmc2Config->remove(optionsMap[sectionTitle][i].name);
            }
          } else if ( v != gv && optionsMap[sectionTitle][i].valid ) {
            optionsMap[sectionTitle][i].value = EmulatorOptionDelegate::boolToString(v);
            qmc2Config->setValue(optionsMap[sectionTitle][i].name, EmulatorOptionDelegate::boolToString(v));
          } else {
            qmc2Config->remove(optionsMap[sectionTitle][i].name);
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_COMBO:
        case QMC2_EMUOPT_TYPE_FILE:
        case QMC2_EMUOPT_TYPE_DIRECTORY:
        case QMC2_EMUOPT_TYPE_STRING:
        default: {
          QString vs = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toString();
          QString gv = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toString();
          if ( qmc2GlobalEmulatorOptions == this ) {
            if ( vs != optionsMap[sectionTitle][i].dvalue ) {
              optionsMap[sectionTitle][i].value = vs;
              qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
            } else {
              qmc2Config->remove(optionsMap[sectionTitle][i].name);
            }
          } else if ( vs != gv && optionsMap[sectionTitle][i].valid ) {
            optionsMap[sectionTitle][i].value = vs;
            qmc2Config->setValue(optionsMap[sectionTitle][i].name, vs);
          } else {
            qmc2Config->remove(optionsMap[sectionTitle][i].name);
          }
          break;
        }
      }
    }
  }
  qmc2Config->endGroup();
  changed = false;
}

void EmulatorOptions::addChoices(QString optionName, QStringList choices)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptions::addChoices(QString optionName = %1, QStringList choices = ...)").arg(optionName));
#endif

	// IMPORTANT: it's assumed that 'optionName' refers to a combo-type option -- this isn't checked and will thus lead to crashes if the assumption is wrong!
	bool optFound = false;
	foreach (QString section, optionsMap.keys()) {
		foreach (EmulatorOption emuOpt, optionsMap[section]) {
			if ( emuOpt.name == optionName ) {
				ComboBoxEditWidget *comboWidget = (ComboBoxEditWidget *)itemWidget(emuOpt.item, QMC2_EMUOPT_COLUMN_VALUE);
				if ( comboWidget ) {
					QString value = comboWidget->comboBoxValue->lineEdit()->text();
					comboWidget->comboBoxValue->insertItems(comboWidget->comboBoxValue->count(), choices);
					comboWidget->comboBoxValue->lineEdit()->setText(value);
				}
				optFound = true;
			}
			if ( optFound ) break;
		}
		if ( optFound ) break;
	}
}

void EmulatorOptions::createMap()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::createMap()");
#endif

  optionsMap.clear();
  sectionItemMap.clear();
  QString sectionTitle;

  foreach ( sectionTitle, templateMap.keys() ) {
    QTreeWidgetItem *sectionItem = new QTreeWidgetItem(this);
    sectionItemMap[sectionTitle] = sectionItem;
    sectionItem->setText(0, sectionTitle);
    optionsMap[sectionTitle] = templateMap[sectionTitle];
    int i;
    for (i = 0; i < optionsMap[sectionTitle].count(); i++ ) {
      optionsMap[sectionTitle][i].value = optionsMap[sectionTitle][i].dvalue;
      EmulatorOption emulatorOption = optionsMap[sectionTitle].at(i);
      QTreeWidgetItem *optionItem = new QTreeWidgetItem(sectionItem);
      optionItem->setHidden(!emulatorOption.visible);
      optionsMap[sectionTitle][i].item = optionItem;
      optionItem->setText(0, emulatorOption.name);
      optionType = emulatorOption.type;
      optionDecimals = emulatorOption.decimals;
      QTreeWidgetItem *childItem;
      childItem = new QTreeWidgetItem(optionItem);
      childItem->setText(0, tr("Type"));
      switch ( emulatorOption.type ) {
        case QMC2_EMUOPT_TYPE_BOOL:
           childItem->setText(1, tr("bool"));
           break;

        case QMC2_EMUOPT_TYPE_INT:
           childItem->setText(1, tr("int"));
           break;

        case QMC2_EMUOPT_TYPE_FLOAT:
           childItem->setText(1, tr("float"));
           break;

        case QMC2_EMUOPT_TYPE_FLOAT2:
           childItem->setText(1, tr("float2"));
           break;

        case QMC2_EMUOPT_TYPE_FLOAT3:
           childItem->setText(1, tr("float3"));
           break;

        case QMC2_EMUOPT_TYPE_FILE:
           childItem->setText(1, tr("file"));
           break;

        case QMC2_EMUOPT_TYPE_DIRECTORY:
           childItem->setText(1, tr("directory"));
           break;

        case QMC2_EMUOPT_TYPE_COMBO:
           childItem->setText(1, tr("choice"));
           break;

        case QMC2_EMUOPT_TYPE_STRING:
           childItem->setText(1, tr("string"));
           break;

        default:
           childItem->setText(1, tr("unknown"));
           break;
      }
      if ( !emulatorOption.shortname.isEmpty() ) {
        childItem = new QTreeWidgetItem(optionItem);
        childItem->setText(0, tr("Short name"));
        childItem->setText(1, emulatorOption.shortname);
      }
      if ( !emulatorOption.dvalue.isEmpty() ) {
        childItem = new QTreeWidgetItem(optionItem);
        childItem->setText(0, tr("Default"));
        if ( emulatorOption.type == QMC2_EMUOPT_TYPE_BOOL ) {
          if ( emulatorOption.dvalue == "true" )
            childItem->setText(1, tr("true"));
          else
            childItem->setText(1, tr("false"));
        } else {
          childItem->setText(1, emulatorOption.dvalue);
        }
      }
      if ( !emulatorOption.description.isEmpty() ) {
        optionDescription = emulatorOption.description;
	optionChoices = emulatorOption.choices;
	optionPart = emulatorOption.part;
        optionItem->setToolTip(0, optionDescription);
        childItem = new QTreeWidgetItem(optionItem);
        childItem->setText(0, tr("Description"));
        childItem->setText(1, emulatorOption.description);
      } else
        optionDescription = "";
      openPersistentEditor(optionItem, 1);
    }
  } 
}

QString EmulatorOptions::readDescription(QXmlStreamReader *xmlReader, QString lang, bool *readNext)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::readDescription(...)");
#endif

  static QString translatedDescription;

  QMap<QString, QString> translations;

  while ( !xmlReader->atEnd() && *readNext ) {
    xmlReader->readNext();
    if ( !xmlReader->hasError() ) {
      if ( xmlReader->isStartElement() ) {
        QString elementType = xmlReader->name().toString();
        if ( elementType == "description" ) {
          QXmlStreamAttributes attributes = xmlReader->attributes();
          QString language = attributes.value("lang").toString();
          QString description = attributes.value("text").toString();
          translations[language] = description;
        } else
          *readNext = false;
      }
    } else
      *readNext = false;
  }

  if ( translations.contains(lang) )
    translatedDescription = translations[lang];
  else if ( translations.contains("us") )
    translatedDescription = translations["us"];
  else
    translatedDescription = tr("unknown");

  return translatedDescription;
}

QStringList EmulatorOptions::readChoices(QXmlStreamReader *xmlReader)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::readChoices(...)");
#endif

	static QStringList validChoices;

	validChoices.clear();
	bool readNext = true;

	while ( !xmlReader->atEnd() && readNext ) {
		if ( !xmlReader->hasError() ) {
			if ( xmlReader->isStartElement() ) {
				QString elementType = xmlReader->name().toString();
				if ( elementType == "choice" ) {
					QXmlStreamAttributes attributes = xmlReader->attributes();
					QString choiceName = attributes.value("name").toString();
					if ( !choiceName.isEmpty() )
						validChoices << choiceName;
				} else
					readNext = false;
			}
		} else
			readNext = false;
		if ( readNext )
			xmlReader->readNext();
	}

	return validChoices;
}

void EmulatorOptions::createTemplateMap()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::createTemplateMap()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("creating template configuration map"));
  
  sectionExpansionMap.clear();
  optionExpansionMap.clear();
  templateMap.clear();
  templateEmulator = tr("unknown");
  templateVersion = tr("unknown");
  templateFormat = tr("unknown");
#if defined(QMC2_EMUTYPE_MAME)
  QString templateFile = qmc2Config->value("MAME/FilesAndDirectories/OptionsTemplateFile").toString();
#elif defined(QMC2_EMUTYPE_MESS)
  QString templateFile = qmc2Config->value("MESS/FilesAndDirectories/OptionsTemplateFile").toString();
#endif
  if ( templateFile.isEmpty() )
#if defined(QMC2_SDLMAME)
    templateFile = QMC2_DEFAULT_DATA_PATH + "/opt/SDLMAME/template.xml";
#elif defined(QMC2_SDLMESS)
    templateFile = QMC2_DEFAULT_DATA_PATH + "/opt/SDLMESS/template.xml";
#elif defined(QMC2_MAME)
    templateFile = QMC2_DEFAULT_DATA_PATH + "/opt/MAME/template.xml";
#elif defined(QMC2_MESS)
    templateFile = QMC2_DEFAULT_DATA_PATH + "/opt/MESS/template.xml";
#endif
  QFile qmc2TemplateFile(templateFile);
  QString lang = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Language").toString();
  if ( lang.isEmpty() )
    lang = "us";
  if ( qmc2TemplateFile.open(QFile::ReadOnly) ) {
    QXmlStreamReader xmlReader(&qmc2TemplateFile);
    QString sectionTitle;
    bool readNext = true;
    while ( !xmlReader.atEnd() ) {
      if ( readNext )
        xmlReader.readNext();
      else
        readNext = true;
      if ( xmlReader.hasError() ) {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: XML error reading template: '%1' in file '%2' at line %3, column %4").
                            arg(xmlReader.errorString()).arg(templateFile).arg(xmlReader.lineNumber()).arg(xmlReader.columnNumber()));
      } else {
        if ( xmlReader.isStartElement() ) {
          QString elementType = xmlReader.name().toString();
          QXmlStreamAttributes attributes = xmlReader.attributes();
          QString name = attributes.value("name").toString();
          if ( elementType == "section" ) {
            sectionTitle = readDescription(&xmlReader, lang, &readNext);
            templateMap[sectionTitle].clear();
#ifdef QMC2_DEBUG
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: elementType = [%1], name = [%2], description = [%3]").
                                arg(elementType).arg(name).arg(sectionTitle));
#endif
          } else if ( elementType == "option" ) {
            bool ignore = false;
	    bool visible = true;
	    int decimals = QMC2_EMUOPT_DFLT_DECIMALS;
	    if ( attributes.hasAttribute("ignore") )
              ignore = attributes.value("ignore") == "true";
            if ( attributes.hasAttribute("visible") )
              visible = attributes.value("visible") == "true";
            if ( attributes.hasAttribute("decimals") )
              decimals = attributes.value("decimals").toString().toInt();
            if ( attributes.hasAttribute(QString("ignore.%1").arg(XSTR(BUILD_OS_NAME))) )
              ignore = attributes.value(QString("ignore.%1").arg(XSTR(BUILD_OS_NAME))) == "true";
            if ( !ignore ) {
              QString type = attributes.value("type").toString();
              QString defaultValue;
              if ( attributes.hasAttribute(QString("default.%1").arg(XSTR(BUILD_OS_NAME))) )
                defaultValue = attributes.value(QString("default.%1").arg(XSTR(BUILD_OS_NAME))).toString();
              else
                defaultValue = attributes.value("default").toString();
              QString optionDescription = readDescription(&xmlReader, lang, &readNext);
	      optionChoices.clear();
              if ( type == "combo" && xmlReader.name().toString() == "choice" )
                optionChoices = readChoices(&xmlReader);
	      optionPart.clear();
	      if ( type == "file" && attributes.hasAttribute("part") )
                optionPart = attributes.value("part").toString();
              templateMap[sectionTitle].append(EmulatorOption(name, "", type, defaultValue, optionDescription, QString::null, optionPart, NULL, false, decimals, optionChoices, visible));
#ifdef QMC2_DEBUG
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: elementType = [%1], name = [%2], type = [%3], default = [%4], description = [%5]").
                                  arg(elementType).arg(name).arg(type).arg(defaultValue).arg(optionDescription));
#endif
	    }
#ifdef QMC2_DEBUG
            else {
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ignored: elementType = [%1], name = [%2], description = [%3]").
                                  arg(elementType).arg(name).arg(optionDescription));
            }
#endif
          } else if ( elementType == "template" ) {
            templateEmulator = attributes.value("emulator").toString();
            templateVersion = attributes.value("version").toString();
            templateFormat = attributes.value("format").toString();
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("template info: emulator = %1, version = %2, format = %3").arg(templateEmulator).arg(templateVersion).arg(templateFormat));
          }
        }
      }
    }
    qmc2TemplateFile.close();
  } else
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open options template file"));

  if ( templateEmulator == tr("unknown") )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine emulator type of template"));
  if ( templateVersion == tr("unknown") )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine template version"));
  if ( templateFormat == tr("unknown") )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine template format"));
}

void EmulatorOptions::checkTemplateMap()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::checkTemplateMap()");
#endif

  if ( qmc2ReloadActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
    return;
  }

  QString userScopePath = QMC2_DYNAMIC_DOT_PATH;

  int diffCount = 0;
  QMap <QString, QString> emuOptions;

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("checking template configuration map against selected emulator"));

  QStringList args;
  QProcess commandProc;
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile").toString());
#if !defined(Q_WS_WIN)
  commandProc.setStandardErrorFile("/dev/null");
#endif

  args << "-noreadconfig" << "-showconfig";
  qApp->processEvents();
  bool commandProcStarted = false;
  int retries = 0;
#if defined(QMC2_EMUTYPE_MAME)
  commandProc.start(qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString(), args);
#elif defined(QMC2_EMUTYPE_MESS)
  commandProc.start(qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString(), args);
#endif
  bool started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
  while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
    started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
    qApp->processEvents();
  }
  if ( started ) {
    commandProcStarted = true;
    bool commandProcRunning = (commandProc.state() == QProcess::Running);
    while ( !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) && commandProcRunning ) {
      qApp->processEvents();
      commandProcRunning = (commandProc.state() == QProcess::Running);
    }
  } else {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MAME executable within a reasonable time frame, giving up"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MESS executable within a reasonable time frame, giving up"));
#endif
    return;
  }

#if defined(QMC2_SDLMAME)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLMESS)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#else
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-unknown.tmp").toString());
#endif
  if ( commandProcStarted && qmc2Temp.open(QFile::ReadOnly) ) {
    QTextStream ts(&qmc2Temp);
    QString s = ts.readAll();
    qmc2Temp.close();
    qmc2Temp.remove();
#if !defined(Q_WS_WIN)
    QStringList sl = s.split("\n");
#else
    QStringList sl = s.split("\r\n");
#endif
    foreach (QString line, sl) {
      QString l = line.simplified();
      if ( l.isEmpty() ) continue;
      if ( l.startsWith("#") ) continue;
      if ( l.startsWith("<") ) continue;
      if ( l.startsWith("readconfig") ) continue;
#if defined(QMC2_SDLMESS)
      // this is a 'non-SDL Windows MESS'-only option and is ignored for all SDLMESS builds
      if ( l.startsWith("newui") ) continue;
#endif
      QStringList wl;
      QRegExp rx("(\\S+|\\\".*\\\")");
      int pos = 0;
      while ( (pos = rx.indexIn(l, pos)) != -1 ) {
        QString s = rx.cap(1);
        s = s.remove("\"");
        wl << s;
        pos += rx.matchedLength();
      }
      if ( wl.count() == 2 )
        emuOptions[wl[0]] = wl[1];
      else if ( wl.count() == 1 )
        emuOptions[wl[0]] = "";
      else
        continue;
#ifdef QMC2_DEBUG
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: option [%1], default value [%2]").arg(wl[0]).arg(emuOptions[wl[0]]));
#endif
    }
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create temporary file, please check emulator executable and permissions"));
  }
  
  QStringList templateOptions;
  foreach (QString sectionTitle, optionsMap.keys()) {
    EmulatorOption option;
    QString assumedType = "unknown";
    int i;
    for (i = 0; i < optionsMap[sectionTitle].count(); i++) {
      option = optionsMap[sectionTitle][i];
      templateOptions << option.name;
      if ( emuOptions.contains(option.name) ) {
        switch ( option.type ) {
          case QMC2_EMUOPT_TYPE_INT:
            assumedType = "int";
            if ( option.dvalue.toInt() != emuOptions[option.name].toInt() ) {
              diffCount++;
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator uses a different default value for option '%1' ('%2' vs. '%3'); assumed option type is '%4'")
                                                     .arg(option.name).arg(option.dvalue.toInt()).arg(emuOptions[option.name].toInt()).arg(assumedType));
            }
            break;

          case QMC2_EMUOPT_TYPE_FLOAT:
            assumedType = "float";
            if ( option.dvalue.toDouble() != emuOptions[option.name].toDouble() ) {
              diffCount++;
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator uses a different default value for option '%1' ('%2' vs. '%3'); assumed option type is '%4'")
                                                     .arg(option.name).arg(option.dvalue.toDouble()).arg(emuOptions[option.name].toDouble()).arg(assumedType));
            }
            break;

          case QMC2_EMUOPT_TYPE_FLOAT2:
            assumedType = "float2";
            if ( option.dvalue != emuOptions[option.name] ) {
              diffCount++;
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator uses a different default value for option '%1' ('%2' vs. '%3'); assumed option type is '%4'")
                                                     .arg(option.name).arg(option.dvalue).arg(emuOptions[option.name]).arg(assumedType));
            }
            break;

          case QMC2_EMUOPT_TYPE_FLOAT3:
            assumedType = "float3";
            if ( option.dvalue != emuOptions[option.name] ) {
              diffCount++;
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator uses a different default value for option '%1' ('%2' vs. '%3'); assumed option type is '%4'")
                                                     .arg(option.name).arg(option.dvalue).arg(emuOptions[option.name]).arg(assumedType));
            }
            break;

          case QMC2_EMUOPT_TYPE_BOOL: {
            assumedType = "bool";
            QString emuOpt = "true";
            if ( emuOptions[option.name] == "0" )
              emuOpt = "false";
            if ( option.dvalue != emuOpt ) {
              diffCount++;
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator uses a different default value for option '%1' ('%2' vs. '%3'); assumed option type is '%4'")
                                                     .arg(option.name).arg(option.dvalue).arg(emuOptions[option.name]).arg(assumedType));
            }
            break;
          }

          case QMC2_EMUOPT_TYPE_COMBO:
            assumedType = "combo";
          case QMC2_EMUOPT_TYPE_FILE:
            if ( assumedType == "unknown" ) assumedType = "file";
          case QMC2_EMUOPT_TYPE_DIRECTORY:
            if ( assumedType == "unknown" ) assumedType = "directory";
          case QMC2_EMUOPT_TYPE_STRING:
          default:
            if ( assumedType == "unknown" ) assumedType = "string";
            if ( option.dvalue.replace("$HOME", "~") != emuOptions[option.name].replace("$HOME", "~") ) {
              diffCount++;
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator uses a different default value for option '%1' ('%2' vs. '%3'); assumed option type is '%4'")
                                                     .arg(option.name).arg(option.dvalue).arg(emuOptions[option.name]).arg(assumedType));
            }
            break;
        }
      } else if ( option.name != "readconfig" ) {
        diffCount++;
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("template option '%1' is unknown to the emulator").arg(option.name));
      }
    }
  }

  QMapIterator<QString, QString> it(emuOptions);
  while ( it.hasNext() ) {
    it.next();
    if ( !templateOptions.contains(it.key()) ) {
      diffCount++;
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator option '%1' with default value '%2' is unknown to the template").arg(it.key()).arg(it.value()));
    }
  }

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (checking template configuration map against selected emulator)"));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check results: %n difference(s)", "", diffCount));
}

void EmulatorOptions::keyPressEvent(QKeyEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::keyPressEvent(QKeyPressEvent *e)");
#endif

  if ( e->key() == Qt::Key_Escape ) {
    if ( lineEditSearch ) {
      if ( lineEditSearch->isVisible() )
        searchTimeout();
      else
        QAbstractItemView::keyPressEvent(e);
    } else
      QAbstractItemView::keyPressEvent(e);
    return;
  }

  if ( !e->text().isEmpty() ) {
    if ( !lineEditSearch )
      lineEditSearch = new QLineEdit(this);
    lineEditSearch->show();
    lineEditSearch->raise();
    lineEditSearch->event(e);
    searchTimer.start(QMC2_SEARCH_TIMEOUT);
    clearSelection();
    if ( !lineEditSearch->text().isEmpty() ) {
      int i;
      bool madeCurrent = false;
      QList<QTreeWidgetItem *> foundItems = findItems(lineEditSearch->text(), Qt::MatchRecursive | Qt::MatchContains, 0);
      for (i = 0; i < foundItems.count(); i++) {
        QTreeWidgetItem *p = foundItems[i];
        if ( p->parent() ) {
          if ( !p->parent()->parent() ) {
            p->setSelected(true);
            p->parent()->setSelected(true);
            if ( !madeCurrent ) {
              scrollToItem(p);
              setCurrentItem(p);
              madeCurrent = true;
            }
          }
        }
      }
    }
    e->accept();
  } else {
    QAbstractItemView::keyPressEvent(e);
  }
}

void EmulatorOptions::searchTimeout()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmulatorOptions::searchTimeout()");
#endif

  searchTimer.stop();
  lineEditSearch->hide();
  lineEditSearch->close();
  delete lineEditSearch;
  lineEditSearch = NULL;
}

void EmulatorOptions::exportToIni(bool global, QString useFileName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptions::exportToIni(bool global = %1, QString useFileName = %2)").arg(global).arg(useFileName));
#endif

  static QBrush redBrush(QColor(255, 0, 0));
  static QBrush greenBrush(QColor(0, 255, 0));

#if defined(QMC2_EMUTYPE_MAME)
  QStringList iniPaths = qmc2Config->value("MAME/Configuration/Global/inipath").toString().split(";", QString::SkipEmptyParts);
#elif defined(QMC2_EMUTYPE_MESS)
  QStringList iniPaths = qmc2Config->value("MESS/Configuration/Global/inipath").toString().split(";", QString::SkipEmptyParts);
#endif
  if ( iniPaths.isEmpty() ) {
    // lookup default value for inipath
    foreach (QString sectionTitle, qmc2GlobalEmulatorOptions->optionsMap.keys() ) {
      for (int optionPos = 0; optionPos < qmc2GlobalEmulatorOptions->optionsMap[sectionTitle].count() && iniPaths.isEmpty(); optionPos++) {
        if ( qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][optionPos].name == "inipath" ) {
          iniPaths = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][optionPos].dvalue.split(";", QString::SkipEmptyParts);
        }
      }
    }
  }
  QStringList writableIniPaths;
  QStringList fileNames;

  // determine list of writable ini-paths
  foreach (QString path, iniPaths) {
    QString pathCopy(path);
    QFileInfo dirInfo(pathCopy.replace("~", QDir::homePath()).replace("$HOME", QDir::homePath()));
    if ( dirInfo.isWritable() && dirInfo.isExecutable() )
      writableIniPaths << path;
  }

  // get correct filename(s) for ini-export
  QString fileName;
  if ( useFileName.isEmpty() ) {
    int i;
    if ( writableIniPaths.count() == 1 )
      iniPaths = writableIniPaths;
    else if ( writableIniPaths.count() < 1 )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ini-export: no writable ini-paths found"));
    if ( iniPaths.count() > 1 ) {
      // multiple ini-paths detected - let user select one or more ini-paths to export to, pre-select all dirs
      ItemSelector itemSelector(this, iniPaths);
      itemSelector.setWindowTitle(tr("Path selection"));
      itemSelector.labelMessage->setText(tr("Multiple ini-paths detected. Select path(s) to export to:"));
      itemSelector.listWidgetItems->setSelectionMode(QAbstractItemView::ExtendedSelection);
      foreach (QListWidgetItem *item, itemSelector.listWidgetItems->findItems("*", Qt::MatchWildcard)) {
        if ( writableIniPaths.contains(item->text()) ) {
          item->setForeground(greenBrush);
          item->setSelected(true);
        } else {
          item->setForeground(redBrush);
          item->setSelected(false);
        }
      }
      if ( itemSelector.exec() == QDialog::Rejected )
        return;
      QList<QListWidgetItem *> itemList = itemSelector.listWidgetItems->selectedItems();
      iniPaths.clear();
      for (i = 0; i < itemList.count(); i++)
        iniPaths << itemList[i]->text();
    }
    if ( iniPaths.count() < 1 ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ini-export: no path selected (or invalid inipath)"));
      return;
    }
    if ( qmc2GlobalEmulatorOptions == this ) {
#if defined(QMC2_EMUTYPE_MAME)
      fileName = "/mame.ini";
#elif defined(QMC2_EMUTYPE_MESS)
      fileName = "/mess.ini";
#endif
    } else {
      if ( !qmc2CurrentItem )
        return;
      if ( qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
        return;
      fileName = "/" + qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON) + ".ini";
    }
    for (i = 0; i < iniPaths.count(); i++) {
      QString fn(fileName);
      fn = fn.prepend(iniPaths[i]);
      fileNames << fn.replace("~", QDir::homePath()).replace("$HOME", QDir::homePath());
    }
  } else {
    fileNames << useFileName;
  }
  
  foreach (fileName, fileNames) {
    QFile iniFile(fileName);
    if ( !iniFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open export file for writing, path = %1").arg(fileName));
      continue;
    }
    QTime elapsedTime;
    miscTimer.start();
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting %1 MAME configuration to %2").arg(global ? tr("global") : tr("game-specific")).arg(fileName));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting %1 MESS configuration to %2").arg(global ? tr("global") : tr("machine-specific")).arg(fileName));
#endif
    QTextStream ts(&iniFile);
    QString sectionTitle;
    qmc2Config->beginGroup(settingsGroup);
    foreach ( sectionTitle, optionsMap.keys() ) {
      QString vs;
      int i;
      for (i = 0; i < optionsMap[sectionTitle].count(); i++) {
        switch ( optionsMap[sectionTitle][i].type ) {
          case QMC2_EMUOPT_TYPE_INT: {
            int v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toInt();
            ts << optionsMap[sectionTitle][i].name << " " << v << "\n";
            break;
          }

          case QMC2_EMUOPT_TYPE_FLOAT: {
            double v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toDouble();
            ts << optionsMap[sectionTitle][i].name << " " << v << "\n";
            break;
          }

          case QMC2_EMUOPT_TYPE_FLOAT2:
          case QMC2_EMUOPT_TYPE_FLOAT3: {
            QString v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toString();
            ts << optionsMap[sectionTitle][i].name << " " << v << "\n";
            break;
          }

          case QMC2_EMUOPT_TYPE_BOOL: {
            bool v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toBool();
            ts << optionsMap[sectionTitle][i].name << " " << (v ? "1" : "0") << "\n";
            break;
          }

          case QMC2_EMUOPT_TYPE_COMBO:
          case QMC2_EMUOPT_TYPE_FILE:
          case QMC2_EMUOPT_TYPE_DIRECTORY:
          case QMC2_EMUOPT_TYPE_STRING:
          default: {
            QString v = optionsMap[sectionTitle][i].item->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toString();
            ts << optionsMap[sectionTitle][i].name << " " << v.replace("~", "$HOME") << "\n";
            break;
          }
        }
      }
    }
    qmc2Config->endGroup();
    iniFile.close();
    elapsedTime = elapsedTime.addMSecs(miscTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting %1 MAME configuration to %2, elapsed time = %3)").arg(global ? tr("global") : tr("game-specific")).arg(fileName).arg(elapsedTime.toString("mm:ss.zzz")));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting %1 MESS configuration to %2, elapsed time = %3)").arg(global ? tr("global") : tr("machine-specific")).arg(fileName).arg(elapsedTime.toString("mm:ss.zzz")));
#endif
  }
}

void EmulatorOptions::importFromIni(bool global, QString useFileName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmulatorOptions::importFromIni(bool global = %1, QString useFileName = %2)").arg(global).arg(useFileName));
#endif

  static QBrush redBrush(QColor(255, 0, 0));
  static QBrush greenBrush(QColor(0, 255, 0));

#if defined(QMC2_EMUTYPE_MAME)
  QStringList iniPaths = qmc2Config->value("MAME/Configuration/Global/inipath").toString().split(";", QString::SkipEmptyParts);
#elif defined(QMC2_EMUTYPE_MESS)
  QStringList iniPaths = qmc2Config->value("MESS/Configuration/Global/inipath").toString().split(";", QString::SkipEmptyParts);
#endif
  if ( iniPaths.isEmpty() ) {
    // lookup default value for inipath
    foreach (QString sectionTitle, qmc2GlobalEmulatorOptions->optionsMap.keys() ) {
      for (int optionPos = 0; optionPos < qmc2GlobalEmulatorOptions->optionsMap[sectionTitle].count() && iniPaths.isEmpty(); optionPos++) {
        if ( qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][optionPos].name == "inipath" ) {
          iniPaths = qmc2GlobalEmulatorOptions->optionsMap[sectionTitle][optionPos].dvalue.split(";", QString::SkipEmptyParts);
        }
      }
    }
  }
  QStringList readableIniPaths;
  QStringList fileNames;

  // determine list of readable ini-paths
  foreach (QString path, iniPaths) {
    QString pathCopy(path);
    QFileInfo dirInfo(pathCopy.replace("~", QDir::homePath()).replace("$HOME", QDir::homePath()));
    if ( dirInfo.isReadable() && dirInfo.isExecutable() )
      readableIniPaths << path;
  }

  // get correct filename for ini-import
  QString fileName;
  if ( useFileName.isEmpty() ) {
    int i;
    if ( readableIniPaths.count() == 1 )
      iniPaths = readableIniPaths;
    else if ( readableIniPaths.count() < 1 )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ini-import: no readable ini-paths found"));
    if ( iniPaths.count() > 1 ) {
      // multiple ini-paths detected - let user select the ini-path to import from, pre-select first dir
      ItemSelector itemSelector(this, iniPaths);
      itemSelector.setWindowTitle(tr("Path selection"));
      itemSelector.labelMessage->setText(tr("Multiple ini-paths detected. Select path(s) to import from:"));
      itemSelector.listWidgetItems->setSelectionMode(QAbstractItemView::ExtendedSelection);
      foreach (QListWidgetItem *item, itemSelector.listWidgetItems->findItems("*", Qt::MatchWildcard)) {
        if ( readableIniPaths.contains(item->text()) ) {
          item->setForeground(greenBrush);
          item->setSelected(true);
        } else {
          item->setForeground(redBrush);
          item->setSelected(false);
        }
      }
      if ( itemSelector.exec() == QDialog::Rejected )
        return;
      QList<QListWidgetItem *> itemList = itemSelector.listWidgetItems->selectedItems();
      iniPaths.clear();
      for (i = 0; i < itemList.count(); i++)
        iniPaths << itemList[i]->text();
    }
    if ( iniPaths.count() < 1 ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ini-import: no path selected (or invalid inipath)"));
      return;
    }
    if ( qmc2GlobalEmulatorOptions == this ) {
#if defined(QMC2_EMUTYPE_MAME)
      fileName = "/mame.ini";
#elif defined(QMC2_EMUTYPE_MESS)
      fileName = "/mess.ini";
#endif
    } else {
      if ( !qmc2CurrentItem )
        return;
      if ( qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
        return;
      QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
      fileName = "/" + gameName + ".ini";
    }
    for (i = 0; i < iniPaths.count(); i++) {
      QString fn(fileName);
      fn = fn.prepend(iniPaths[i]);
      fileNames << fn.replace("~", QDir::homePath()).replace("$HOME", QDir::homePath());
    }
  } else {
    fileNames << useFileName;
  }
  
  foreach (fileName, fileNames) {
    QFile iniFile(fileName);
    if ( !iniFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open import file for reading, path = %1").arg(fileName));
      continue;
    }
    QTime elapsedTime;
    miscTimer.start();
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("importing %1 MAME configuration from %2").arg(global ? tr("global") : tr("game-specific")). arg(fileName));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("importing %1 MESS configuration from %2").arg(global ? tr("global") : tr("machine-specific")). arg(fileName));
#endif
    QTextStream ts(&iniFile);

    // read ini-file and set corresponding emulator options to their values
    int lineCounter = 0;
    while ( !ts.atEnd() ) {
      qApp->processEvents();
      QString lineTrimmed = ts.readLine().trimmed();
      lineCounter++;
      if ( !lineTrimmed.isEmpty() && !lineTrimmed.startsWith("#") && !lineTrimmed.startsWith("<UNADORNED") ) {
        QStringList words = lineTrimmed.split(QRegExp("\\s+"));
        if ( words.count() > 0 ) {
          QString option = words[0];

          // lookup option in map
          QString sectionTitle, sectionTitleFound = "";
          int optionPos, optionPosFound = -1;
          foreach ( sectionTitle, optionsMap.keys() ) {
            for (optionPos = 0; optionPos < optionsMap[sectionTitle].count() && optionPosFound == -1; optionPos++ ) {
              if ( optionsMap[sectionTitle][optionPos].name == option ) {
                sectionTitleFound = sectionTitle;
                optionPosFound = optionPos;
              }
            }
            if ( optionPosFound != -1 )
              break;
          }

          if ( optionPosFound != -1 && words.count() > 1 ) {
            QString value = lineTrimmed.mid(lineTrimmed.indexOf(words[1], words[0].length()));
            switch ( optionsMap[sectionTitleFound][optionPosFound].type ) {
              case QMC2_EMUOPT_TYPE_INT: {
                if ( qmc2GlobalEmulatorOptions == this )
                  qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value.toInt());
                else
                  qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value.toInt());
                break;
              }

              case QMC2_EMUOPT_TYPE_FLOAT: {
                if ( qmc2GlobalEmulatorOptions == this )
                  qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value.toDouble());
                else
                  qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value.toDouble());
                break;
              }

              case QMC2_EMUOPT_TYPE_FLOAT2:
              case QMC2_EMUOPT_TYPE_FLOAT3: {
                if ( qmc2GlobalEmulatorOptions == this )
                  qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value);
                else
                  qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value);
                break;
              }

              case QMC2_EMUOPT_TYPE_BOOL: {
                if ( qmc2GlobalEmulatorOptions == this ) {
                  qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::ForegroundRole, QColor(0, 0, 0, 0));
                  qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::FontRole, QFont("Helvetiva", 1));
                  if ( value == "0" )
                    qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, false);
                  else
                    qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, true);
                } else {
                  qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::ForegroundRole, QColor(0, 0, 0, 0));
                  qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::FontRole, QFont("Helvetiva", 1));
                  if ( value == "0" )
                    qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, false);
                  else
                    qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, true);
                }
                break;
              }

              case QMC2_EMUOPT_TYPE_COMBO:
              case QMC2_EMUOPT_TYPE_FILE:
              case QMC2_EMUOPT_TYPE_DIRECTORY:
              case QMC2_EMUOPT_TYPE_STRING:
              default: {
                if ( qmc2GlobalEmulatorOptions == this )
                  qmc2GlobalEmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value.replace("$HOME", "~"));
                else
                  qmc2EmulatorOptions->optionsMap[sectionTitleFound][optionPosFound].item->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, value.replace("$HOME", "~"));
                break;
              }
            }
          } else if ( optionPosFound == -1 ) {
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: unknown option '%1' at line %2 (%3) ignored").arg(option).arg(lineCounter).arg(fileName));
          }
        } else {
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: invalid syntax at line %1 (%2) ignored").arg(lineCounter).arg(fileName));
        }
      }
    }
    iniFile.close();
    elapsedTime = elapsedTime.addMSecs(miscTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (importing %1 MAME configuration from %2, elapsed time = %3)").arg(global ? tr("global") : tr("game-specific")).arg(fileName).arg(elapsedTime.toString("mm:ss.zzz")));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (importing %1 MESS configuration from %2, elapsed time = %3)").arg(global ? tr("global") : tr("machine-specific")).arg(fileName).arg(elapsedTime.toString("mm:ss.zzz")));
#endif
  }
}
