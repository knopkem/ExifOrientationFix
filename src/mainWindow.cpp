/*
 * Copyright (C) 2014 Michael Knopke
 *
 * See the COPYRIGHT file for terms of use.
 */

#include "mainWindow.h"
#include <version_config.h>

#include <QtGui>
#include <QtWidgets>


//--------------------------------------------------------------------------------------

class MainWindowPrivate {
public:
    QLineEdit*      sourceLineEdit;
    QPushButton*    sourceButton;
    QCheckBox*      traverseCheckBox;
    QCheckBox*      applyOrientationCheckBox;
    QCheckBox*      resizeCheckBox;
    QSpinBox*       maxSizeSpinBox;
    QLineEdit*      targetLineEdit;
    QPushButton*    targetButton;

    QLineEdit*      patternLineEdit;
    QLineEdit*      exampleLineEdit;

    QLabel*         actionLabel;

    QPushButton*    copyButton;

    Mover*          mover;

    QSettings*      settings;
};

//--------------------------------------------------------------------------------------

MainWindow::MainWindow() : d(new MainWindowPrivate)
{
    setWindowFlags( Qt::Dialog );
    this->setWindowTitle( tr("ExifOrientationFix - ") + VERSION_STRING );
    this->setMinimumWidth(800);

    d->settings = new QSettings(QApplication::organizationName(), QApplication::applicationName());

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(createIntroGroup());
    mainLayout->addWidget(createSourceGroup());
    mainLayout->addWidget(createTargetGroup());
    mainLayout->addWidget(createActionGroup());
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    // Create connections
    QObject::connect(d->sourceButton, SIGNAL(clicked()), this, SLOT(onSourceButtonClicked()));
    QObject::connect(d->targetButton, SIGNAL(clicked()), this, SLOT(onTargetButtonClicked()));

    QObject::connect(d->copyButton, SIGNAL(clicked()), this, SLOT(doCopy()));

    d->sourceButton->setFocus();

    d->mover = new Mover(this);

    readSettings();

 }

//--------------------------------------------------------------------------------------

MainWindow::~MainWindow( void )
{
    writeSettings();
    delete d->settings;
    delete d;
}

//--------------------------------------------------------------------------------------

QGroupBox* MainWindow::createIntroGroup()
{
    QGroupBox* introGroupBox = new QGroupBox(tr("Introduction"));
    QLabel* introText = new QLabel;
    introText->setText(tr("Welcome to the Exif Orientation Fix tool.\n"
    "\n"
    "This tools scans your input directory for exif files and applies rotation as described in the exif metadata directly to the images.\n"
    "This can be useful when using those images with tools that do not apply exif orientation (e.g. Ghost).\n"
    ));
    QVBoxLayout* vboxIntro = new QVBoxLayout;
    vboxIntro->addWidget(introText);

    introGroupBox->setLayout(vboxIntro);
    return introGroupBox;
}

//--------------------------------------------------------------------------------------

QGroupBox* MainWindow::createSourceGroup()
{
    QGroupBox* sourceGroupBox = new QGroupBox(tr("Source Folder"));
    d->sourceLineEdit = new QLineEdit;
    d->sourceLineEdit->setPlaceholderText(tr("Directory where your original images are stored"));

    d->sourceButton = new QPushButton(tr("Choose source folder"));
    d->sourceButton->setMaximumWidth(200);

    d->traverseCheckBox = new QCheckBox(tr("Include Subdirectories"));
    d->traverseCheckBox->setChecked(true);

    QHBoxLayout *hboxSource = new QHBoxLayout;
    hboxSource->addWidget(d->sourceButton);
    hboxSource->addWidget(d->traverseCheckBox);

    QVBoxLayout *vboxSource = new QVBoxLayout;
    vboxSource->addWidget(d->sourceLineEdit);
    vboxSource->addLayout(hboxSource);
    sourceGroupBox->setLayout(vboxSource);
    return sourceGroupBox;
}

//--------------------------------------------------------------------------------------

QGroupBox* MainWindow::createTargetGroup()
{
    QGroupBox* targetGroupBox = new QGroupBox(tr("Target Folder"));
    d->targetLineEdit = new QLineEdit;
    d->targetLineEdit->setPlaceholderText(tr("Directory where output will be stored"));

    d->targetButton = new QPushButton(tr("Choose target folder"));
    d->targetButton->setMaximumWidth(200);

    QHBoxLayout *hboxTarget = new QHBoxLayout;
    hboxTarget->addWidget(d->targetButton);
    hboxTarget->addStretch();

    QVBoxLayout *vboxTarget = new QVBoxLayout;
    vboxTarget->addWidget(d->targetLineEdit);
    vboxTarget->addLayout(hboxTarget);
    targetGroupBox->setLayout(vboxTarget);
    return targetGroupBox;
}

//--------------------------------------------------------------------------------------

QGroupBox* MainWindow::createActionGroup()
{
    QGroupBox* actionGroupBox = new QGroupBox(tr("Action"));
    d->applyOrientationCheckBox = new QCheckBox(tr("Apply rotation according to Exif orientation"));
    d->applyOrientationCheckBox->setChecked(true);
    d->resizeCheckBox = new QCheckBox("Resize images");
    d->resizeCheckBox->setChecked(true);
    connect(d->resizeCheckBox, &QCheckBox::stateChanged, this, &MainWindow::updateAction);
    d->maxSizeSpinBox = new QSpinBox();
    d->maxSizeSpinBox->setRange(64, 999999);
    d->maxSizeSpinBox->setValue(1280);

    d->copyButton = new QPushButton(tr("START"));

    QHBoxLayout* hbox1 = new QHBoxLayout;
    hbox1->addWidget(d->applyOrientationCheckBox);
    hbox1->addStretch();

    QHBoxLayout* hbox2 = new QHBoxLayout;
    hbox2->addWidget(d->resizeCheckBox);
    QLabel* label = new QLabel(tr("max width/height"));
    hbox2->addWidget(d->maxSizeSpinBox);
    hbox2->addWidget(label);
    hbox2->addStretch();

    d->actionLabel = new QLabel();
    d->actionLabel->setMinimumWidth(300);

    QHBoxLayout* hboxAction = new QHBoxLayout;
    hboxAction->addWidget(d->copyButton);
    hboxAction->addWidget(d->actionLabel);
    hboxAction->addStretch();

    QVBoxLayout* vboxAction = new QVBoxLayout;
    vboxAction->addLayout(hbox1);
    vboxAction->addLayout(hbox2);
    vboxAction->addLayout(hboxAction);

    actionGroupBox->setLayout(vboxAction);
    return actionGroupBox;
}

//--------------------------------------------------------------------------------------

void MainWindow::updateAction(int state)
{
    d->maxSizeSpinBox->setEnabled(state == Qt::Checked);
}

//--------------------------------------------------------------------------------------

void MainWindow::onSourceButtonClicked()
{
    d->sourceLineEdit->setText(openFileDialog());
}

//--------------------------------------------------------------------------------------

void MainWindow::onTargetButtonClicked()
{
    d->targetLineEdit->setText(openFileDialog());
}

//--------------------------------------------------------------------------------------

QString MainWindow::openFileDialog()
{
    QDir selDir;
    QFileDialog dialog(this);
    // dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (dialog.exec()) {
        QStringList selected = dialog.selectedFiles();
        if (!selected.empty())
            return (selected.first());
    }
    return "";
}

//--------------------------------------------------------------------------------------

void MainWindow::doCopy()
{
    if (!validateSelection()) {
        return;
    }

    int maxSize = -1;
    if (d->resizeCheckBox->isChecked()) {
        maxSize = d->maxSizeSpinBox->value();
    }
    d->actionLabel->setText("working...");
    bool result = d->mover->performOperations(d->sourceLineEdit->text(), d->targetLineEdit->text(), d->traverseCheckBox->isChecked(), d->applyOrientationCheckBox->isChecked(), maxSize);
    d->actionLabel->setText(result ? tr("Operation finished successfully") : tr("There where errors, check log file"));
}

//--------------------------------------------------------------------------------------

bool MainWindow::validateSelection()
{
    if ( !Mover::dirExists(d->sourceLineEdit->text())) {
        QMessageBox::warning(this, tr("Attention"),
            tr("Invalid source folder!"),
            QMessageBox::Ok);
        return false;
    }

    if ( !Mover::makedir(d->targetLineEdit->text()) ) {
        QMessageBox::warning(this, tr("Attention"),
            tr("Cannot create target folder!"),
            QMessageBox::Ok);
            return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------

void MainWindow::readSettings()
{
	QString inputPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
	QString outputPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/exif_output";

    d->sourceLineEdit->setText(d->settings->value("SourceFolder", inputPath).toString());
    d->targetLineEdit->setText(d->settings->value("TargetFolder", outputPath).toString());
}

//--------------------------------------------------------------------------------------

void MainWindow::writeSettings()
{
    d->settings->setValue("SourceFolder", d->sourceLineEdit->text());
    d->settings->setValue("TargetFolder", d->targetLineEdit->text());
    d->settings->sync();
}

//--------------------------------------------------------------------------------------



