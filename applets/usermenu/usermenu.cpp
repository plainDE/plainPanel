#include "usermenu.h"

#include "powerdialog.h"

void UserMenuApplet::externalWidgetSetup() {
    mExternalWidget = new QPushButton();
    mExternalWidget->setFont(mCfgMan->mFont);
    mExternalWidget->setObjectName("userButton");
    mExternalWidget->setToolTip(tr("Power & Settings"));
    static_cast<QPushButton*>(mExternalWidget)->setFlat(true);

    if (mParentPanel->mPanelLayout == Horizontal) {
        QString text = QString(" %1").arg(getenv("USER"));
        int userMenuWidth = mParentPanel->mFontMetrics->horizontalAdvance(text) + 20;
        static_cast<QPushButton*>(mExternalWidget)->setText(text);
        mExternalWidget->setMaximumWidth(userMenuWidth);
    }

    QString avatar = mCfgMan->mAvatar;
    if (!avatar.compare("")) {
        static_cast<QPushButton*>(mExternalWidget)->setIcon(
            QIcon::fromTheme("computer")
        );
    }
    else {
        if (QIcon::hasThemeIcon(avatar)) {
            static_cast<QPushButton*>(mExternalWidget)->setIcon(
                QIcon::fromTheme(avatar)
            );
        }
        else if (QFile::exists(avatar)) {
            static_cast<QPushButton*>(mExternalWidget)->setIcon(
                QIcon(avatar)
            );
        }
    }

    // Make connections
    connect(static_cast<QPushButton*>(mExternalWidget),
            &QPushButton::clicked, this, [this]() {
        if (!mInternalWidget->isVisible()) {
            if (mCfgMan->mTransparent) {
                setBlurredBackground();
            }
            mInternalWidget->show();
        }
        else {
            mInternalWidget->hide();
        }
    });
}

void UserMenuApplet::internalWidgetSetup() {
    mInternalWidget = new QWidget();

    // Geometry
    int width = mParentPanel->mFontMetrics->horizontalAdvance("About plainDE") + 25;
    int height = 150;
    preliminaryInternalWidgetSetup(width, height, true);

    mInternalWidget->setObjectName("userMenu");

    QVBoxLayout* userMenuLayout = new QVBoxLayout(mInternalWidget);
    userMenuLayout->setContentsMargins(1, 1, 1, 1);

    QPushButton* settingsEntry = new QPushButton();
    settingsEntry->setFlat(true);
    settingsEntry->setText(tr("Settings"));
    settingsEntry->setIcon(QIcon::fromTheme("preferences-system"));
    settingsEntry->setFont(mCfgMan->mFont);
    settingsEntry->setStyleSheet("text-align: left;");
    userMenuLayout->addWidget(settingsEntry);

    QPushButton* aboutEntry = new QPushButton();
    aboutEntry->setFlat(true);
    aboutEntry->setText(tr("About plainDE"));
    aboutEntry->setIcon(QIcon("/usr/share/plainDE/menuIcon.png"));
    aboutEntry->setFont(mCfgMan->mFont);
    aboutEntry->setStyleSheet("text-align: left;");
    userMenuLayout->addWidget(aboutEntry);

    QPushButton* logOutEntry = new QPushButton();
    logOutEntry->setFlat(true);
    logOutEntry->setText(tr("Log Out"));
    logOutEntry->setIcon(QIcon::fromTheme("system-log-out"));
    logOutEntry->setFont(mCfgMan->mFont);
    logOutEntry->setStyleSheet("text-align: left;");
    userMenuLayout->addWidget(logOutEntry);

    QPushButton* suspendEntry = new QPushButton();
    suspendEntry->setFlat(true);
    suspendEntry->setText(tr("Suspend"));
    suspendEntry->setIcon(QIcon::fromTheme("system-suspend"));
    suspendEntry->setFont(mCfgMan->mFont);
    suspendEntry->setStyleSheet("text-align: left;");
    userMenuLayout->addWidget(suspendEntry);

    QPushButton* rebootEntry = new QPushButton();
    rebootEntry->setFlat(true);
    rebootEntry->setText("Reboot");
    rebootEntry->setIcon(QIcon::fromTheme("system-reboot"));
    rebootEntry->setFont(mCfgMan->mFont);
    rebootEntry->setStyleSheet("text-align: left;");
    userMenuLayout->addWidget(rebootEntry);

    QPushButton* powerOffEntry = new QPushButton();
    powerOffEntry->setFlat(true);
    powerOffEntry->setText("Power Off");
    powerOffEntry->setIcon(QIcon::fromTheme("system-shutdown"));
    powerOffEntry->setFont(mCfgMan->mFont);
    powerOffEntry->setStyleSheet("text-align: left;");
    userMenuLayout->addWidget(powerOffEntry);

    // Make connections
    connect(powerOffEntry, &QPushButton::clicked, this,
                            [this]() {
        mInternalWidget->hide();
        PowerDialog* pd = new PowerDialog(Type::PowerOff, mParentPanel, mCfgMan);
        if (mCfgMan->mSecondsUntilPowerOff > -1) {
            pd->startTimer();
        }
        connect(pd, &PowerDialog::actionRequested, this, [this, pd]() {
            emit static_cast<Initializer*>(
                mParentPanel->mExecHolder)->panelShouldQuit();
            if (pd->mPlayLogoutSound) {
                pd->mPlayer->play();
                connect(pd->mPlayer, &QMediaPlayer::stateChanged, this, [pd]() {
                    if (pd->mPlayer->state() == QMediaPlayer::StoppedState) {
                        system("systemctl poweroff");
                    }
                });
            }
            else {

                connect(mParentPanel, &Panel::animationFinished, this, []() {
                    system("systemctl poweroff");
                });
            }
        });
        connect(pd, &PowerDialog::cancelled, this, [pd]() {
            delete pd;
        });
    });

    connect(rebootEntry, &QPushButton::clicked, this,
            [this]() {
        mInternalWidget->hide();
        PowerDialog* pd = new PowerDialog(Type::Reboot, mParentPanel, mCfgMan);
        if (mCfgMan->mSecondsUntilPowerOff > -1) {
            pd->startTimer();
        }
        connect(pd, &PowerDialog::actionRequested, this, [this, pd]() {
            emit static_cast<Initializer*>(
                mParentPanel->mExecHolder)->panelShouldQuit();
            if (pd->mPlayLogoutSound) {
                pd->mPlayer->play();
                connect(pd->mPlayer, &QMediaPlayer::stateChanged, this, [pd]() {
                    if (pd->mPlayer->state() == QMediaPlayer::StoppedState) {
                        system("systemctl reboot");
                    }
                });
            }
            else {
                connect(mParentPanel, &Panel::animationFinished, this, []() {
                    system("systemctl reboot");
                });
            }
        });
        connect(pd, &PowerDialog::cancelled, this, [pd]() {
            delete pd;
        });
    });

    connect(logOutEntry, &QPushButton::clicked, this,
            [this]() {
        mInternalWidget->hide();
        PowerDialog* pd = new PowerDialog(Type::LogOut, mParentPanel, mCfgMan);
        if (mCfgMan->mSecondsUntilPowerOff > -1) {
            pd->startTimer();
        }
        connect(pd, &PowerDialog::actionRequested, this, [this, pd]() {
            emit static_cast<Initializer*>(
                mParentPanel->mExecHolder)->panelShouldQuit();
            if (pd->mPlayLogoutSound) {
                qDebug() << "1";
                pd->mPlayer->play();
                connect(pd->mPlayer, &QMediaPlayer::stateChanged, this, [pd]() {
                    if (pd->mPlayer->state() == QMediaPlayer::StoppedState) {
                        system("loginctl kill-user $USER");
                    }
                });
            }
            else {
                connect(mParentPanel, &Panel::animationFinished, this, []() {
                    system("loginctl kill-user $USER");
                });
            }
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
                            [this]() {
        mInternalWidget->hide();

        QProcess* process = new QProcess(mParentPanel->mExecHolder);
        process->start("/usr/bin/plainControlCenter", {""});
    });

    this->connect(aboutEntry, &QPushButton::clicked, this,
                            [this]() {
        mInternalWidget->hide();

        QProcess* process = new QProcess(mParentPanel->mExecHolder);
        process->start("/usr/bin/plainAbout", {"--plainPanel"});
    });
}

UserMenuApplet::UserMenuApplet(ConfigManager* cfgMan,
                               Panel* parentPanel) : StaticApplet("org.plainDE.userMenu",
                                                                  cfgMan,
                                                                  parentPanel) {

}

UserMenuApplet::~UserMenuApplet() {

}
