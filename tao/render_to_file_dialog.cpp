// ****************************************************************************
//  render_to_file_dialog.cpp                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//    Implementation of RenderToFileDialog
//
//
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "render_to_file_dialog.h"
#include "application.h"
#include "widget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QCompleter>
#include <QSettings>

namespace Tao {

RenderToFileDialog::RenderToFileDialog(Widget *widget, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "clone" dialog
// ----------------------------------------------------------------------------
    : QDialog(parent),
      folder(QDir::toNativeSeparators(Application::defaultProjectFolderPath() +
                                    tr("/frames"))),
      fileName("frame%0d.png"),
      x(640), y(480), start(-1.0), duration(10.0), fps(25), startPage(0),
      timeOffset(0.0),
      widget(widget),
      okToDismiss(false), okButton(NULL), rendering(false)
{
    setupUi(this);
    okButton = buttonBox->button(QDialogButtonBox::Ok);
    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->setValue(0);
    firstFrameEdit->setText("1");

    // Restore previous settings

    QSettings settings;
    settings.beginGroup("renderToFiles");
    folder = settings.value("folder", folder).toString();
    fileName = settings.value("fileName", fileName).toString();
    x = settings.value("x", x).toInt();
    y = settings.value("y", y).toInt();
    start = settings.value("start2", start).toDouble();
    duration = settings.value("duration", duration).toDouble();
    fps = settings.value("fps", fps).toDouble();
    startPage = settings.value("startPage", startPage).toInt();
    timeOffset = settings.value("timeOffset", timeOffset).toDouble();
    settings.endGroup();

    // Fill values

    folderEdit->setText(folder);
    fileEdit->setText(fileName);
    QCompleter *pc = new QCompleter(TaoApp->pathCompletions(), this);
    folderEdit->setCompleter(pc);
    xResolution->setText(QString::number(x));
    yResolution->setText(QString::number(y));
    startTimeEdit->setText(QString::number(start));
    durationEdit->setText(QString::number(duration));
    fpsEdit->setText(QString::number(fps));
    startPageEdit->setText(QString::number(startPage));
    timeOffsetEdit->setText(QString::number(timeOffset));
}


void RenderToFileDialog::accept()
// ----------------------------------------------------------------------------
//    Ok/dismiss button was clicked
// ----------------------------------------------------------------------------
{
    if (okToDismiss)
        return QDialog::accept();

    // Check/create folder
    QString folder = folderEdit->text();
    QDir dir(folder);
    if (!dir.exists() && folder != "/dev/null")
    {
        int ret;
        ret = QMessageBox::question(this, tr("Folder does not exist"),
                                    tr("Do you want do create folder: %1?")
                                    .arg(folder),
                                    QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes)
            return;
        bool ok = dir.mkpath(folder);
        if (!ok)
        {
            QMessageBox::warning(this, tr("Error"),
                                 tr("Could not create folder."));
            return;
        }
    }

    // Read other values
    fileName = fileEdit->text();
    x = xResolution->text().toInt();
    y = yResolution->text().toInt();
    start = startTimeEdit->text().toDouble();
    duration = durationEdit->text().toDouble();
    fps = fpsEdit->text().toDouble();
    startPage = startPageEdit->text().toInt();
    timeOffset = timeOffsetEdit->text().toDouble();

    // Save values
    TaoApp->addPathCompletion(folder);
    QSettings settings;
    settings.beginGroup("renderToFiles");
    settings.setValue("folder", folder);
    settings.setValue("fileName", fileName);
    settings.setValue("x", x);
    settings.setValue("y", y);
    settings.setValue("start2", start);
    settings.setValue("end", duration);
    settings.setValue("fps", fps);
    settings.setValue("startPage", startPage);
    settings.setValue("timeOffset", timeOffset);
    settings.endGroup();

    okButton->setEnabled(false);
    setCursor(Qt::BusyCursor);
    connect(widget, SIGNAL(renderFramesProgress(int)),
            progressBar, SLOT(setValue(int)));
    connect(widget, SIGNAL(renderFramesDone()),
            this, SLOT(done()));

    rendering = true;
    widget->renderFrames(x, y, start, duration, folder, fps, startPage,
                         timeOffset, "", fileName,
                         firstFrameEdit->text().toInt());
}


void RenderToFileDialog::reject()
// ----------------------------------------------------------------------------
//    Cancel pressed
// ----------------------------------------------------------------------------
{
    if (!rendering)
        return QDialog::reject();
    widget->cancelRenderFrames();
}


void RenderToFileDialog::done()
// ----------------------------------------------------------------------------
//    The render operation has completed succesfully
// ----------------------------------------------------------------------------
{
    setCursor(Qt::ArrowCursor);
    okToDismiss = true;
    dismissShortly();
}


void RenderToFileDialog::failed()
// ----------------------------------------------------------------------------
//    The render operation has failed
// ----------------------------------------------------------------------------
{
    setCursor(Qt::ArrowCursor);
    QDialog::reject();
}


void RenderToFileDialog::dismissShortly()
// ----------------------------------------------------------------------------
//    Close dialog after a short period of time (let user see 100% completion)
// ----------------------------------------------------------------------------
{
    QTimer::singleShot(200, this, SLOT(dismiss()));
}


void RenderToFileDialog::dismiss()
// ----------------------------------------------------------------------------
//    Immediately close dialog (on success)
// ----------------------------------------------------------------------------
{
    QDialog::accept();
}


void RenderToFileDialog::on_browseButton_clicked()
// ----------------------------------------------------------------------------
//    The "Browse..." button was clicked
// ----------------------------------------------------------------------------
{
    QString folder = QFileDialog::getExistingDirectory(this,
                         tr("Select destination folder"),
                         folderEdit->text());
    if (!folder.isEmpty())
        folderEdit->setText(folder);
}

}
