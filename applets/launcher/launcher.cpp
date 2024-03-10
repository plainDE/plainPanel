#include "launcher.h"

void LauncherApplet::externalWidgetSetup() {
    QIcon::setThemeName(mCfgMan->mIconTheme);

    mExternalWidget = new QPushButton();
    static_cast<QPushButton*>(mExternalWidget)->setIconSize(
                        QSize(mParentPanel->mLauncherIconSize,
                              mParentPanel->mLauncherIconSize)
                        );
    static_cast<QPushButton*>(mExternalWidget)->setFlat(true);

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

        static_cast<QPushButton*>(mExternalWidget)->setIcon(
            resolveIconNameOrPath(iconPath)
            );

        mExternalWidget->setToolTip(tooltipLabel);
        connect(static_cast<QPushButton*>(mExternalWidget),
                &QPushButton::clicked, this, [this, exec]() {
            execute(exec);
        });
    }
    else {
        // Arbitrary Executable
        QString iconPath = launcherData[2];
        static_cast<QPushButton*>(mExternalWidget)->setIcon(
            resolveIconNameOrPath(iconPath)
            );

        if (launcherData.count() == 4) {
            mExternalWidget->setToolTip(launcherData[3]);
        }

        connect(static_cast<QPushButton*>(mExternalWidget),
                &QPushButton::clicked, mParentPanel,
                [this, launcherData]() {
            execute(launcherData[1]);
        });
    }
}

QIcon LauncherApplet::resolveIconNameOrPath(QString iconNameOrPath) {
    if (QIcon::hasThemeIcon(iconNameOrPath)) {
        return QIcon::fromTheme(iconNameOrPath);
    }
    else if (QFile::exists(iconNameOrPath)) {
        return QIcon(iconNameOrPath);
    }
    else {
        return QIcon::fromTheme("dialog-question");
    }
}

void LauncherApplet::execute(QString cmd) {
    QProcess* process = new QProcess(mParentPanel->mExecHolder);
    mParentPanel->mProcesses.append(process);
    process->start(cmd);

    if (mCfgMan->mEnableAnimation) {
        QPropertyAnimation* animation = new QPropertyAnimation(mExternalWidget, "iconSize");
        animation->setDuration(200);
        animation->setStartValue(
            static_cast<QPushButton*>(mExternalWidget)->iconSize());
        animation->setEndValue(QSize(64, 64));
        animation->start();
        connect(animation, &QPropertyAnimation::finished, this, [this, animation]() {
            if (animation != NULL && mExternalWidget != NULL) {
                animation->setDuration(50);
                animation->setStartValue(
                    static_cast<QPushButton*>(mExternalWidget)->iconSize());
                animation->setEndValue(QSize(mIconSize, mIconSize));
                animation->start();
            }
        });
    }
}

LauncherApplet::LauncherApplet(ConfigManager* cfgMan,
                               Panel* parentPanel,
                               QString entry) : StaticApplet(
                                                    "org.plainDE.launcher",
                                                    cfgMan,
                                                    parentPanel
                                                ) {
    mIconSize = parentPanel->mLauncherIconSize;
    mEntry = entry;
}

LauncherApplet::~LauncherApplet() {

}
