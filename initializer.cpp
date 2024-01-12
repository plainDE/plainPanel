#include "initializer.h"

#include <QWidget>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QProcess>

#include "dbusintegration.h"
#include "panel.h"
#include "configman.h"

QList<Panel*> panels;

void Initializer::autostart() {
    QString homeDir = QDir::homePath();
    QString path = QString("%1/.config/autostart").arg(homeDir);
    QDir autostartDir(path);
    QStringList autostartEntries = autostartDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    qDebug() << "Autostart:" << autostartEntries;

    for (int i = 0; i < autostartEntries.length(); ++i) {
        QString exec;
        //qDebug() << autostartDir.absoluteFilePath(autostartEntries[i])
        QSettings desktopFileReader(autostartDir.absoluteFilePath(
                                        autostartEntries[i]), QSettings::IniFormat);
        desktopFileReader.sync();
        desktopFileReader.beginGroup("Desktop Entry");
            exec = desktopFileReader.value("Exec").toString();
        desktopFileReader.endGroup();

        // https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.0.html
        // 'The Exec key'
        if (exec[exec.length()-2] == "%") {
            exec.chop(2);
        }
        QProcess* process = new QProcess(this);
        process->start(exec);
    }
}

void Initializer::reconfigurePanel() {
    for (int i = 0; i < panels.count(); ++i) {
        delete panels[i];
    }
    panels.clear();

    delete mCfgMan;
    mCfgMan = new ConfigManager();
    mCfgMan->readConfig();
    mCfgMan->setFields();

    for (int i = 1; i <= mCfgMan->mCountPanels; ++i) {
        if (!mCfgMan->mPanels.at(i-1).isNull) {
            Panel* panel = new Panel(this, mCfgMan, i, mApp, panels);
            panels.append(panel);
        }
    }
}

void Initializer::highlightPanel(int n) {
    if (n >= 1 && n <= panels.count()) {
        panels.at(n-1)->highlight();
    }
    else {
        qDebug() << "Cannot highlight: Panel" << n << "does not exist!";
    }
}

Initializer::Initializer(QApplication* app) {
    qDebug() << QString("plainPanel %1.%2.%3").arg(MAJOR_VER,
                                                   MINOR_VER,
                                                   PATCH_VER);

    mCfgMan = new ConfigManager();
    mCfgMan->readConfig();
    mCfgMan->setFields();
    mApp = app;

    panels.clear();

    for (int i = 1; i <= mCfgMan->mCountPanels; ++i) {
        if (!mCfgMan->mPanels.at(i-1).isNull) {
            Panel* panel = new Panel(this, mCfgMan, i, mApp, panels);
            panels.append(panel);
        }
    }

    autostart();

    DBusIntegration db("org.plainDE.plainPanel", "/Actions", "org.plainDE.actions", this);
}

Initializer::~Initializer() {

}
