#include <QSettings>

#include "pcb.h"
#include "macros.h"

// external global variables
extern QSettings *qmc2Config;
extern bool qmc2UsePCBFile;
extern bool qmc2ScaledPCB;

PCB::PCB(QWidget *parent)
	: ImageWidget(parent)
{
	QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ActiveImageFormats/pcb", QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts[i].toInt();
}

QString PCB::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PCBFile").toString();
}

QString PCB::imageDir()
{
	return cleanDir(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PCBDirectory").toString());
}

bool PCB::useZip()
{
	return qmc2UsePCBFile;
}

bool PCB::scaledImage()
{
	return qmc2ScaledPCB;
}
