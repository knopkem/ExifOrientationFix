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
#include <QImageReader>
#include <QImageWriter>

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

bool Mover::performOperations(const QString &source, const QString &target, bool traverseSubdirectories, bool fixOrientation, int maxSize)
{
    d->sourcePath = source;

    // make sure the target folder exists
    if (!makedir(target) ) {
        return false;
    }

    d->currentTarget = target;
	QString logs = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/exif_logs";
    makedir(logs);
    QString logFilepath = logs + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt";
    SimpleLog::startFileLogging(logFilepath);

    QElapsedTimer elapsed;
    elapsed.start();

    // query all files in the directory
    QStringList files = findAllFilesInDirectory(source, traverseSubdirectories);

    QProgressDialog progress(tr("Processing files..."), tr("Cancel"), 0, 0);
    progress.setModal(true);
    progress.setRange(0, d->filesTotal);

    int i = 0;
    bool result = true;
    foreach(const QString & file, files) {

        if (progress.wasCanceled()) {
            break;
        }

        QImageReader reader(file);
        reader.setAutoTransform(false);
        if (reader.canRead()) {
            QImage image = reader.read();
            QFlags<QImageIOHandler::Transformation> tr = reader.transformation();
            int degree = 0;
            bool horizontally = false;
            bool vertically = false;

            if (fixOrientation) {
                switch (tr)
                {
                case QImageIOHandler::TransformationNone:
                    break;
                case QImageIOHandler::TransformationMirror:
                    vertically = true;
                    break;
                case QImageIOHandler::TransformationFlip:
                    horizontally = true;
                    break;
                case QImageIOHandler::TransformationRotate180:
                    degree = 180;
                    break;
                case QImageIOHandler::TransformationRotate90:
                    degree = 90;
                    break;
                case QImageIOHandler::TransformationMirrorAndRotate90:
                    vertically = true;
                    degree = 90;
                    break;
                case QImageIOHandler::TransformationFlipAndRotate90:
                    horizontally = true;
                    degree = 90;
                    break;
                case QImageIOHandler::TransformationRotate270:
                    degree = 270;
                    break;
                }
            }

            QMatrix mat;
            mat.rotate(degree);
            QImage flippedImage = image.mirrored(horizontally, vertically);
            QImage rotatedImage = flippedImage.transformed(mat);
            QImage scaledImage = rotatedImage;

            if (maxSize > 0) {
                if (rotatedImage.height() > maxSize) {
                    scaledImage = rotatedImage.scaledToHeight(maxSize);
                }
                else if (rotatedImage.width() > maxSize) {
                    scaledImage = rotatedImage.scaledToWidth(maxSize);
                }
            }

            QFileInfo info(file);
            QString targetFileName = target + "/" + info.fileName();

            QFile f(targetFileName);
            if (f.exists()) {
                targetFileName = target + "/" + QUuid::createUuid().toString() + ".jpg";
            }

            QImageWriter writer(targetFileName);
            writer.setTransformation(QImageIOHandler::TransformationNone);
            if (!writer.write(scaledImage)) {
                result = false;
            }
            progress.setValue(i++);
        }
    }
    progress.hide();

    return result;
}

//--------------------------------------------------------------------------------------

QStringList Mover::findAllFilesInDirectory( const QString& path, bool traverseSubdir )
{
    d->filesTotal = 0;
    QStringList result;

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


