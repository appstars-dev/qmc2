#ifndef _SWLISTEXPORT_H_
#define _SWLISTEXPORT_H_

#include <Qt>
#include "ui_swlistexport.h"

#define QMC2_SWLISTEXPORT_FORMAT_ASCII_INDEX		0
#define QMC2_SWLISTEXPORT_FORMAT_CSV_INDEX		1
#define QMC2_SWLISTEXPORT_FORMAT_SEP_INDEX		2
#define QMC2_SWLISTEXPORT_FORMAT_ALL_INDEX		3

class SoftwareListExporter : public QDialog, public Ui::SoftwareListExporter
{
	Q_OBJECT

	public:
		QStringList columnNames;
		QStringList columnNamesUntranslated;
		QStringList defaultColumnActivation;
		bool exportListAutoCorrected;

		SoftwareListExporter(QWidget *parent = 0);
		~SoftwareListExporter();

		void exportToASCII();
		void exportToCSV();

	public slots:
		void adjustIconSizes();

		// automatically connected slots
		void on_toolButtonBrowseASCIIFile_clicked();
    		void on_toolButtonBrowseCSVFile_clicked();
		void on_pushButtonExport_clicked();
		void on_comboBoxOutputFormat_currentIndexChanged(int);
		void on_checkBoxExportToClipboard_toggled(bool);

	protected:
		void closeEvent(QCloseEvent *);
		void hideEvent(QHideEvent *);
		void showEvent(QShowEvent *);
};

#endif
