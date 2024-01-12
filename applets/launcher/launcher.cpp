#include "launcher.h"

void LauncherApplet::externalWidgetSetup(ConfigManager* cfgMan,
                                         Panel* parentPanel) {
    QIcon::setThemeName(cfgMan->mIconTheme);

    mExternalWidget = new QPushButton();
    mExternalWidget->setIconSize(QSize(parentPanel->mLauncherIconSize,
                                       parentPanel->mLauncherIconSize));
    mExternalWidget->setFlat(true);

    QStringList launcherData = mEntry.split(':');
    if (mEntry.endsWith(".desktop")) {
        // Desktop Entry
        QString desktopEntryPath;
        if (QFile::exists("/usr/share/applications/" + launcherData[1])) {
            desktopEntryPath = "/usr/share/applications/" + launcherData[1];
        }
        else {
            QString homeDir = QDir::homePath();
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
            mExternalWidget->setIcon(QIcon::fromTheme(iconPath));
        }
        else {
            if (QFile::exists(iconPath)) {
                mExternalWidget->setIcon(QIcon(iconPath));
            }
            else {
                mExternalWidget->setIcon(QIcon::fromTheme("dialog-question"));
            }
        }

        mExternalWidget->setToolTip(tooltipLabel);
        connect(mExternalWidget, &QPushButton::clicked, this, [this, exec,
                                                               cfgMan,
                                                               parentPanel]() {
            execute(cfgMan, parentPanel, exec);
        });
    }
    else {
        // Arbitrary Executable
        QString iconPath = launcherData[2];
        if (QFile::exists(iconPath)) {
            mExternalWidget->setIcon(QIcon(iconPath));
        }
        else {
            mExternalWidget->setIcon(QIcon::fromTheme(iconPath));
        }

        if (launcherData.count() == 4) {
            mExternalWidget->setToolTip(launcherData[3]);
        }

        connect(mExternalWidget, &QPushButton::clicked, parentPanel,
                [this, launcherData, cfgMan, parentPanel]() {
            execute(cfgMan, parentPanel, launcherData[1]);
        });
    }
}

void LauncherApplet::execute(ConfigManager* cfgMan,
                             Panel* parentPanel,
                             QString cmd) {
    QProcess* process = new QProcess(parentPanel->mExecHolder);
    parentPanel->mProcesses.append(process);
    process->start(cmd);

    if (cfgMan->mEnableAnimation) {
        QPropertyAnimation* animation = new QPropertyAnimation(mExternalWidget, "iconSize");
        animation->setDuration(200);
        animation->setStartValue(mExternalWidget->iconSize());
        animation->setEndValue(QSize(64, 64));
        animation->start();
        connect(animation, &QPropertyAnimation::finished, this, [this, animation]() {
            if (mExternalWidget != NULL) {
                animation->setDuration(50);
                animation->setStartValue(mExternalWidget->iconSize());
                animation->setEndValue(QSize(mIconSize, mIconSize));
                animation->start();
            }
        });
    }
}

LauncherApplet::LauncherApplet(ConfigManager* cfgMan,
                               Panel* parentPanel,
                               QString additionalInfo) : Applet(cfgMan,
                                                                parentPanel,
                                                                additionalInfo) {
    mIconSize = parentPanel->mLauncherIconSize;
    mEntry = additionalInfo;
}

LauncherApplet::~LauncherApplet() {

}
