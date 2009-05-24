#include <QSettings>

#include "detailsetup.h"
#include "macros.h"

#include "qmc2main.h"
extern MainWindow *qmc2MainWindow;

extern QSettings *qmc2Config;

DetailSetup::DetailSetup(QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DetailSetup::DetailSetup(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the game/machine detail setup isn't working yet!"));
#endif

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  shortTitleMap[QMC2_PREVIEW_INDEX] = tr("Previe&w");
  longTitleMap[QMC2_PREVIEW_INDEX] = tr("Game preview image");
  iconMap[QMC2_PREVIEW_INDEX] = QIcon(QString::fromUtf8(":/data/img/camera.png"));
  shortTitleMap[QMC2_FLYER_INDEX] = tr("Fl&yer");
  longTitleMap[QMC2_FLYER_INDEX] = tr("Game flyer image");
  iconMap[QMC2_FLYER_INDEX] = QIcon(QString::fromUtf8(":/data/img/thumbnail.png"));
  shortTitleMap[QMC2_GAMEINFO_INDEX] = tr("Game &info");
  longTitleMap[QMC2_GAMEINFO_INDEX] = tr("Game information");
  iconMap[QMC2_GAMEINFO_INDEX] = QIcon(QString::fromUtf8(":/data/img/info.png"));
  shortTitleMap[QMC2_EMUINFO_INDEX] = tr("Em&ulator info");
  longTitleMap[QMC2_EMUINFO_INDEX] = tr("Emulator information");
  iconMap[QMC2_EMUINFO_INDEX] = QIcon(QString::fromUtf8(":/data/img/info.png"));
  shortTitleMap[QMC2_CONFIG_INDEX] = tr("&Configuration");
  longTitleMap[QMC2_CONFIG_INDEX] = tr("Emulator configuration");
  iconMap[QMC2_CONFIG_INDEX] = QIcon(QString::fromUtf8(":/data/img/work.png"));
  shortTitleMap[QMC2_CABINET_INDEX] = tr("Ca&binet");
  longTitleMap[QMC2_CABINET_INDEX] = tr("Arcade cabinet image");
  iconMap[QMC2_CABINET_INDEX] = QIcon(QString::fromUtf8(":/data/img/arcadecabinet.png"));
  shortTitleMap[QMC2_CONTROLLER_INDEX] = tr("C&ontroller");
  longTitleMap[QMC2_CONTROLLER_INDEX] = tr("Control panel image");
  iconMap[QMC2_CONTROLLER_INDEX] = QIcon(QString::fromUtf8(":/data/img/joystick.png"));
  shortTitleMap[QMC2_MARQUEE_INDEX] = tr("&Marquee");
  longTitleMap[QMC2_MARQUEE_INDEX] = tr("Marquee image");
  iconMap[QMC2_MARQUEE_INDEX] = QIcon(QString::fromUtf8(":/data/img/marquee.png"));
  shortTitleMap[QMC2_TITLE_INDEX] = tr("Titl&e");
  longTitleMap[QMC2_TITLE_INDEX] = tr("Title screen image");
  iconMap[QMC2_TITLE_INDEX] = QIcon(QString::fromUtf8(":/data/img/arcademode.png"));
  availableDetailList << QMC2_PREVIEW_INDEX
                      << QMC2_FLYER_INDEX
                      << QMC2_GAMEINFO_INDEX
                      << QMC2_EMUINFO_INDEX
                      << QMC2_CONFIG_INDEX
                      << QMC2_CABINET_INDEX
                      << QMC2_CONTROLLER_INDEX
                      << QMC2_MARQUEE_INDEX
                      << QMC2_TITLE_INDEX;
  tabWidgetsMap[QMC2_PREVIEW_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PREVIEW_INDEX);
  tabWidgetsMap[QMC2_FLYER_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_FLYER_INDEX);
  tabWidgetsMap[QMC2_GAMEINFO_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_GAMEINFO_INDEX);
  tabWidgetsMap[QMC2_EMUINFO_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_EMUINFO_INDEX);
  tabWidgetsMap[QMC2_CONFIG_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONFIG_INDEX);
  tabWidgetsMap[QMC2_CABINET_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CABINET_INDEX);
  tabWidgetsMap[QMC2_CONTROLLER_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONTROLLER_INDEX);
  tabWidgetsMap[QMC2_MARQUEE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_MARQUEE_INDEX);
  tabWidgetsMap[QMC2_TITLE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_TITLE_INDEX);
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  shortTitleMap[QMC2_PREVIEW_INDEX] = tr("Previe&w");
  longTitleMap[QMC2_PREVIEW_INDEX] = tr("Machine preview image");
  iconMap[QMC2_PREVIEW_INDEX] = QIcon(QString::fromUtf8(":/data/img/camera.png"));
  shortTitleMap[QMC2_FLYER_INDEX] = tr("Fl&yer");
  longTitleMap[QMC2_FLYER_INDEX] = tr("Machine flyer image");
  iconMap[QMC2_FLYER_INDEX] = QIcon(QString::fromUtf8(":/data/img/thumbnail.png"));
  shortTitleMap[QMC2_MACHINEINFO_INDEX] = tr("Machine &info");
  longTitleMap[QMC2_MACHINEINFO_INDEX] = tr("Machine information");
  iconMap[QMC2_MACHINEINFO_INDEX] = QIcon(QString::fromUtf8(":/data/img/info.png"));
  shortTitleMap[QMC2_CONFIG_INDEX] = tr("&Configuration");
  longTitleMap[QMC2_CONFIG_INDEX] = tr("Emulator configuration");
  iconMap[QMC2_CONFIG_INDEX] = QIcon(QString::fromUtf8(":/data/img/work.png"));
  shortTitleMap[QMC2_DEVICE_INDEX] = tr("De&vices");
  longTitleMap[QMC2_DEVICE_INDEX] = tr("Device configuration");
  iconMap[QMC2_DEVICE_INDEX] = QIcon(QString::fromUtf8(":/data/img/tape.png"));
  availableDetailList << QMC2_PREVIEW_INDEX
                      << QMC2_FLYER_INDEX
                      << QMC2_MACHINEINFO_INDEX
                      << QMC2_CONFIG_INDEX
                      << QMC2_DEVICE_INDEX;
  tabWidgetsMap[QMC2_PREVIEW_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PREVIEW_INDEX);
  tabWidgetsMap[QMC2_FLYER_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_FLYER_INDEX);
  tabWidgetsMap[QMC2_MACHINEINFO_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_MACHINEINFO_INDEX);
  tabWidgetsMap[QMC2_CONFIG_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONFIG_INDEX);
  tabWidgetsMap[QMC2_DEVICE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_DEVICE_INDEX);
#endif

  setupUi(this);
  hide();

  QMapIterator<int, QString> it(longTitleMap);
  while ( it.hasNext() ) {
    it.next();
    listWidgetAvailableDetails->addItem(new QListWidgetItem(iconMap[it.key()], it.value()));
  }

  loadDetail();

  adjustSize();
}

DetailSetup::~DetailSetup()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::~DetailSetup()");
#endif

}

void DetailSetup::loadDetail()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::loadDetail()");
#endif

  activeDetailList.clear();
  if ( !qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ActiveDetails") ) {
    // use default detail list
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    activeDetailList << QMC2_PREVIEW_INDEX
                     << QMC2_FLYER_INDEX
                     << QMC2_GAMEINFO_INDEX
                     << QMC2_EMUINFO_INDEX
                     << QMC2_CONFIG_INDEX
                     << QMC2_CABINET_INDEX
                     << QMC2_CONTROLLER_INDEX
                     << QMC2_MARQUEE_INDEX
                     << QMC2_TITLE_INDEX;
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
    activeDetailList << QMC2_PREVIEW_INDEX
                     << QMC2_FLYER_INDEX
                     << QMC2_MACHINEINFO_INDEX
                     << QMC2_CONFIG_INDEX
                     << QMC2_DEVICE_INDEX;
#endif
  } else {
    QStringList activeIndexList = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ActiveDetails").toStringList();
    foreach (QString s, activeIndexList) {
      int i = s.toInt();
      if ( availableDetailList.contains(i) )
        activeDetailList << i;
    }
  }

  appliedDetailList = activeDetailList;

  listWidgetActiveDetails->clear();
  foreach (int i, activeDetailList)
    listWidgetActiveDetails->addItem(new QListWidgetItem(iconMap[i], longTitleMap[i]));
}

void DetailSetup::saveDetail()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::saveDetail()");
#endif

  appliedDetailList.clear();
  qmc2MainWindow->tabWidgetGameDetail->clear();
  QStringList activeIndexList;
  foreach (int i, activeDetailList)
    if ( availableDetailList.contains(i) ) {
      appliedDetailList << i;
      activeIndexList << QString::number(i);
      qmc2MainWindow->tabWidgetGameDetail->addTab(tabWidgetsMap[i], iconMap[i], shortTitleMap[i]);
    }
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ActiveDetails", activeIndexList);
}

void DetailSetup::on_listWidgetAvailableDetails_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_listWidgetAvailableDetails_itemSelectionChanged()");
#endif

  if ( listWidgetAvailableDetails->selectedItems().count() > 0 ) {
    pushButtonActivateDetails->setEnabled(TRUE);
  } else {
    pushButtonActivateDetails->setEnabled(FALSE);
  }
}
 
void DetailSetup::on_listWidgetActiveDetails_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_listWidgetActiveDetails_itemSelectionChanged()");
#endif

  if ( listWidgetActiveDetails->selectedItems().count() > 0 ) {
    pushButtonDeactivateDetails->setEnabled(TRUE);
    if ( listWidgetActiveDetails->selectedItems().count() == 1 ) {
      pushButtonDetailsUp->setEnabled(TRUE);
      pushButtonDetailsDown->setEnabled(TRUE);
    } else {
      pushButtonDetailsUp->setEnabled(FALSE);
      pushButtonDetailsDown->setEnabled(FALSE);
    }
  } else {
    pushButtonDeactivateDetails->setEnabled(FALSE);
    pushButtonDetailsUp->setEnabled(FALSE);
    pushButtonDetailsDown->setEnabled(FALSE);
  }
}

void DetailSetup::on_pushButtonActivateDetails_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonActivateDetails_clicked()");
#endif

  foreach (QListWidgetItem *item, listWidgetAvailableDetails->selectedItems()) {
    if ( item ) {
      QList<QListWidgetItem *> il = listWidgetActiveDetails->findItems(item->text(), Qt::MatchExactly); 
      if ( il.count() == 0 ) {
        int pageIndex = longTitleMap.key(item->text());
        if ( availableDetailList.contains(pageIndex) ) {
          listWidgetActiveDetails->addItem(new QListWidgetItem(iconMap[pageIndex], item->text()));
          activeDetailList << pageIndex;
        }
      }
    }
  }
}

void DetailSetup::on_pushButtonDeactivateDetails_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonDeactivateDetails_clicked()");
#endif

  foreach (QListWidgetItem *item, listWidgetActiveDetails->selectedItems()) {
    if ( item ) {
      int row = listWidgetActiveDetails->row(item);
      QListWidgetItem *takenItem = listWidgetActiveDetails->takeItem(row);
      if ( takenItem )
        delete takenItem;
      if ( row >= 0 && row < activeDetailList.count() )
        activeDetailList.removeAt(row);
    }
  }
}

void DetailSetup::on_pushButtonDetailsUp_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonDetailsUp_clicked()");
#endif

  foreach (QListWidgetItem *item, listWidgetActiveDetails->selectedItems()) {
    if ( item ) {
      int row = listWidgetActiveDetails->row(item);
      if ( row > 0 ) {
        activeDetailList.move(row, row - 1);
        QListWidgetItem *takenItem = listWidgetActiveDetails->takeItem(row);
        if ( takenItem ) {
          listWidgetActiveDetails->insertItem(row - 1, takenItem);
          takenItem->setSelected(TRUE);
        }
      }
    }
  }
}

void DetailSetup::on_pushButtonDetailsDown_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonDetailsDown_clicked()");
#endif

  foreach (QListWidgetItem *item, listWidgetActiveDetails->selectedItems()) {
    if ( item ) {
      int row = listWidgetActiveDetails->row(item);
      if ( row < listWidgetActiveDetails->count() - 1 ) {
        activeDetailList.move(row, row + 1);
        QListWidgetItem *takenItem = listWidgetActiveDetails->takeItem(row);
        if ( takenItem ) {
          listWidgetActiveDetails->insertItem(row + 1, takenItem);
          takenItem->setSelected(TRUE);
        }
      }
    }
  }
}

void DetailSetup::on_pushButtonOk_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonOk_clicked()");
#endif

  saveDetail();
}

void DetailSetup::on_pushButtonApply_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonApply_clicked()");
#endif

  saveDetail();
}

void DetailSetup::on_pushButtonCancel_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonCancel_clicked()");
#endif

  loadDetail();
}
