#include <QSettings>

#include "controller.h"
#include "macros.h"

// external global variables
extern QSettings *qmc2Config;
extern bool qmc2UseControllerFile;
extern bool qmc2ScaledController;

Controller::Controller(QWidget *parent)
	: ImageWidget(parent)
{
	// NOP
}

QString Controller::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ControllerFile").toString();
}

QString Controller::imageDir()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ControllerDirectory").toString();
}

bool Controller::useZip()
{
	return qmc2UseControllerFile;
}

bool Controller::scaledImage()
{
	return qmc2ScaledController;
}
