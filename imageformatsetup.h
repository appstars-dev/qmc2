#ifndef _IMAGEFORMATSETUP_H_
#define _IMAGEFORMATSETUP_H_

#include <QMap>
#include <QList>
#include <QStringList>

#include "ui_imageformatsetup.h"

class ImageFormatSetup : public QDialog, public Ui::ImageFormatSetup
{
	Q_OBJECT

       	public:
		ImageFormatSetup(QWidget *parent = 0);
		~ImageFormatSetup();

		static QStringList artworkClassPrefixes;
		static QStringList artworkClassNames;
		static QStringList artworkClassIcons;

	public slots:
		void adjustIconSizes();
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();
		void on_pushButtonRestore_clicked();
		void on_comboBoxImageType_currentIndexChanged(int);
		void on_treeWidget_itemClicked(QTreeWidgetItem *, int);

	protected:
		void showEvent(QShowEvent *);
		void resizeEvent(QResizeEvent *);
		void hideEvent(QHideEvent *);

	private:
		QMap<QString, QList<int> > mActiveFormats;
		int mPreviousClassIndex;
};

#endif
