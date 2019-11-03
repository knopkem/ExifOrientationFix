#ifndef mover_h__
#define mover_h__

/*
 * Copyright (C) 2014 Michael Knopke
 *
 * See the COPYRIGHT file for terms of use.
 */

#include <QObject>
#include <QList>

class MoverPrivate;
class Mover : public QObject
{
    Q_OBJECT
public:
    Mover(QObject* parent= NULL);
    ~Mover();

    bool performOperations(const QString& source, const QString& target, bool traverseSubdirectories, int maxSize);

    static bool makedir(const QString& path);

    static bool dirExists(const QString& path);
protected:

    QStringList findAllFilesInDirectory(const QString& path, bool traverseSubdir);

private:
    MoverPrivate* d;
};
#endif // mover_h__
