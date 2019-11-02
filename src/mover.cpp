/*
 * Copyright (C) 2014 Michael Knopke
 *
 * See the COPYRIGHT file for terms of use.
 */

#include "mover.h"
#include "simpleLog.h"

#include <QtGui>
#include <QtWidgets>
#include <QtConcurrentMap>

class MoverPrivate
{
public:
    QString appPath;
    QString currentTarget;
    QString sourcePath;
    int filesTotal;
};

//--------------------------------------------------------------------------------------

Mover::Mover( QObject* parent/*= NULL*/ ) : QObject(parent), d(new MoverPrivate)
{
    d->appPath = QApplication::applicationDirPath();
#ifdef Q_WS_MAC
    d->appPath = QCoreApplication::applicationDirPath() + "/../../..";
#endif
}

//--------------------------------------------------------------------------------------

Mover::~Mover()
{
    delete d;
    d = NULL;
}

//--------------------------------------------------------------------------------------

bool Mover::performOperations(const QString &source, const QString &target, bool traverseSubdirectories)
{
    d->sourcePath = source;

    // make sure the target folder exists
    if (!makedir(target) ) {
        return false;
    }

    d->currentTarget = target;
	QString logs = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/yaps_logs";
    makedir(logs);
    QString logFilepath = logs + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt";
    SimpleLog::startFileLogging(logFilepath);

    QElapsedTimer elapsed;
    elapsed.start();

    // query all files in the directory
    QStringList files = findAllFilesInDirectory(source, traverseSubdirectories);

    return true;
}

//--------------------------------------------------------------------------------------

QStringList Mover::findAllFilesInDirectory( const QString& path, bool traverseSubdir )
{
    d->filesTotal = 0;
    QStringList result;
    qDebug() << "traversal started";

    QDir dir(path);
    dir.setFilter(QDir::Files);

    QProgressDialog progress(tr("Looking for files..."), tr("Cancel"), 0, 0);
    progress.setModal(true);

    if ( dir.exists() ) {
        bool stop = false;
        QDirIterator directory_walker(path, QDir::Files, QDirIterator::Subdirectories);
        int i=0;
        while( directory_walker.hasNext() && !stop) {
            if (progress.wasCanceled()) {
                break;
            }
            QString file = directory_walker.next();
            result << file;

            if (!traverseSubdir) {
                stop = true;
            }

            progress.setValue(i++);
            d->filesTotal++;
        }
    }

    qDebug() << "traversal finished";
    progress.hide();

    return result;
}

//--------------------------------------------------------------------------------------

bool Mover::dirExists( const QString& path )
{
    QDir dir(path);
    return dir.exists(path);
}

//--------------------------------------------------------------------------------------

bool Mover::makedir( const QString& path )
{
    QDir dir(path);

    if (dir.exists(path)){
        return true;
    }

    if (!dir.mkpath(path)) {
        qWarning() << "Could not create directory: " << path;
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------


