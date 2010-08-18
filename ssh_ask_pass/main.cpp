#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include "ssh_ask_pass_dialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (argc != 2)
    {
        QFileInfo fi(argv[0]);
        QString usage =
                QCoreApplication::tr("Usage: %1 <message>")
                    .arg(fi.fileName());
        QMessageBox::critical(NULL, QCoreApplication::tr("Error"),
                              usage);
        return 1;
    }
    QString question(argv[1]);
    SshAskPassDialog w(question);
    w.show();
    return a.exec();
}
