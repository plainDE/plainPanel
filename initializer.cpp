#include "initializer.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QProcess>
#include <KX11Extras>

#include "dbusintegration.h"
#include "panel.h"

QJsonObject initConfig;
QList<QWidget*> panels;
QList<WId> panelIDs;

void Initializer::readConfig() {
    // set globally readable variable for reading settings

    QString homeDirectory = getenv("HOME");
    QFile configReader;
    QString data;

    if (!QFile::exists(homeDirectory + "/.config/plainDE/config.json")) {
        qDebug() << homeDirectory + "/.config/plainDE/config.json" + " does not exist. Generating new...";
        system("python3 /usr/share/plainDE/tools/genconfig.py");
    }
    else {
        system("python3 /usr/share/plainDE/tools/update-config.py");
    }

    configReader.setFileName(homeDirectory + "/.config/plainDE/config.json");
    configReader.open(QIODevice::ReadOnly | QIODevice::Text);
    data = configReader.readAll();
    configReader.close();
    initConfig = QJsonDocument::fromJson(data.toUtf8()).object();
}

void Initializer::autostart() {
    QStringList autostartEntries = initConfig["autostart"].toVariant().toStringList();
    QString pathToCurrentDesktopFile = "";
    QString exec = "";

    foreach (QString entry, autostartEntries) {
        pathToCurrentDesktopFile = "/usr/share/applications/" + entry;

        QSettings desktopFileReader(pathToCurrentDesktopFile, QSettings::IniFormat);
        desktopFileReader.sync();
        desktopFileReader.beginGroup("Desktop Entry");
            exec = desktopFileReader.value("Exec").toString();
        desktopFileReader.endGroup();

        /* https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.0.html
         * 'The Exec key' */
        if (exec[exec.length()-2] == "%") {
            exec.chop(2);
        }
        QProcess* process = new QProcess(this);
        process->start(exec);
    }


    // setxkbmap
    qDebug() << initConfig["kbLayouts"].toVariant().toStringList();
    if (!initConfig["kbLayouts"].toVariant().toStringList().isEmpty() &&
        !initConfig["kbLayoutToggle"].toString().isEmpty()) {
        QProcess* setxkbmapProcess = new QProcess(this);
        setxkbmapProcess->start("setxkbmap -layout " + initConfig["kbLayouts"].toString() +
                                " -option " + initConfig["kbLayoutToggle"].toString());
    }
}

void Initializer::reconfigurePanel() {
    foreach (QWidget* panel, panels) {
        delete panel;
    }

    readConfig();

    panels.clear();
    panelIDs.clear();
    if (initConfig.contains("panel1")) {
        Panel* panel = new Panel(nullptr, this, &initConfig, 1);
        panels.append(panel);
        panelIDs.append(panel->winId());
    }
    if (initConfig.contains("panel2")) {
        Panel* panel = new Panel(nullptr, this, &initConfig, 2);
        panels.append(panel);
        panelIDs.append(panel->winId());
    }

    // Moving panel on other workspaces - Bugfix #3
    this->connect(KX11Extras::self(), &KX11Extras::currentDesktopChanged, this, []() {
        foreach (WId id, panelIDs) {
            KX11Extras::setOnDesktop(id, KX11Extras::currentDesktop());
        }
    });

    // setxkbmap
    qDebug() << initConfig["kbLayouts"].toVariant().toStringList();
    if (!initConfig["kbLayouts"].toVariant().toStringList().isEmpty() &&
        !initConfig["kbLayoutToggle"].toString().isEmpty()) {
        QProcess* setxkbmapProcess = new QProcess(this);
        setxkbmapProcess->start("setxkbmap -layout " + initConfig["kbLayouts"].toString() +
                                " -option " + initConfig["kbLayoutToggle"].toString());
    }
}

Initializer::Initializer() {
    readConfig();

    panels.clear();
    panelIDs.clear();
    if (initConfig.contains("panel1")) {
        Panel* panel = new Panel(nullptr, this, &initConfig, 1);
        panels.append(panel);
        panelIDs.append(panel->winId());
    }
    if (initConfig.contains("panel2")) {
        Panel* panel = new Panel(nullptr, this, &initConfig, 2);
        panels.append(panel);
        panelIDs.append(panel->winId());
    }

    // Moving panel on other workspaces - Bugfix #3
    this->connect(KX11Extras::self(), &KX11Extras::currentDesktopChanged, this, []() {
        foreach (WId id, panelIDs) {
            KX11Extras::setOnDesktop(id, KX11Extras::currentDesktop());
        }
    });

    autostart();

    DBusIntegration db("org.plainDE.plainPanel", "/Actions", "org.plainDE.actions", this);
}

Initializer::~Initializer() {

}
