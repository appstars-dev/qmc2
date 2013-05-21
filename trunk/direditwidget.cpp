#include <QApplication>
#include <QFileDialog>
#include "direditwidget.h"

#include "qmc2main.h"
#include "options.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QString qmc2DirectoryEditStartPath;
extern Options *qmc2Options;

DirectoryEditWidget::DirectoryEditWidget(QString dirPath, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DirectoryEditWidget::DirectoryEditWidget(QString filePath = %1, QWidget *parent = %2)").arg(dirPath).arg((qulonglong) parent));
#endif

	setupUi(this);

	lineEditDirectory->setText(dirPath);
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonBrowse->setIconSize(iconSize);
}

DirectoryEditWidget::~DirectoryEditWidget()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DirectoryEditWidget::~DirectoryEditWidget()");
#endif

}

void DirectoryEditWidget::on_toolButtonBrowse_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DirectoryEditWidget::on_toolButtonBrowse_clicked()");
#endif

	QString startPath = lineEditDirectory->text();
	if ( startPath.isEmpty() )
		startPath = qmc2DirectoryEditStartPath;
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose directory"), startPath, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isEmpty() )
		lineEditDirectory->setText(s);
}
