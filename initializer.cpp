#include "initializer.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QProcess>

#include "dbusintegration.h"
#include "panel.h"

QJsonObject initConfig;
QList<Panel*> panels;
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

void Initializer::setxkbmap() {
    // setxkbmap
    qDebug() << initConfig["kbLayouts"].toVariant().toStringList();
    if (!initConfig["kbLayouts"].toVariant().toStringList().isEmpty() &&
        !initConfig["kbLayoutToggle"].toString().isEmpty()) {
        QProcess* setxkbmapProcess = new QProcess(this);
        QString kbLayouts = initConfig["kbLayouts"].toString().toLower();
        QString kbLayoutToggle = initConfig["kbLayoutToggle"].toString().toLower();

        QString finalKbLayouts = "";
        QString finalKbLayoutToggle = "";
        bool illegalCharacterEncountered = false;
        foreach (QChar c, kbLayouts) {
            // a~z + comma
            if ((c.unicode() >= 97 && c.unicode() <= 122) ||
                (c.unicode() == 44)) {
                finalKbLayouts += c;
            }
            else {
                illegalCharacterEncountered = true;
                break;
            }
        }

        foreach (QChar c, kbLayoutToggle) {
            // a~z + underscore + colon
            if ((c.unicode() >= 97 && c.unicode() <= 122) ||
                (c.unicode() == 95) ||
                (c.unicode() == 58)) {
                finalKbLayoutToggle += c;
            }
            else {
                illegalCharacterEncountered = true;
                break;
            }
        }

        if (illegalCharacterEncountered) {
            QMessageBox warning;
            warning.setText("An illegal character has been encountered. "
                            "Please check ~/.config/plainDE/config.json "
                            "kbLayout and kbLayoutToggle entries. "
                            "Sanitizer result: " +
                            finalKbLayouts + " for kbLayouts and " +
                            finalKbLayoutToggle + " for kbLayoutToggle.");
            warning.exec();
        }

        setxkbmapProcess->start("setxkbmap", {"-layout",
                                              finalKbLayouts,
                                              "-option",
                                              finalKbLayoutToggle});

        initConfig["kbLayouts"] = QJsonValue(finalKbLayouts);
        initConfig["kbLayoutToggle"] = QJsonValue(finalKbLayoutToggle);
    }
}

void Initializer::autostart() {
    QStringList autostartEntries = initConfig["autostart"].toVariant().toStringList();
    QString pathToCurrentDesktopFile = "";
    QString exec = "";

    foreach (QString entry, autostartEntries) {
        pathToCurrentDesktopFile = "/usr/share/applications/" + entry;
        if (!QFile::exists(pathToCurrentDesktopFile)) {
            QString homeDir = getenv("HOME");
            pathToCurrentDesktopFile = homeDir + "/.local/share/applications/" + entry;
        }

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
}

void Initializer::reconfigurePanel() {
    foreach (QWidget* panel, panels) {
        delete panel;
    }

    readConfig();
    setxkbmap();

    panels.clear();
    panelIDs.clear();
    for (int i = 1; i <= initConfig["countPanels"].toInt(); ++i) {
        if (!initConfig["panel" + QString::number(i)].isNull()) {
            Panel* panel = new Panel(this,
                                     &initConfig,
                                     i,
                                     mApp,
                                     panels);
            panels.append(panel);
            panelIDs.append(panel->winId());
        }
    }

    // Moving panel on other workspaces - Bugfix #3
    this->connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, []() {
        foreach (WId id, panelIDs) {
            KWindowSystem::setOnDesktop(id, KWindowSystem::currentDesktop());
        }
    });
}

Initializer::Initializer(QApplication* app) {
    readConfig();
    setxkbmap();
    mApp = app;

    panels.clear();
    panelIDs.clear();

    for (int i = 1; i <= initConfig["countPanels"].toInt(); ++i) {
        if (!initConfig["panel" + QString::number(i)].isNull()) {
            Panel* panel = new Panel(this,
                                     &initConfig,
                                     i,
                                     mApp,
                                     panels);
            panels.append(panel);
            panelIDs.append(panel->winId());
        }
    }

    // Moving panel on other workspaces - Bugfix #3
    this->connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, []() {
        foreach (WId id, panelIDs) {
            KWindowSystem::setOnDesktop(id, KWindowSystem::currentDesktop());
        }
    });

    autostart();

    DBusIntegration db("org.plainDE.plainPanel", "/Actions", "org.plainDE.actions", this);
}

Initializer::~Initializer() {

}
