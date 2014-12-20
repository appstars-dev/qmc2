#ifndef _PCB_H_
#define _PCB_H_

#include "imagewidget.h"

class PCB : public ImageWidget
{
	Q_OBJECT 

	public:
		PCB(QWidget *parent);

		virtual QString cachePrefix() { return "pcb"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("PCB"); }
		virtual int imageTypeNumeric() { return QMC2_IMGTYPE_PCB; }
		virtual bool useZip();
		virtual bool useSevenZip();
		virtual bool scaledImage();
};

#endif