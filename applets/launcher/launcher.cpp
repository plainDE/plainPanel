#include "launcher.h"

Launcher::Launcher(Panel* parentPanel,
                   QString entry,
                   int iconSize,
                   QString iconTheme,
                   QList<QProcess*>* processes,
                   QObject* execHolder) {
    QIcon::setThemeName(iconTheme);

    QStringList launcherData = entry.split(':');
    if (entry.endsWith(".desktop")) {
        // Desktop Entry
        QString desktopEntryPath;

        if (QFile::exists("/usr/share/applications/" + launcherData[1])) {
            desktopEntryPath = "/usr/share/applications/" + launcherData[1];
        }
        else {
            QString homeDir = getenv("HOME");
            desktopEntryPath = homeDir + "/.local/share/applications/" + launcherData[1];
        }

        QString exec;
        QString iconPath;
        QString tooltipLabel;

        QSettings desktopFileReader(desktopEntryPath, QSettings::IniFormat);
        desktopFileReader.sync();
        desktopFileReader.beginGroup("Desktop Entry");
        exec = desktopFileReader.value("Exec").toString();
        iconPath = desktopFileReader.value("Icon").toString();
        tooltipLabel = desktopFileReader.value("Name").toString();
        desktopFileReader.endGroup();

        // https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.0.html#exec-variables
        if (exec[exec.length()-2] == "%") {
            exec.chop(2);
        }

        if (QIcon::hasThemeIcon(iconPath)) {
            this->setIcon(QIcon::fromTheme(iconPath));
        }
        else {
            if (QFile::exists(iconPath)) {
                this->setIcon(QIcon(iconPath));
            }
            else {
                this->setIcon(QIcon::fromTheme("dialog-question"));
            }
        }

        this->setToolTip(tooltipLabel);

        parentPanel->connect(this, &QPushButton::clicked, parentPanel,
                      [exec, execHolder, processes]() {
            QProcess* process = new QProcess(execHolder);
            processes->append(process);
            process->start(exec);
        });
    }
    else {
        // Arbitrary Executable
        QString iconPath = launcherData[2];
        if (QFile::exists(iconPath)) {
            this->setIcon(QIcon(iconPath));
        }
        else {
            this->setIcon(QIcon::fromTheme(iconPath));
        }

        parentPanel->connect(this, &QPushButton::clicked, parentPanel,
                             [launcherData, execHolder, processes]() {
            QProcess* process = new QProcess(execHolder);
            processes->append(process);
            process->start(launcherData[1]);
        });
    }


    this->setIconSize(QSize(iconSize, iconSize));
    this->setFlat(true);
}

Launcher::~Launcher() {

}
