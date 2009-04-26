#include "itemselect.h"
#include "macros.h"

#ifdef QMC2_DEBUG
#include "qmc2main.h"
extern MainWindow *qmc2MainWindow;
#endif

ItemSelector::ItemSelector(QWidget *parent, QStringList &items)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ItemSelector::ItemSelector(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ", ...)");
#endif

  setupUi(this);

  listWidgetItems->addItems(items);
}

ItemSelector::~ItemSelector()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ItemSelector::~ItemSelector()");
#endif

}
