#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QtScript>
#include <QtScriptTools>
#include <QObject>
#include <QStringList>
#include <QMap>
#include <QDir>
#include <QDirIterator>

#include "projectwidget.h"
#include "scriptwidget.h"
#include "macros.h"

class ScriptWidget;
class ProjectWidget;

class ScriptEngine : public QObject
{
    Q_OBJECT

public:
    bool externalStop;

    explicit ScriptEngine(ScriptWidget *parent);
    virtual ~ScriptEngine();

    void runScript(QString script);
    void stopScript();

public slots:
    // print a message to the script-log
    void log(QString message);

    // find files in folders
    void dirStartEntryList(QString path, QString filter, bool subDirs = false);
    bool dirHasNextEntry();
    QString dirNextEntry();
    QStringList dirEntryList(QString path, QString filter = QString(), bool sort = true, bool ascending = true);
    QStringList dirSubDirList(QString path, QString filter = QString(), bool sort = true, bool ascending = true);

    // dump hard-disk templates
    void dumpHardDiskTemplates();

    // run shell commands
    int runShellCommand(QString command, bool detached = false);

    // retrieve user input
    QString inputGetFilePath(QString initialPath = QString(), QString filter = QString(), QString windowTitle = QString());
    QString inputGetFolderPath(QString initialPath = QString(), QString windowTitle = QString());
    QString inputGetStringValue(QString initialValue = QString(), QString windowTitle = QString(), QString labelText = QString());
    QString inputGetListItem(QString initialValue = QString(), QStringList itemList = QStringList(), bool editable = false, QString windowTitle = QString(), QString labelText = QString());
    int inputGetIntValue(int initialValue = 0, QString windowTitle = QString(), QString labelText = QString());
    double inputGetDoubleValue(double initialValue = 0.0, int decimals = 1, QString windowTitle = QString(), QString labelText = QString());
    bool inputOk() { return mInputOk; }

    // control the progress-bar
    void progressSetRange(int min, int max);
    void progressSetValue(int value);

    // project creation / destruction / status / return code
    void projectCreate(QString id, QString type);
    void projectCreateFromFile(QString id, QString fileName);
    void projectCreateFromString(QString id, QString buffer);
    void projectClone(QString sourceId, QString destinationId);
    void projectDestroy(QString id);
    QString projectStatus(QString id);
    int projectReturnCode(QString id);
    void projectSetType(QString id, QString type);
    QString projectGetType(QString id);
    void projectToWindow(QString id);
    void projectCloneToWindow(QString sourceId);

    // properties for project-type 'Info'
    void projectSetInfoInputFile(QString id, QString file);
    QString projectGetInfoInputFile(QString id);
    void projectSetInfoVerbose(QString id, bool verbose);
    bool projectGetInfoVerbose(QString id);

    // properties for project-type 'Verify'
    void projectSetVerifyInputFile(QString id, QString file);
    QString projectGetVerifyInputFile(QString id);
    void projectSetVerifyParentInputFile(QString id, QString file);
    QString projectGetVerifyParentInputFile(QString id);

    // properties for project-type 'Copy'
    void projectSetCopyInputFile(QString id, QString file);
    QString projectGetCopyInputFile(QString id);
    void projectSetCopyParentInputFile(QString id, QString file);
    QString projectGetCopyParentInputFile(QString id);
    void projectSetCopyOutputFile(QString id, QString file);
    QString projectGetCopyOutputFile(QString id);
    void projectSetCopyParentOutputFile(QString id, QString file);
    QString projectGetCopyParentOutputFile(QString id);
    void projectSetCopyForce(QString id, bool force);
    bool projectGetCopyForce(QString id);
    void projectSetCopyInputStartByte(QString id, int byte);
    int projectGetCopyInputStartByte(QString id);
    void projectSetCopyInputStartHunk(QString id, int hunk);
    int projectGetCopyInputStartHunk(QString id);
    void projectSetCopyInputBytes(QString id, int bytes);
    int projectGetCopyInputBytes(QString id);
    void projectSetCopyInputHunks(QString id, int hunks);
    int projectGetCopyInputHunks(QString id);
    void projectSetCopyHunkSize(QString id, int size);
    int projectGetCopyHunkSize(QString id);
    void projectSetCopyCompressors(QString id, QString compressors);
    QString projectGetCopyCompressors(QString id);
    void projectSetCopyProcessors(QString id, int processors);
    int projectGetCopyProcessors(QString id);

    // properties for project-type 'CreateRaw'
    void projectSetCreateRawInputFile(QString id, QString file);
    QString projectGetCreateRawInputFile(QString id);
    void projectSetCreateRawOutputFile(QString id, QString file);
    QString projectGetCreateRawOutputFile(QString id);
    void projectSetCreateRawParentOutputFile(QString id, QString file);
    QString projectGetCreateRawParentOutputFile(QString id);
    void projectSetCreateRawForce(QString id, bool force);
    bool projectGetCreateRawForce(QString id);
    void projectSetCreateRawInputStartByte(QString id, int byte);
    int projectGetCreateRawInputStartByte(QString id);
    void projectSetCreateRawInputStartHunk(QString id, int hunk);
    int projectGetCreateRawInputStartHunk(QString id);
    void projectSetCreateRawInputBytes(QString id, int bytes);
    int projectGetCreateRawInputBytes(QString id);
    void projectSetCreateRawInputHunks(QString id, int hunks);
    int projectGetCreateRawInputHunks(QString id);
    void projectSetCreateRawHunkSize(QString id, int size);
    int projectGetCreateRawHunkSize(QString id);
    void projectSetCreateRawUnitSize(QString id, int size);
    int projectGetCreateRawUnitSize(QString id);
    void projectSetCreateRawCompressors(QString id, QString compressors);
    QString projectGetCreateRawCompressors(QString id);
    void projectSetCreateRawProcessors(QString id, int processors);
    int projectGetCreateRawProcessors(QString id);

    // properties for project-type 'CreateHD'
    void projectSetCreateHDInputFile(QString id, QString file);
    QString projectGetCreateHDInputFile(QString id);
    void projectSetCreateHDOutputFile(QString id, QString file);
    QString projectGetCreateHDOutputFile(QString id);
    void projectSetCreateHDParentOutputFile(QString id, QString file);
    QString projectGetCreateHDParentOutputFile(QString id);
    void projectSetCreateHDForce(QString id, bool force);
    bool projectGetCreateHDForce(QString id);
    void projectSetCreateHDInputStartByte(QString id, int byte);
    int projectGetCreateHDInputStartByte(QString id);
    void projectSetCreateHDInputStartHunk(QString id, int hunk);
    int projectGetCreateHDInputStartHunk(QString id);
    void projectSetCreateHDInputBytes(QString id, int bytes);
    int projectGetCreateHDInputBytes(QString id);
    void projectSetCreateHDInputHunks(QString id, int hunks);
    int projectGetCreateHDInputHunks(QString id);
    void projectSetCreateHDHunkSize(QString id, int size);
    int projectGetCreateHDHunkSize(QString id);
    void projectSetCreateHDCompressors(QString id, QString compressors);
    QString projectGetCreateHDCompressors(QString id);
    void projectSetCreateHDProcessors(QString id, int processors);
    int projectGetCreateHDProcessors(QString id);
    void projectSetCreateHDSectorSize(QString id, int sectorSize);
    int projectGetCreateHDSectorSize(QString id);
    void projectSetCreateHDCylinders(QString id, int cylinders);
    int projectGetCreateHDCylinders(QString id);
    void projectSetCreateHDHeads(QString id, int heads);
    int projectGetCreateHDHeads(QString id);
    void projectSetCreateHDSectors(QString id, int sectors);
    int projectGetCreateHDSectors(QString id);
    void projectSetCreateHDChsFromTemplate(QString id, QString vendorName, QString diskName);

    // properties for project-type 'CreateCD'
    void projectSetCreateCDInputFile(QString id, QString file);
    QString projectGetCreateCDInputFile(QString id);
    void projectSetCreateCDOutputFile(QString id, QString file);
    QString projectGetCreateCDOutputFile(QString id);
    void projectSetCreateCDParentOutputFile(QString id, QString file);
    QString projectGetCreateCDParentOutputFile(QString id);
    void projectSetCreateCDForce(QString id, bool force);
    bool projectGetCreateCDForce(QString id);
    void projectSetCreateCDHunkSize(QString id, int size);
    int projectGetCreateCDHunkSize(QString id);
    void projectSetCreateCDCompressors(QString id, QString compressors);
    QString projectGetCreateCDCompressors(QString id);
    void projectSetCreateCDProcessors(QString id, int processors);
    int projectGetCreateCDProcessors(QString id);

    // properties for project-type 'CreateLD'
    void projectSetCreateLDInputFile(QString id, QString file);
    QString projectGetCreateLDInputFile(QString id);
    void projectSetCreateLDOutputFile(QString id, QString file);
    QString projectGetCreateLDOutputFile(QString id);
    void projectSetCreateLDParentOutputFile(QString id, QString file);
    QString projectGetCreateLDParentOutputFile(QString id);
    void projectSetCreateLDForce(QString id, bool force);
    bool projectGetCreateLDForce(QString id);
    void projectSetCreateLDInputStartFrame(QString id, int frame);
    int projectGetCreateLDInputStartFrame(QString id);
    void projectSetCreateLDInputFrames(QString id, int frames);
    int projectGetCreateLDInputFrames(QString id);
    void projectSetCreateLDHunkSize(QString id, int size);
    int projectGetCreateLDHunkSize(QString id);
    void projectSetCreateLDCompressors(QString id, QString compressors);
    QString projectGetCreateLDCompressors(QString id);
    void projectSetCreateLDProcessors(QString id, int processors);
    int projectGetCreateLDProcessors(QString id);

    // properties for project-type 'ExtractRaw'
    void projectSetExtractRawInputFile(QString id, QString file);
    QString projectGetExtractRawInputFile(QString id);
    void projectSetExtractRawParentInputFile(QString id, QString file);
    QString projectGetExtractRawParentInputFile(QString id);
    void projectSetExtractRawOutputFile(QString id, QString file);
    QString projectGetExtractRawOutputFile(QString id);
    void projectSetExtractRawForce(QString id, bool force);
    bool projectGetExtractRawForce(QString id);
    void projectSetExtractRawInputStartByte(QString id, int byte);
    int projectGetExtractRawInputStartByte(QString id);
    void projectSetExtractRawInputStartHunk(QString id, int hunk);
    int projectGetExtractRawInputStartHunk(QString id);
    void projectSetExtractRawInputBytes(QString id, int bytes);
    int projectGetExtractRawInputBytes(QString id);
    void projectSetExtractRawInputHunks(QString id, int hunks);
    int projectGetExtractRawInputHunks(QString id);

    // properties for project-type 'ExtractHD'
    void projectSetExtractHDInputFile(QString id, QString file);
    QString projectGetExtractHDInputFile(QString id);
    void projectSetExtractHDParentInputFile(QString id, QString file);
    QString projectGetExtractHDParentInputFile(QString id);
    void projectSetExtractHDOutputFile(QString id, QString file);
    QString projectGetExtractHDOutputFile(QString id);
    void projectSetExtractHDForce(QString id, bool force);
    bool projectGetExtractHDForce(QString id);
    void projectSetExtractHDInputStartByte(QString id, int byte);
    int projectGetExtractHDInputStartByte(QString id);
    void projectSetExtractHDInputStartHunk(QString id, int hunk);
    int projectGetExtractHDInputStartHunk(QString id);
    void projectSetExtractHDInputBytes(QString id, int bytes);
    int projectGetExtractHDInputBytes(QString id);
    void projectSetExtractHDInputHunks(QString id, int hunks);
    int projectGetExtractHDInputHunks(QString id);

    // properties for project-type 'ExtractCD'

    // properties for project-type 'ExtractLD'

    // properties for project-type 'DumpMeta'

    // properties for project-type 'AddMeta'

    // properties for project-type 'DelMeta'

    // run / stop / synchronize / destroy projects
    void runProjects(QString idList = QString());
    void stopProjects(QString idList = QString());
    void syncProjects(QString idList = QString());
    void destroyProjects(QString idList = QString());

private:
    QScriptEngine *mEngine;
    QScriptEngineDebugger *mEngineDebugger;
    QMap<QString, ProjectWidget *> mProjectMap;
    ScriptWidget *mScriptWidget;
    QStringList mErrorStates;
    QDir mEntryListDir;
    QDirIterator *mEntryListIterator;
    bool mInputOk;
};

#endif // SCRIPTENGINE_H
