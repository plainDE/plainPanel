#include "usermenu.h"

#include "powerdialog.h"

void UserMenuApplet::externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel) {
    mExternalWidget = new QPushButton();
    mExternalWidget->setFont(cfgMan->mFont);
    mExternalWidget->setObjectName("userButton");
    mExternalWidget->setToolTip(tr("Power & Settings"));
    mExternalWidget->setFlat(true);

    if (parentPanel->mPanelLayout == Horizontal) {
        QString text = QString(" %1").arg(getenv("USER"));
        int userMenuWidth = parentPanel->mFontMetrics->horizontalAdvance(text) + 20;
        mExternalWidget->setText(text);
        mExternalWidget->setMaximumWidth(userMenuWidth);
    }

    QString avatar = cfgMan->mAvatar;
    if (!avatar.compare("")) {
        mExternalWidget->setIcon(QIcon::fromTheme("computer"));
    }
    else {
        if (QIcon::hasThemeIcon(avatar)) {
            mExternalWidget->setIcon(QIcon::fromTheme(avatar));
        }
        else if (QFile::exists(avatar)) {
            mExternalWidget->setIcon(QIcon(avatar));
        }
    }

    // Make connections
    connect(mExternalWidget, &QPushButton::clicked, this, [this, cfgMan]() {
        if (!mInternalWidget->isVisible()) {
            if (cfgMan->mTransparent) {
                setBlurredBackground(mInternalWidget);
            }
            mInternalWidget->show();
        }
        else {
            mInternalWidget->hide();
        }
    });
}

void UserMenuApplet::internalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel) {
    mInternalWidget = new QWidget();

    // Geometry
    int width = parentPanel->mFontMetrics->horizontalAdvance("About plainDE") + 25;
    int height = 150;
    preliminaryInternalWidgetSetup(mInternalWidget,
                                   mExternalWidget,
                                   cfgMan,
                                   parentPanel,
                                   width,
                                   height,
                                   true);

    mInternalWidget->setObjectName("userMenu");

    QVBoxLayout* userMenuLayout = new QVBoxLayout(mInternalWidget);
    userMenuLayout->setContentsMargins(1, 1, 1, 1);

    QPushButton* settingsEntry = new QPushButton();
    settingsEntry->setFlat(true);
    settingsEntry->setText(tr("Settings"));
    settingsEntry->setIcon(QIcon::fromTheme("preferences-system"));
    settingsEntry->setFont(cfgMan->mFont);
    settingsEntry->setStyleSheet("text-align: left;");
    userMenuLayout->addWidget(settingsEntry);

    QPushButton* aboutEntry = new QPushButton();
    aboutEntry->setFlat(true);
    aboutEntry->setText(tr("About plainDE"));
    aboutEntry->setIcon(QIcon("/usr/share/plainDE/menuIcon.png"));
    aboutEntry->setFont(cfgMan->mFont);
    aboutEntry->setStyleSheet("text-align: left;");
    userMenuLayout->addWidget(aboutEntry);

    QPushButton* logOutEntry = new QPushButton();
    logOutEntry->setFlat(true);
    logOutEntry->setText(tr("Log Out"));
    logOutEntry->setIcon(QIcon::fromTheme("system-log-out"));
    logOutEntry->setFont(cfgMan->mFont);
    logOutEntry->setStyleSheet("text-align: left;");
    userMenuLayout->addWidget(logOutEntry);

    QPushButton* suspendEntry = new QPushButton();
    suspendEntry->setFlat(true);
    suspendEntry->setText(tr("Suspend"));
    suspendEntry->setIcon(QIcon::fromTheme("system-suspend"));
    suspendEntry->setFont(cfgMan->mFont);
    suspendEntry->setStyleSheet("text-align: left;");
    userMenuLayout->addWidget(suspendEntry);

    QPushButton* rebootEntry = new QPushButton();
    rebootEntry->setFlat(true);
    rebootEntry->setText("Reboot");
    rebootEntry->setIcon(QIcon::fromTheme("system-reboot"));
    rebootEntry->setFont(cfgMan->mFont);
    rebootEntry->setStyleSheet("text-align: left;");
    userMenuLayout->addWidget(rebootEntry);

    QPushButton* powerOffEntry = new QPushButton();
    powerOffEntry->setFlat(true);
    powerOffEntry->setText("Power Off");
    powerOffEntry->setIcon(QIcon::fromTheme("system-shutdown"));
    powerOffEntry->setFont(cfgMan->mFont);
    powerOffEntry->setStyleSheet("text-align: left;");
    userMenuLayout->addWidget(powerOffEntry);

    // Make connections
    connect(powerOffEntry, &QPushButton::clicked, this,
                            [this, parentPanel, cfgMan]() {
        mInternalWidget->hide();
        PowerDialog* pd = new PowerDialog(Type::PowerOff,
                                          parentPanel,
                                          cfgMan);
        if (cfgMan->mSecondsUntilPowerOff > -1) {
            pd->startTimer();
        }
        connect(pd, &PowerDialog::actionRequested, this, [this, parentPanel]() {
            emit panelShouldQuit();
            connect(parentPanel, &Panel::animationFinished, this, []() {
                system("systemctl poweroff");
            });
        });
        connect(pd, &PowerDialog::cancelled, this, [pd]() {
            delete pd;
        });
    });

    connect(rebootEntry, &QPushButton::clicked, this,
            [this, parentPanel, cfgMan]() {
        mInternalWidget->hide();
        PowerDialog* pd = new PowerDialog(Type::Reboot,
                                          parentPanel,
                                          cfgMan);
        if (cfgMan->mSecondsUntilPowerOff > -1) {
            pd->startTimer();
        }
        connect(pd, &PowerDialog::actionRequested, this, [this, parentPanel]() {
            emit panelShouldQuit();
            connect(parentPanel, &Panel::animationFinished, this, []() {
                system("systemctl reboot");
            });
        });
        connect(pd, &PowerDialog::cancelled, this, [pd]() {
            delete pd;
        });
    });

    connect(logOutEntry, &QPushButton::clicked, this,
            [this, parentPanel, cfgMan]() {
        mInternalWidget->hide();
        PowerDialog* pd = new PowerDialog(Type::LogOut,
                                          parentPanel,
                                          cfgMan);
        if (cfgMan->mSecondsUntilPowerOff > -1) {
            pd->startTimer();
        }
        connect(pd, &PowerDialog::actionRequested, this, [this, parentPanel]() {
            emit panelShouldQuit();
            connect(parentPanel, &Panel::animationFinished, this, []() {
                system("loginctl kill-user $USER");
            });
        });
        connect(pd, &PowerDialog::cancelled, this, [pd]() {
            delete pd;
        });
    });

    this->connect(suspendEntry, &QPushButton::clicked, this,
                            [this]() {
        mInternalWidget->hide();
        system("systemctl suspend");
    });

    this->connect(settingsEntry, &QPushButton::clicked, this,
                            [this, parentPanel]() {
        mInternalWidget->hide();

        QProcess* process = new QProcess(parentPanel->mExecHolder);
        process->start("/usr/bin/plainControlCenter");
    });

    this->connect(aboutEntry, &QPushButton::clicked, this,
                            [this, parentPanel]() {
        mInternalWidget->hide();

        QProcess* process = new QProcess(parentPanel->mExecHolder);
        process->start("/usr/bin/plainAbout --plainPanel");
    });
}

UserMenuApplet::UserMenuApplet(ConfigManager* cfgMan,
                               Panel* parentPanel,
                               QString additionalInfo) : Applet(cfgMan, parentPanel, additionalInfo) {

}

UserMenuApplet::~UserMenuApplet() {

}
