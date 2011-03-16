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
      folder(Application::defaultProjectFolderPath() + tr("/frames")),
      x(640), y(480), start(0.0), end(10.0), fps(25),
      widget(widget),
      okToDismiss(false), okButton(NULL), rendering(false)
{
    setupUi(this);
    okButton = buttonBox->button(QDialogButtonBox::Ok);
    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->setValue(0);

    // Restore previous settings

    QSettings settings;
    settings.beginGroup("renderToFiles");
    folder = settings.value("folder", folder).toString();
    x = settings.value("x", x).toInt();
    y = settings.value("y", y).toInt();
    start = settings.value("start", start).toDouble();
    end = settings.value("end", end).toDouble();
    fps = settings.value("fps", fps).toDouble();
    settings.endGroup();

    // Fill values

    folderEdit->setText(folder);
    QCompleter *pc = new QCompleter(TaoApp->pathCompletions(), this);
    folderEdit->setCompleter(pc);
    xResolution->setText(QString::number(x));
    yResolution->setText(QString::number(y));
    startEdit->setText(QString::number(start));
    endEdit->setText(QString::number(end));
    fpsEdit->setText(QString::number(fps));
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
    if (!dir.exists())
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
    x = xResolution->text().toInt();
    y = yResolution->text().toInt();
    start = startEdit->text().toDouble();
    end = endEdit->text().toDouble();
    fps = fpsEdit->text().toDouble();

    // Save values
    TaoApp->addPathCompletion(folder);
    QSettings settings;
    settings.beginGroup("renderToFiles");
    settings.setValue("folder", folder);
    settings.setValue("x", x);
    settings.setValue("y", y);
    settings.setValue("start", start);
    settings.setValue("end", end);
    settings.setValue("fps", fps);
    settings.endGroup();

    okButton->setEnabled(false);
    setCursor(Qt::BusyCursor);
    connect(widget, SIGNAL(renderFramesProgress(int)),
            progressBar, SLOT(setValue(int)));
    connect(widget, SIGNAL(renderFramesDone()),
            this, SLOT(done()));

    rendering = true;
    widget->renderFrames(x, y, start, end, folder, fps);
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
