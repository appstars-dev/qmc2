#ifndef _EMUOPT_H_
#define _EMUOPT_H_

#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QCheckBox>
#include <QTreeWidget>
#include <QSettings>
#include <QMap>
#include <QTimer>
#include <QTime>
#include <QKeyEvent>
#include <QXmlStreamReader>

#include "macros.h"

class EmulatorOptionDelegate : public QStyledItemDelegate
{
  Q_OBJECT

  public:
    EmulatorOptionDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;
    virtual void setEditorData(QWidget *, const QModelIndex &) const;
    virtual void setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const;
    virtual void updateEditorGeometry(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;

    static QString boolToString(bool b) { return (b ? QString("true") : QString("false")); }
    static bool stringToBool(QString s) { return (s == "true" ? true : false); }

  public slots:
    void dataChanged();
};

class EmulatorOption
{
  public:
    QString name, shortname, dvalue, description, value, part;
    QStringList choices;
    bool valid, visible;
    int type;
    int decimals;
    QTreeWidgetItem *item;

    EmulatorOption() {
      valid = false;
    };
    EmulatorOption(QString n, QString sn, QString t, QString dv, QString d, QString v, QString p, QTreeWidgetItem *i, bool va, int dec = QMC2_EMUOPT_DFLT_DECIMALS, QStringList c = QStringList(), bool vis = true) {
      name = n;
      shortname = sn;
      type = QMC2_EMUOPT_TYPE_UNKNOWN;
      switch ( t.at(0).toAscii() ) {
        case 'b': type = QMC2_EMUOPT_TYPE_BOOL; break;
        case 'i': type = QMC2_EMUOPT_TYPE_INT; break;
        case 'f': if ( t == "float" || t == "float1" ) type = QMC2_EMUOPT_TYPE_FLOAT;
                  else if ( t == "file" ) type = QMC2_EMUOPT_TYPE_FILE;
                  else if ( t == "float2" ) type = QMC2_EMUOPT_TYPE_FLOAT2;
                  else if ( t == "float3" ) type = QMC2_EMUOPT_TYPE_FLOAT3;
                  break;
        case 's': type = QMC2_EMUOPT_TYPE_STRING; break;
        case 'd': type = QMC2_EMUOPT_TYPE_DIRECTORY; break;
        case 'c': type = QMC2_EMUOPT_TYPE_COMBO; break;
        default: type = QMC2_EMUOPT_TYPE_UNKNOWN; break;
      }
      dvalue = dv;
      description = d;
      value = v;
      part = p;
      item = i;
      valid = va;
      choices = c;
      visible = vis;
      decimals = dec;
    }
};

class EmulatorOptions : public QTreeWidget
{
  Q_OBJECT

  public:
    QTimer searchTimer;
    QTime miscTimer;
    QLineEdit *lineEditSearch;
    EmulatorOptionDelegate *delegate;
    QString settingsGroup;
    QString templateEmulator;
    QString templateVersion;
    QString templateFormat;
    QMap<QString, QList<EmulatorOption> > optionsMap;
    static QMap<QString, QList<EmulatorOption> > templateMap;
    static QMap<QString, bool> sectionExpansionMap;
    static QMap<QString, QTreeWidgetItem *> sectionItemMap;
    static QMap<QString, bool> optionExpansionMap;
    static int horizontalScrollPosition;
    static int verticalScrollPosition;
    bool loadActive;
    bool changed;

    EmulatorOptions(QString, QWidget *parent = 0);
    ~EmulatorOptions();

    void pseudoConstructor();
    void pseudoDestructor();
    QString readDescription(QXmlStreamReader *, QString, bool *);
    QStringList readChoices(QXmlStreamReader *);

  public slots:
    void load(bool overwrite = false);
    void save();
    void addChoices(QString, QStringList);
    void createTemplateMap();
    void checkTemplateMap();
    void createMap();
    void searchTimeout();
    void exportToIni(bool global, QString useFileName = QString());
    void importFromIni(bool global, QString useFileName = QString());

  protected:
    virtual void keyPressEvent(QKeyEvent *);
};

#endif
