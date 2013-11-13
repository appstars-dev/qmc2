#include "settings.h"
#include "preview.h"
#include "macros.h"

// external global variables
extern Settings *qmc2Config;
extern bool qmc2UsePreviewFile;
extern bool qmc2ScaledPreview;

Preview::Preview(QWidget *parent)
	: ImageWidget(parent)
{
}

QString Preview::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PreviewFile").toString();
}

QString Preview::imageDir()
{
	return cleanDir(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PreviewDirectory").toString());
}

bool Preview::useZip()
{
	return qmc2UsePreviewFile;
}

bool Preview::scaledImage()
{
	return qmc2ScaledPreview;
}
