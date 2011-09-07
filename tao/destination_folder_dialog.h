#ifndef DESTINATION_FOLDER_DIALOG_H
#define DESTINATION_FOLDER_DIALOG_H
// ****************************************************************************
//  detination_folder_dialog.h                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//    A dialog box for the user to select a destination folder. There is a
//    drop-down list of known locations (paths), and fields to select any other
//    folder. Selected path is returned in native format.
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include <QtGui>


namespace Tao {

class DestinationFolderDialog : public QDialog
{
    Q_OBJECT

public:
    DestinationFolderDialog(QString defaultPath,
                            QStringList &knownLocations,
                            QWidget *parent = 0);

public:
    QString        target();

public slots:
    virtual void   accept();
    virtual void   reject();

protected slots:
    void           comboBoxIndexChanged(int index);
    void           nameChanged();
    void           locationChanged();
    void           chooseLocation();

protected:
    void           updateNameAndLocation(QString path);

protected:
    QStringList    knownLocations;
    QString        saved;
    QComboBox     *combo;
    QLineEdit     *nameLineEdit, *locationLineEdit;
    QPushButton   *locationChooseButton;
    bool           rejected;
};

}

#endif // DESTINATION_FOLDER_DIALOG_H
