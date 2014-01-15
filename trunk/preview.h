#ifndef _PREVIEW_H_
#define _PREVIEW_H_

#include "imagewidget.h"

class Preview : public ImageWidget
{
	Q_OBJECT 

	public:
		Preview(QWidget *parent);

		virtual QString cachePrefix() { return "prv"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("preview"); }
		virtual int imageTypeNumeric() { return QMC2_IMGTYPE_PREVIEW; }
		virtual bool useZip();
		virtual bool useSevenZip();
		virtual bool scaledImage();
};

#endif
