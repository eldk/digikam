/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database settings widget
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dbsettingswidget.h"

// Qt includes

#include <QGridLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSqlDatabase>
#include <QSqlError>
#include <QLabel>
#include <QGroupBox>
#include <QTimer>
#include <QTemporaryFile>
#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <QTextBrowser>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dwidgetutils.h"
#include "dexpanderbox.h"
#include "dbengineparameters.h"
#include "databaseserverstarter.h"

namespace Digikam
{

class DatabaseSettingsWidget::Private
{

public:

    Private()
    {
        databasePathLabel = 0;
        expertSettings    = 0;
        dbNoticeBox       = 0;
        sqlInit           = 0;
    }

    QLabel*       databasePathLabel;
    QTextBrowser* sqlInit;
    QGroupBox*    expertSettings;
    QGroupBox*    dbNoticeBox;
};

DatabaseSettingsWidget::DatabaseSettingsWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setupMainArea();
}

DatabaseSettingsWidget::~DatabaseSettingsWidget()
{
    delete d;
}

void DatabaseSettingsWidget::setupMainArea()
{
    QVBoxLayout* const layout  = new QVBoxLayout();
    setLayout(layout);

    // --------------------------------------------------------

    QGroupBox* const dbConfigBox = new QGroupBox(i18n("Database Configuration"), this);
    QVBoxLayout* const vlay      = new QVBoxLayout(dbConfigBox);
    d->databasePathLabel         = new QLabel(i18n("<p>Set here the location where the database files will be stored on your system. "
                                                   "There are 3 database files : one for all root albums, one for thumnails, "
                                                   "and one for faces recognition.<br/>"
                                                   "Write access is required to be able to edit image properties.</p>"
                                                   "Databases are digiKam core engines. Take a care to use a place hosted by a fast "
                                                   "hardware (as SSD) with enough free space especially for thumbnails database.</p>"
                                                   "<p>Note: a remote file system such as NFS, cannot be used here. "
                                                   "For performance reasons, it's also recommended to not use a removable media.</p>"
                                                   "<p></p>"), dbConfigBox);
    d->databasePathLabel->setWordWrap(true);

    dbPathEdit                                       = new DFileSelector(dbConfigBox);
    dbPathEdit->setFileDlgMode(QFileDialog::Directory);

    DHBox* const typeHbox                            = new DHBox();
    QLabel* const databaseTypeLabel                  = new QLabel(typeHbox);
    dbType                                           = new QComboBox(typeHbox);
    databaseTypeLabel->setText(i18n("Type:"));

    QLabel* const dbNameCoreLabel                    = new QLabel(i18n("Core Db Name:"));
    dbNameCore                                       = new QLineEdit();
    QLabel* const dbNameThumbnailsLabel              = new QLabel(i18n("Thumbs Db Name:"));
    dbNameThumbnails                                 = new QLineEdit();
    QLabel* const dbNameFaceLabel                    = new QLabel(i18n("Face Db Name:"));
    dbNameFace                                       = new QLineEdit();
    QLabel* const hostNameLabel                      = new QLabel(i18n("Host Name:"));
    hostName                                         = new QLineEdit();
    QLabel* const hostPortLabel                      = new QLabel(i18n("Host Port:"));
    hostPort                                         = new QSpinBox();
    hostPort->setMaximum(65535);

    QLabel* const connectionOptionsLabel             = new QLabel(i18n("Connect options:"));
    connectionOptions                                = new QLineEdit();

    QLabel* const userNameLabel                      = new QLabel(i18n("User:"));
    userName                                         = new QLineEdit();

    QLabel* const passwordLabel                      = new QLabel(i18n("Password:"));
    password                                         = new QLineEdit();
    password->setEchoMode(QLineEdit::Password);

    QPushButton* const checkDatabaseConnectionButton = new QPushButton(i18n("Check Database Connection"));

    d->expertSettings                                = new QGroupBox();
    d->expertSettings->setFlat(true);
    QFormLayout* const expertSettinglayout           = new QFormLayout();
    d->expertSettings->setLayout(expertSettinglayout);

    expertSettinglayout->addRow(hostNameLabel,          hostName);
    expertSettinglayout->addRow(hostPortLabel,          hostPort);
    expertSettinglayout->addRow(dbNameCoreLabel,        dbNameCore);
    expertSettinglayout->addRow(dbNameThumbnailsLabel,  dbNameThumbnails);
    expertSettinglayout->addRow(dbNameFaceLabel,        dbNameFace);
    expertSettinglayout->addRow(userNameLabel,          userName);
    expertSettinglayout->addRow(passwordLabel,          password);
    expertSettinglayout->addRow(connectionOptionsLabel, connectionOptions);

    expertSettinglayout->addWidget(checkDatabaseConnectionButton);

    vlay->addWidget(typeHbox);
    vlay->addWidget(new DLineWidget(Qt::Horizontal));
    vlay->addWidget(d->databasePathLabel);
    vlay->addWidget(dbPathEdit);
    vlay->addWidget(d->expertSettings);
    vlay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vlay->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    d->dbNoticeBox           = new QGroupBox(i18n("Database Server Intructions"), this);
    QVBoxLayout* const vlay2 = new QVBoxLayout(d->dbNoticeBox);
    QLabel* const notice     = new QLabel(i18n("<p>digiKam expects the above database and user account to already exists. "
                                               "This user also require full access to the database.<br>"
                                               "If your database is not already set up, you can use the following SQL commands "
                                               "(after replacing the password with the correct one)."), d->dbNoticeBox);
    notice->setWordWrap(true);

    d->sqlInit = new QTextBrowser(d->dbNoticeBox);
    d->sqlInit->setOpenExternalLinks(false);
    d->sqlInit->setOpenLinks(false);
    d->sqlInit->setReadOnly(false);
    
    vlay2->addWidget(notice);
    vlay2->addWidget(d->sqlInit);
    vlay2->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vlay2->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    layout->setContentsMargins(QMargins());
    layout->addWidget(dbConfigBox);
    layout->addWidget(d->dbNoticeBox);
    layout->addStretch();
    
    // --------- fill with default values ---------------------
    
    dbType->addItem(i18n("SQLite"),                        SQlite);

#ifdef HAVE_MYSQLSUPPORT
    
#   ifdef HAVE_INTERNALMYSQL
    dbType->addItem(i18n("MySQL Internal (experimental)"), MysqlInternal);
#   endif

    dbType->addItem(i18n("MySQL Server (experimental)"),   MysqlServer);
#endif

    dbType->setToolTip(i18n("<p>Select here the type of database backend.</p>"
                                  "<p><b>SQlite</b> backend is for local database storage with a small or medium collection sizes. "
                                  "It is the default and recommended backend.</p>"
#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL
                                  "<p><b>MySQL Internal</b> backend is for local database storage with huge collection sizes. "
                                  "Be careful: this one still in experimental stage.</p>"
#   endif
                                  
                                  "<p><b>MySQL Server</b> backend is a more robust solution especially for remote and shared database storage. "
                                  "It is also more efficient to manage huge collection sizes. "
                                  "Be careful: this one still in experimental stage.</p>"
#endif
                                 ));

    // --------------------------------------------------------

    connect(dbType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotHandleDBTypeIndexChanged(int)));

    connect(checkDatabaseConnectionButton, SIGNAL(clicked()),
            this, SLOT(checkDatabaseConnection()));
    
    connect(dbNameCore, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(dbNameThumbnails, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(dbNameFace, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(userName, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    slotHandleDBTypeIndexChanged(dbType->currentIndex());
}

int DatabaseSettingsWidget::databaseType() const
{
    return dbType->currentIndex();
}

QString DatabaseSettingsWidget::databaseBackend() const
{
    switch(databaseType())
    {
        case MysqlInternal:
        case MysqlServer:
        {
            return DbEngineParameters::MySQLDatabaseType();
        }
        default: // SQlite
        {
            return DbEngineParameters::SQLiteDatabaseType();
        }
    }
}

void DatabaseSettingsWidget::slotChangeDatabasePath(const QUrl& result)
{
#ifdef _WIN32
    // Work around bug #189168
    QTemporaryFile temp;
    temp.setFileTemplate(result.toLocalFile(QUrl::AddTrailingSlash) + QLatin1String("XXXXXX"));
    temp.open();

    if (!result.isEmpty() && !temp.open())
#else
    QFileInfo targetPath(result.toLocalFile());

    if (!result.isEmpty() && !targetPath.isWritable())
#endif
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("You do not seem to have write access to this database folder.\n"
                                   "Without this access, the caption and tag features will not work."));
    }

    checkDBPath();
}

void DatabaseSettingsWidget::slotDatabasePathEditedDelayed()
{
    QTimer::singleShot(300, this, SLOT(slotDatabasePathEdited()));
}

void DatabaseSettingsWidget::slotDatabasePathEdited()
{
    QString newPath = dbPathEdit->lineEdit()->text();

#ifndef _WIN32

    if (!newPath.isEmpty() && !QDir::isAbsolutePath(newPath))
    {
        dbPathEdit->lineEdit()->setText(QDir::homePath() + QLatin1Char('/') + QDir::fromNativeSeparators(newPath));
    }

#endif

    dbPathEdit->lineEdit()->setText(QDir::toNativeSeparators(newPath));

    checkDBPath();
}

void DatabaseSettingsWidget::slotHandleDBTypeIndexChanged(int index)
{
    setDatabaseInputFields(index);
    handleInternalServer(index);
    slotUpdateSqlInit();
}

void DatabaseSettingsWidget::setDatabaseInputFields(int index)
{
    switch(index)
    {
        case MysqlInternal:
        case MysqlServer:
        {
            d->databasePathLabel->setVisible(false);
            dbPathEdit->setVisible(false);
            d->expertSettings->setVisible(true);

            disconnect(dbPathEdit, SIGNAL(signalUrlSelected(QUrl)),
                    this, SLOT(slotChangeDatabasePath(QUrl)));

            disconnect(dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                    this, SLOT(slotDatabasePathEditedDelayed()));
            
            d->dbNoticeBox->setVisible(index == MysqlServer);
            break;
        }
        default: // SQlite
        {
            d->databasePathLabel->setVisible(true);
            dbPathEdit->setVisible(true);
            d->expertSettings->setVisible(false);

            connect(dbPathEdit, SIGNAL(signalUrlSelected(QUrl)),
                    this, SLOT(slotChangeDatabasePath(QUrl)));

            connect(dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                    this, SLOT(slotDatabasePathEditedDelayed()));

            d->dbNoticeBox->setVisible(false);
            break;
        }
    }
}

void DatabaseSettingsWidget::handleInternalServer(int index)
{
    bool enableFields = (index == MysqlInternal);
        
    hostName->setEnabled(enableFields == Qt::Unchecked);
    hostPort->setEnabled(enableFields == Qt::Unchecked);
    dbNameCore->setEnabled(enableFields == Qt::Unchecked);
    dbNameThumbnails->setEnabled(enableFields == Qt::Unchecked);
    dbNameFace->setEnabled(enableFields == Qt::Unchecked);
    userName->setEnabled(enableFields == Qt::Unchecked);
    password->setEnabled(enableFields == Qt::Unchecked);
    connectionOptions->setEnabled(enableFields == Qt::Unchecked);

    if (enableFields == Qt::Unchecked)
    {
        hostPort->setValue(3306);
    }
    else
    {
        hostPort->setValue(-1);
    }
}

void DatabaseSettingsWidget::slotUpdateSqlInit()
{
    QString sql = QString::fromLatin1("CREATE DATABASE %1; "
                                      "GRANT ALL PRIVILEGES ON %2.* TO \'%3\'@\'localhost\' IDENTIFIED BY \'password\'; "
                                      "FLUSH PRIVILEGES;\n")
                                      .arg(dbNameCore->text())
                                      .arg(dbNameCore->text())
                                      .arg(userName->text());

    if (dbNameThumbnails->text() != dbNameCore->text())
    {
        sql += QString::fromLatin1("CREATE DATABASE %1; "
                                   "GRANT ALL PRIVILEGES ON %2.* TO \'%3\'@\'localhost\' IDENTIFIED BY \'password\'; "
                                   "FLUSH PRIVILEGES;\n")
                                   .arg(dbNameThumbnails->text())
                                   .arg(dbNameThumbnails->text())
                                   .arg(userName->text());
    }

    if (dbNameFace->text() != dbNameCore->text())
    {
        sql += QString::fromLatin1("CREATE DATABASE %1; "
                                   "GRANT ALL PRIVILEGES ON %2.* TO \'%3\'@\'localhost\' IDENTIFIED BY \'password\'; "
                                   "FLUSH PRIVILEGES;\n")
                                   .arg(dbNameFace->text())
                                   .arg(dbNameFace->text())
                                   .arg(userName->text());
    }

    d->sqlInit->setText(sql);
}

void DatabaseSettingsWidget::checkDatabaseConnection()
{
    // TODO : if check DB connection operations can be threaded, use DBusyDlg dialog there...

    qApp->setOverrideCursor(Qt::WaitCursor);

    QString databaseID(QLatin1String("ConnectionTest"));
    QSqlDatabase testDatabase     = QSqlDatabase::addDatabase(databaseBackend(), databaseID);
    DbEngineParameters parameters = getDbEngineParameters();
    testDatabase.setHostName(parameters.hostName);
    testDatabase.setPort(parameters.port);
    testDatabase.setUserName(parameters.userName);
    testDatabase.setPassword(parameters.password);
    testDatabase.setConnectOptions(parameters.connectOptions);

    qApp->restoreOverrideCursor();

    bool result = testDatabase.open();

    if (result)
    {
        QMessageBox::information(qApp->activeWindow(), i18n("Database connection test"),
                                 i18n("Database connection test successful."));
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), i18n("Database connection test"),
                              i18n("Database connection test was not successful. <p>Error was: %1</p>",
                                   testDatabase.lastError().text()) );
    }

    testDatabase.close();
    QSqlDatabase::removeDatabase(databaseID);
}

void DatabaseSettingsWidget::checkDBPath()
{
/*
    bool dbOk          = false;
    bool pathUnchanged = true;
*/
    QString newPath    = dbPathEdit->lineEdit()->text();

    if (!dbPathEdit->lineEdit()->text().isEmpty())
    {
        QDir dbDir(newPath);
        QDir oldDir(originalDbPath);
/*
        dbOk          = dbDir.exists();
        pathUnchanged = (dbDir == oldDir);
*/
    }

    //TODO create an Enable button slot, if the path is valid
    //d->mainDialog->enableButtonOk(dbOk);
}

void DatabaseSettingsWidget::setParametersFromSettings(const ApplicationSettings* const settings)
{
    originalDbPath    = settings->getDatabaseFilePath();
    originalDbBackend = settings->getDatabaseType();
    dbPathEdit->lineEdit()->setText(settings->getDatabaseFilePath());

    if (settings->getDatabaseType() == DbEngineParameters::SQLiteDatabaseType())
    {
        dbType->setCurrentIndex(SQlite);
    }
#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL
    else if (settings->getDatabaseType() == DbEngineParameters::MySQLDatabaseType() && settings->getInternalDatabaseServer())
    {
        dbType->setCurrentIndex(MysqlInternal);
    }
#   endif
    else
    {
        dbType->setCurrentIndex(MysqlServer);
    }
#endif

    dbNameCore->setText(settings->getDatabaseNameCore());
    dbNameThumbnails->setText(settings->getDatabaseNameThumbnails());
    dbNameFace->setText(settings->getDatabaseNameFace());
    hostName->setText(settings->getDatabaseHostName());
    hostPort->setValue(settings->getDatabasePort());
    connectionOptions->setText(settings->getDatabaseConnectoptions());

    userName->setText(settings->getDatabaseUserName());
    password->setText(settings->getDatabasePassword());
    
    slotHandleDBTypeIndexChanged(dbType->currentIndex());
}

DbEngineParameters DatabaseSettingsWidget::getDbEngineParameters() const
{
    DbEngineParameters parameters;

    if ((databaseType() == SQlite) || !(databaseType() == MysqlInternal))
    {
        parameters.connectOptions = connectionOptions->text();
        parameters.databaseType   = databaseBackend();
        parameters.hostName       = hostName->text();
        parameters.password       = password->text();
        parameters.port           = hostPort->text().toInt();
        parameters.userName       = userName->text();

        if (parameters.databaseType == DbEngineParameters::SQLiteDatabaseType())
        {
            parameters.databaseNameCore       = QDir::cleanPath(dbPathEdit->lineEdit()->text() + 
                                                QLatin1Char('/') + QLatin1String("digikam4.db"));
            parameters.databaseNameThumbnails = QDir::cleanPath(dbPathEdit->lineEdit()->text() + 
                                                QLatin1Char('/') + QLatin1String("thumbnails-digikam.db"));
            parameters.databaseNameFace       = QDir::cleanPath(dbPathEdit->lineEdit()->text() + 
                                                QLatin1Char('/') + QLatin1String("recognition.db"));
        }
        else
        {
            parameters.databaseNameCore       = dbNameCore->text();
            parameters.databaseNameThumbnails = dbNameThumbnails->text();
            parameters.databaseNameFace       = dbNameFace->text();
        }
    }
    else
    {
        parameters = DbEngineParameters::defaultParameters(databaseBackend());
        DatabaseServerStarter::startServerManagerProcess(databaseBackend());
    }

    return parameters;
}

} // namespace Digikam
