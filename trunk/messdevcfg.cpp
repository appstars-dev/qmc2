#include <QList>
#include <QMap>
#include <QHeaderView>
#include <QFileInfo>
#include <QFileDialog>

#include "messdevcfg.h"
#include "gamelist.h"
#include "macros.h"
#include "qmc2main.h"
#include "options.h"
#include "fileeditwidget.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;
extern Gamelist *qmc2Gamelist;
extern bool qmc2CleaningUp;
extern bool qmc2EarlyStartup;
extern QString qmc2FileEditStartPath;

QMap<QString, QString> messXmlDataCache;
QList<FileEditWidget *> messFileEditWidgetList;

MESSDeviceFileDelegate::MESSDeviceFileDelegate(QObject *parent)
  : QItemDelegate(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::MESSDeviceFileDelegate(QObject *parent = %1)").arg((qulonglong)parent));
#endif

  messFileEditWidgetList.clear();
}

QWidget *MESSDeviceFileDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::createEditor(QWidget *parent = %1, const QStyleOptionViewItem &option, const QModelIndex &index)").arg((qulonglong)parent));
#endif

  int row = index.row();
  QModelIndex sibling = index.sibling(row, QMC2_DEVCONFIG_COLUMN_EXT);
  QStringList extensions = sibling.model()->data(sibling, Qt::EditRole).toString().split("/", QString::SkipEmptyParts);
  QString filterString = tr("All files") + " (*)";
  if ( extensions.count() > 0 ) {
#if defined(Q_WS_WIN)
    filterString = tr("Valid device files") + " (*.zip";
#else
    filterString = tr("Valid device files") + " (*.[zZ][iI][pP]";
#endif
    for (int i = 0; i < extensions.count(); i++)
      filterString += QString(" *.%1").arg(extensions[i]);
    filterString += ");;" + tr("All files") + " (*)";
  }
  FileEditWidget *fileEditWidget = new FileEditWidget("", filterString, parent);
  fileEditWidget->installEventFilter(const_cast<MESSDeviceFileDelegate*>(this));
  connect(fileEditWidget, SIGNAL(dataChanged(QWidget *)), this, SLOT(dataChanged(QWidget *)));
  messFileEditWidgetList.insert(row, fileEditWidget);

  return fileEditWidget;
}

void MESSDeviceFileDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::setEditorData(QWidget *editor = %1, const QModelIndex &index)").arg((qulonglong)editor));
#endif

  QString value = index.model()->data(index, Qt::EditRole).toString();
  FileEditWidget *fileEditWidget = static_cast<FileEditWidget *>(editor);
  int cPos = fileEditWidget->lineEditFile->cursorPosition();
  fileEditWidget->lineEditFile->setText(value);
  fileEditWidget->lineEditFile->setCursorPosition(cPos);
}

void MESSDeviceFileDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::setModelData(QWidget *editor = %1, QAbstractItemModel *model = %2, const QModelIndex &index)").arg((qulonglong)editor).arg((qulonglong)model));
#endif

  FileEditWidget *fileEditWidget = static_cast<FileEditWidget*>(editor);
  QString v = fileEditWidget->lineEditFile->text();
  model->setData(index, v);
}

void MESSDeviceFileDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::updateEditorGeometry(QWidget *editor = %1, const QStyleOptionViewItem &option, const QModelIndex &index)").arg((qulonglong)editor));
#endif

  editor->setGeometry(option.rect);
  QFontMetrics fm(QApplication::font());
  QSize iconSize(fm.height() - 2, fm.height() - 2);
  FileEditWidget *fileEditWidget = static_cast<FileEditWidget *>(editor);
  fileEditWidget->toolButtonBrowse->setIconSize(iconSize);
}

void MESSDeviceFileDelegate::dataChanged(QWidget *widget)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceFileDelegate::dataChanged(QWidget *widget = %1)").arg((qulonglong)widget));
#endif

  emit commitData(widget);
  FileEditWidget *fileEditWidget = static_cast<FileEditWidget*>(widget);
  emit editorDataChanged(fileEditWidget->lineEditFile->text());
}

MESSDeviceConfigurator::MESSDeviceConfigurator(QString machineName, QWidget *parent)
  : QWidget(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::MESSDeviceConfigurator(QString machineName = %1, QWidget *parent = %2)").arg(machineName).arg((qulonglong)parent));
#endif

  setupUi(this);

  messMachineName = machineName;
  dontIgnoreNameChange = false;
  treeWidgetDeviceSetup->setItemDelegateForColumn(QMC2_DEVCONFIG_COLUMN_FILE, &fileEditDelegate);
  connect(&fileEditDelegate, SIGNAL(editorDataChanged(const QString &)), this, SLOT(editorDataChanged(const QString &)));
  treeWidgetDeviceSetup->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/DeviceSetupHeaderState").toByteArray());
  QList<int> vSplitterSizes;
  QSize vSplitterSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/vSplitter").toSize();
  if ( vSplitterSize.width() > 0 || vSplitterSize.height() > 0 )
    vSplitterSizes << vSplitterSize.width() << vSplitterSize.height();
  else
    vSplitterSizes << 100 << 100;
  vSplitter->setSizes(vSplitterSizes);

  QFontMetrics fm(QApplication::font());
  QSize iconSize(fm.height() - 2, fm.height() - 2);
  pushButtonNewConfiguration->setIconSize(iconSize);
  pushButtonCloneConfiguration->setIconSize(iconSize);
  pushButtonSaveConfiguration->setIconSize(iconSize);
  pushButtonRemoveConfiguration->setIconSize(iconSize);

  // configuration menu
  configurationMenu = new QMenu(pushButtonConfiguration);
  QString s = tr("Select default device directory");
  QAction *action = configurationMenu->addAction(tr("&Default device directory for '%1'...").arg(messMachineName));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(actionSelectDefaultDeviceDirectory_triggered()));
  // FIXME: remove this when the device configuration generator is ready
#if QMC2_WIP_CODE == 1
  s = tr("Generate device configurations");
  action = configurationMenu->addAction(tr("&Generate configurations for '%1'...").arg(messMachineName));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/configure.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(actionGenerateDeviceConfigurations_triggered()));
#endif
  pushButtonConfiguration->setMenu(configurationMenu);

  // device configuration list context menu
  deviceConfigurationListMenu = new QMenu(this);
  s = tr("Play selected game");
  action = deviceConfigurationListMenu->addAction(tr("&Play"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
  connect(action, SIGNAL(triggered()), qmc2MainWindow, SLOT(on_actionPlay_activated()));
#if defined(Q_WS_X11)
  s = tr("Play selected game (embedded)");
  action = deviceConfigurationListMenu->addAction(tr("Play &embedded"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
  connect(action, SIGNAL(triggered()), qmc2MainWindow, SLOT(on_actionPlayEmbedded_activated()));
#endif

  // device instance list context menu
  deviceContextMenu = new QMenu(this);
  s = tr("Select a file to be mapped to this device instance");
  action = deviceContextMenu->addAction(tr("Select file..."));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(actionSelectFile_triggered()));
}

MESSDeviceConfigurator::~MESSDeviceConfigurator()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::~MESSDeviceConfigurator()");
#endif

  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/DeviceSetupHeaderState", treeWidgetDeviceSetup->header()->saveState());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MESSDeviceConfigurator/vSplitter", QSize(vSplitter->sizes().at(0), vSplitter->sizes().at(1)));
}

QString &MESSDeviceConfigurator::getXmlData(QString machineName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::getXmlData(QString machineName = %1)").arg(machineName));
#endif

  static QString xmlBuffer;

  xmlBuffer = messXmlDataCache[machineName];

  if ( xmlBuffer.isEmpty() ) {
    int i = 0;
    QString s = "<machine name=\"" + machineName + "\"";
    while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
    xmlBuffer = "<?xml version=\"1.0\"?>\n";
    while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") )
      xmlBuffer += qmc2Gamelist->xmlLines[i++].simplified() + "\n";
    xmlBuffer += "</machine>\n";
    messXmlDataCache[machineName] = xmlBuffer;
  }

  return xmlBuffer;
}

bool MESSDeviceConfigurator::load()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::load()");
#endif

  QString xmlBuffer = getXmlData(messMachineName);
  
  QXmlInputSource xmlInputSource;
  xmlInputSource.setData(xmlBuffer);
  MESSDeviceConfiguratorXmlHandler xmlHandler(treeWidgetDeviceSetup);
  QXmlSimpleReader xmlReader;
  xmlReader.setContentHandler(&xmlHandler);
  xmlReader.parse(xmlInputSource);

  configurationMap.clear();

  qmc2Config->beginGroup(QString("MESS/Configuration/Devices/%1").arg(messMachineName));
  QString selectedConfiguration = qmc2Config->value("SelectedConfiguration").toString();
  QStringList configurationList = qmc2Config->childGroups();

  foreach (QString configName, configurationList) {
    configurationMap[configName].first = qmc2Config->value(QString("%1/Instances").arg(configName)).toStringList();
    configurationMap[configName].second = qmc2Config->value(QString("%1/Files").arg(configName)).toStringList();
    QListWidgetItem *item = new QListWidgetItem(configName, listWidgetDeviceConfigurations);
    if ( selectedConfiguration == configName ) listWidgetDeviceConfigurations->setCurrentItem(item);
  }

  qmc2FileEditStartPath = qmc2Config->value("DefaultDeviceDirectory").toString();

  qmc2Config->endGroup();

  // use the 'general software folder' as fall-back, if applicable
  if ( qmc2FileEditStartPath.isEmpty() ) {
    qmc2FileEditStartPath = qmc2Config->value("MESS/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
    QDir machineSoftwareFolder(qmc2FileEditStartPath + "/" + messMachineName);
    if ( machineSoftwareFolder.exists() )
      qmc2FileEditStartPath = machineSoftwareFolder.canonicalPath();
  }

  dontIgnoreNameChange = true;
  QListWidgetItem *noDeviceItem = new QListWidgetItem(tr("No devices"), listWidgetDeviceConfigurations);
  if ( listWidgetDeviceConfigurations->currentItem() == NULL )
    listWidgetDeviceConfigurations->setCurrentItem(noDeviceItem);
  dontIgnoreNameChange = false;

  return true;
}

bool MESSDeviceConfigurator::save()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::save()");
#endif

  QString group = QString("MESS/Configuration/Devices/%1").arg(messMachineName);
  QString devDir = qmc2Config->value(QString("%1/DefaultDeviceDirectory").arg(group), "").toString();

  qmc2Config->remove(group);
  qmc2Config->beginGroup(group);

  if ( configurationMap.count() > 0 ) {
    foreach (QString configName, configurationMap.keys()) {
      QPair<QStringList, QStringList> config = configurationMap[configName];
      qmc2Config->setValue(QString("%1/Instances").arg(configName), config.first);
      qmc2Config->setValue(QString("%1/Files").arg(configName), config.second);
    }
  }

  if ( !devDir.isEmpty() )
    qmc2Config->setValue("DefaultDeviceDirectory", devDir);

  QListWidgetItem *curItem = listWidgetDeviceConfigurations->currentItem();
  if ( curItem != NULL ) {
    if ( curItem->text() == tr("No devices") )
      qmc2Config->remove("SelectedConfiguration");
    else
      qmc2Config->setValue("SelectedConfiguration", curItem->text());
  }

  qmc2Config->endGroup();

  return true;
}

void MESSDeviceConfigurator::on_pushButtonNewConfiguration_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_pushButtonNewConfiguration_clicked()");
#endif

  dontIgnoreNameChange = true;
  lineEditConfigurationName->clear();
  pushButtonCloneConfiguration->setEnabled(false);
  pushButtonSaveConfiguration->setEnabled(false);
  pushButtonRemoveConfiguration->setEnabled(false);
  treeWidgetDeviceSetup->setEnabled(true);
  lineEditConfigurationName->setFocus();
}

void MESSDeviceConfigurator::on_pushButtonCloneConfiguration_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_pushButtonCloneConfiguration_clicked()");
#endif

  // create a clone of an existing device configuration
  QString sourceName = lineEditConfigurationName->text();
  int copies = 1;
  QString targetName = tr("%1. copy of ").arg(copies) + sourceName;
  while ( configurationMap.contains(targetName) )
    targetName = tr("%1. copy of ").arg(++copies) + sourceName;

  configurationMap.insert(targetName, configurationMap[sourceName]);
  listWidgetDeviceConfigurations->insertItem(listWidgetDeviceConfigurations->count(), targetName);
  
  dontIgnoreNameChange = true;
  lineEditConfigurationName->setText(targetName);
}

void MESSDeviceConfigurator::on_pushButtonSaveConfiguration_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_pushButtonSaveConfiguration_clicked()");
#endif

  QString cfgName = lineEditConfigurationName->text();

  if ( cfgName.isEmpty() )
    return;

  QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(cfgName, Qt::MatchExactly);
  if ( matchedItemList.count() > 0 ) {
    // save existing device configuration
    QList<QTreeWidgetItem *> allItems = treeWidgetDeviceSetup->findItems("*", Qt::MatchWildcard);
    QStringList instances, files;
    foreach (QTreeWidgetItem *item, allItems) {
      QString fileName = item->data(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole).toString();
      if ( !fileName.isEmpty() ) {
        instances << item->data(QMC2_DEVCONFIG_COLUMN_NAME, Qt::EditRole).toString();
        files << fileName;
      }
    }
    configurationMap[cfgName].first = instances;
    configurationMap[cfgName].second = files;
  } else {
    // add new device configuration
    listWidgetDeviceConfigurations->insertItem(listWidgetDeviceConfigurations->count(), cfgName);
    dontIgnoreNameChange = true;
    on_pushButtonSaveConfiguration_clicked();
  }

  on_lineEditConfigurationName_textChanged(lineEditConfigurationName->text());
}

void MESSDeviceConfigurator::on_pushButtonRemoveConfiguration_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::on_pushButtonRemoveConfiguration_clicked()");
#endif

  QString cfgName = lineEditConfigurationName->text();

  QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(cfgName, Qt::MatchExactly);
  if ( matchedItemList.count() > 0 ) {
    // remove existing device configuration
    configurationMap.remove(cfgName);
    int row = listWidgetDeviceConfigurations->row(matchedItemList[0]);
    QListWidgetItem *prevItem = NULL;
    if ( row > 0 )
      prevItem = listWidgetDeviceConfigurations->item(row - 1);
    QListWidgetItem *item = listWidgetDeviceConfigurations->takeItem(row);
    delete item;
    if ( prevItem )
      listWidgetDeviceConfigurations->setCurrentItem(prevItem);
  }
}

void MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_itemActivated(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_itemActivated(QListWidgetItem *item = %1)").arg((qulonglong) item));
#endif

  QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_activated()));
}

void MESSDeviceConfigurator::on_lineEditConfigurationName_textChanged(const QString &text)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_lineEditConfigurationName_textChanged(const QString &text = %1)").arg(text));
#endif

  pushButtonSaveConfiguration->setEnabled(false);
  if ( text == tr("No devices") ) {
    pushButtonCloneConfiguration->setEnabled(false);
    pushButtonSaveConfiguration->setEnabled(false);
    pushButtonRemoveConfiguration->setEnabled(false);
    treeWidgetDeviceSetup->setEnabled(false);
  } else if ( !text.isEmpty() ) {
    QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(text, Qt::MatchExactly);
    if ( matchedItemList.count() > 0 ) {
      pushButtonRemoveConfiguration->setEnabled(true);
      pushButtonSaveConfiguration->setEnabled(true);
      pushButtonCloneConfiguration->setEnabled(true);
    } else {
      pushButtonRemoveConfiguration->setEnabled(false);
      pushButtonSaveConfiguration->setEnabled(true);
      pushButtonCloneConfiguration->setEnabled(false);
    }
    treeWidgetDeviceSetup->setEnabled(true);
  } else {
    pushButtonCloneConfiguration->setEnabled(false);
    pushButtonSaveConfiguration->setEnabled(false);
    pushButtonRemoveConfiguration->setEnabled(false);
    treeWidgetDeviceSetup->setEnabled(true);
  }

  if ( dontIgnoreNameChange ) {
    QList<QTreeWidgetItem *> setupItemList = treeWidgetDeviceSetup->findItems("*", Qt::MatchWildcard);
    foreach (QTreeWidgetItem *setupItem, setupItemList)
      setupItem->setData(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole, QString());

    QList<QListWidgetItem *> matchedItemList = listWidgetDeviceConfigurations->findItems(text, Qt::MatchExactly);
    if ( matchedItemList.count() > 0 ) {
      matchedItemList[0]->setSelected(true);
      listWidgetDeviceConfigurations->setCurrentItem(matchedItemList[0]);
      listWidgetDeviceConfigurations->scrollToItem(matchedItemList[0]);
      QString configName = matchedItemList[0]->text();
      if ( configurationMap.contains(configName) ) {
        QPair<QStringList, QStringList> valuePair = configurationMap[configName];
        int i;
        for (i = 0; i < valuePair.first.count(); i++) {
          QList<QTreeWidgetItem *> itemList = treeWidgetDeviceSetup->findItems(valuePair.first[i], Qt::MatchExactly);
          if ( itemList.count() > 0 )
            itemList[0]->setData(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole, valuePair.second[i]);
        }
      }
    } else {
      listWidgetDeviceConfigurations->clearSelection();
      pushButtonRemoveConfiguration->setEnabled(false);
      pushButtonCloneConfiguration->setEnabled(false);
    }
  }
  dontIgnoreNameChange = false;
}

void MESSDeviceConfigurator::editorDataChanged(const QString &text)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::editorDataChanged(const QString &text = %1)").arg(text));
#endif

  if ( lineEditConfigurationName->text().isEmpty() && !text.isEmpty() ) {
    int copies = 0;
    QString sourceName = text;
    QFileInfo fi(sourceName);
    sourceName = fi.completeBaseName();
    QString targetName = sourceName;
    while ( configurationMap.contains(targetName) )
      targetName = tr("%1. variant of ").arg(++copies) + sourceName;
    lineEditConfigurationName->setText(targetName);
  }
}

void MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_currentTextChanged(const QString &text)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_currentTextChanged(const QString &text = %1)").arg(text));
#endif

  dontIgnoreNameChange = true;
  lineEditConfigurationName->setText(text);
  dontIgnoreNameChange = false;
}

void MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_itemClicked(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_itemClicked(QListWidgetItem * = %1)").arg((qulonglong)item));
#endif

  dontIgnoreNameChange = true;
  lineEditConfigurationName->setText(item->text());
  dontIgnoreNameChange = false;
}

void MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_customContextMenuRequested(const QPoint &point)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_listWidgetDeviceConfigurations_customContextMenuRequested(const QPoint &point = (%1, %2))").arg(point.x()).arg(point.y()));
#endif

  QListWidgetItem *item = listWidgetDeviceConfigurations->itemAt(point);
  if ( item ) {
    listWidgetDeviceConfigurations->setCurrentItem(item);
    listWidgetDeviceConfigurations->setItemSelected(item, true);
    deviceConfigurationListMenu->move(listWidgetDeviceConfigurations->viewport()->mapToGlobal(point));
    deviceConfigurationListMenu->show();
  }
}

void MESSDeviceConfigurator::on_treeWidgetDeviceSetup_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::on_treeWidgetDeviceSetup_customContextMenuRequested(const QPoint &p = [%1, %2])").arg(p.x()).arg(p.y()));
#endif

	if ( treeWidgetDeviceSetup->itemAt(p) ) {
		deviceContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetDeviceSetup->viewport()->mapToGlobal(p), deviceContextMenu));
		deviceContextMenu->show();
	}
}

void MESSDeviceConfigurator::actionSelectFile_triggered()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::actionSelectFile_triggered()");
#endif

	QTreeWidgetItem *item = treeWidgetDeviceSetup->currentItem();
	if ( item ) {
		int row = treeWidgetDeviceSetup->indexOfTopLevelItem(item);
		if ( row >= 0 ) {
			FileEditWidget *few = messFileEditWidgetList[row];
			if ( few )
				QTimer::singleShot(0, few, SLOT(on_toolButtonBrowse_clicked()));
		}
	}
}

void MESSDeviceConfigurator::actionSelectDefaultDeviceDirectory_triggered()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::actionSelectDefaultDeviceDirectory_triggered()");
#endif

  QString group = QString("MESS/Configuration/Devices/%1").arg(messMachineName);
  QString path = qmc2Config->value(group + "/DefaultDeviceDirectory", "").toString();

  if ( path.isEmpty() ) {
    path = qmc2Config->value("MESS/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
    QDir machineSoftwareFolder(path + "/" + messMachineName);
    if ( machineSoftwareFolder.exists() )
      path = machineSoftwareFolder.canonicalPath();
  }

  qmc2Config->beginGroup(group);

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose default device directory for '%1'").arg(messMachineName), path, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if ( !s.isEmpty() )
    qmc2Config->setValue("DefaultDeviceDirectory", s);
  qmc2FileEditStartPath = qmc2Config->value("DefaultDeviceDirectory").toString();

  qmc2Config->endGroup();

  if ( qmc2FileEditStartPath.isEmpty() ) {
    qmc2FileEditStartPath = qmc2Config->value("MESS/FilesAndDirectories/GeneralSoftwareFolder", ".").toString();
    QDir machineSoftwareFolder(qmc2FileEditStartPath + "/" + messMachineName);
    if ( machineSoftwareFolder.exists() )
      qmc2FileEditStartPath = machineSoftwareFolder.canonicalPath();
  }
}

void MESSDeviceConfigurator::actionGenerateDeviceConfigurations_triggered()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfigurator::actionGenerateDeviceConfigurations_triggered()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("FIXME: sorry, the MESS device configuration generator isn't working yet"));
}

void MESSDeviceConfigurator::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

  if ( e )
    e->accept();
}

void MESSDeviceConfigurator::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

  save();

  if ( e )
    e->accept();
}

void MESSDeviceConfigurator::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfigurator::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

  if ( e )
    e->accept();
}

MESSDeviceConfiguratorXmlHandler::MESSDeviceConfiguratorXmlHandler(QTreeWidget *parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfiguratorXmlHandler::MESSDeviceConfiguratorXmlHandler(QTreeWidget *parent = %1)").arg((qulonglong) parent));
#endif

  parentTreeWidget = parent;
}

MESSDeviceConfiguratorXmlHandler::~MESSDeviceConfiguratorXmlHandler()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfiguratorXmlHandler::~MESSDeviceConfiguratorXmlHandler()");
#endif

}

bool MESSDeviceConfiguratorXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfiguratorXmlHandler::startElement(...)");
#endif

  if ( qName == "device" ) {
    deviceType = attributes.value("type");
    deviceTag = attributes.value("tag");
    deviceInstances.clear();
    deviceExtensions.clear();
    deviceBriefName.clear();
  } else if ( qName == "instance" ) {
    deviceInstances << attributes.value("name");
    deviceBriefName = attributes.value("briefname");
  } else if ( qName == "extension" ) {
    deviceExtensions << attributes.value("name");
  }

  return true;
}

bool MESSDeviceConfiguratorXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MESSDeviceConfiguratorXmlHandler::endElement(...)");
#endif

  if ( qName == "device" ) {
    foreach (QString instance, deviceInstances) {
      QTreeWidgetItem *deviceItem = new QTreeWidgetItem(parentTreeWidget);
      deviceItem->setText(QMC2_DEVCONFIG_COLUMN_NAME, instance);
      deviceItem->setText(QMC2_DEVCONFIG_COLUMN_BRIEF, deviceBriefName);
      deviceItem->setText(QMC2_DEVCONFIG_COLUMN_TYPE, deviceType);
      deviceItem->setText(QMC2_DEVCONFIG_COLUMN_TAG, deviceTag);
      deviceItem->setText(QMC2_DEVCONFIG_COLUMN_EXT, deviceExtensions.join("/"));
      parentTreeWidget->openPersistentEditor(deviceItem, QMC2_DEVCONFIG_COLUMN_FILE);
      deviceItem->setData(QMC2_DEVCONFIG_COLUMN_FILE, Qt::EditRole, QString());
    }
  }

  return true;
}

bool MESSDeviceConfiguratorXmlHandler::characters(const QString &str)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MESSDeviceConfiguratorXmlHandler::characters(const QString &str = %1)").arg(str));
#endif

  return true;
}
