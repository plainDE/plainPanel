#include "launcher.h"

Launcher::Launcher(Panel* parentPanel,
                   QString entry,
                   int iconSz,
                   QString iconTheme,
                   QList<QProcess*>* processes,
                   QObject* execHolder,
                   bool animate) {
    QIcon::setThemeName(iconTheme);
    this->setIconSize(QSize(iconSz, iconSz));
    this->setFlat(true);

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
                      [this, exec, execHolder, processes, animate, iconSz]() {
            QProcess* process = new QProcess(execHolder);
            processes->append(process);
            process->start(exec);

            if (animate) {
                QPropertyAnimation* animation = new QPropertyAnimation(this, "iconSize");
                animation->setDuration(200);
                animation->setStartValue(this->iconSize());
                animation->setEndValue(QSize(64, 64));
                animation->start();
                this->connect(animation, &QPropertyAnimation::finished, this, [this, iconSz, animation]() {
                    //this->setIconSize(QSize(iconSz, iconSz));
                    animation->setDuration(50);
                    animation->setStartValue(this->iconSize());
                    animation->setEndValue(QSize(iconSz, iconSz));
                    animation->start();
                });
            }
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
                             [this, launcherData, execHolder, processes, animate, iconSz]() {
            QProcess* process = new QProcess(execHolder);
            processes->append(process);
            process->start(launcherData[1]);

            if (animate) {
                QPropertyAnimation* animation = new QPropertyAnimation(this, "iconSize");
                animation->setDuration(200);
                animation->setStartValue(this->iconSize());
                animation->setEndValue(QSize(64, 64));
                animation->start();
                this->connect(animation, &QPropertyAnimation::finished, this, [this, iconSz, animation]() {
                    //this->setIconSize(QSize(iconSz, iconSz));
                    animation->setDuration(50);
                    animation->setStartValue(this->iconSize());
                    animation->setEndValue(QSize(iconSz, iconSz));
                    animation->start();
                });
            }
        });
    }
}

Launcher::~Launcher() {

}
