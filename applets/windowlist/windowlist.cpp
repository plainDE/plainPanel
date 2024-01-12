#include "windowlist.h"

void WindowListApplet::externalWidgetSetup(ConfigManager* cfgMan, Panel* parentPanel) {
    if (parentPanel->mPanelLayout == Horizontal) {
        mExternalLayout = new QHBoxLayout();
    }
    else {  // Vertical
        mExternalLayout = new QVBoxLayout();
    }
    mExternalLayout->setContentsMargins(0, 0, 0, 0);
    mExternalLayout->setSpacing(parentPanel->mSpacing);

    // Make connections
    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, [this, cfgMan,
                                                                       parentPanel]() {
        addButtons(cfgMan, parentPanel);
    });
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, [this, cfgMan,
                                                                         parentPanel]() {
        removeOldButtons(cfgMan, parentPanel);
    });
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, [this,
                                                                               cfgMan]() {
        accentActiveWindow(cfgMan);
    });
    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, [this, cfgMan,
                                                                                 parentPanel]() {
        addButtons(cfgMan, parentPanel);
    });
}

void WindowListApplet::addButtons(ConfigManager* cfgMan, Panel* parentPanel) {
    getWinList();
    for (auto it = mWIDList.cbegin(), end = mWIDList.cend(); it != end; ++it) {
        KWindowInfo pIDInfo(*it, NET::WMPid);
        if (pIDInfo.pid() != parentPanel->mPanelPID) {
            KWindowInfo desktopInfo(*it, NET::WMDesktop);
            if (!mWinWidgets.contains(*it) && desktopInfo.isOnCurrentDesktop()) {
                KWindowInfo nameInfo(*it, NET::WMName);
                QPixmap icon = KWindowSystem::icon(*it, -1, parentPanel->mPanelThickness, true);
                QString winName = nameInfo.name();

                QPushButton* windowButton = new QPushButton();
                if (parentPanel->mPanelLayout == Horizontal && cfgMan->mWinListShowTitles) {
                    windowButton->setText(winName);
                    windowButton->setFont(cfgMan->mFont);
                }
                windowButton->setIcon(icon);
                windowButton->setIconSize(QSize(mIconSize, mIconSize));
                windowButton->setToolTip(winName);
                if (cfgMan->mTransparent) {
                    windowButton->setFlat(true);
                }

                mWinWidgets[*it] = windowButton;
                mExternalLayout->addWidget(windowButton);

                if (parentPanel->mPanelLayout == Horizontal && cfgMan->mWinListShowTitles) {
                    mFullTitleByWId[*it] = winName;
                    windowButton->setText(shortenTitle(parentPanel, windowButton, winName));
                }

                this->connect(windowButton, &QPushButton::clicked, this, [this, windowButton]() {
                    KWindowInfo windowInfo(mWinWidgets.key(windowButton), NET::WMState | NET::XAWMState);
                    if (windowInfo.mappingState() != NET::Visible || windowInfo.isMinimized()) {
                        KWindowSystem::unminimizeWindow(mWinWidgets.key(windowButton));
                    }
                    else {
                        KWindowSystem::minimizeWindow(mWinWidgets.key(windowButton));
                    }
                });

                if (parentPanel->mPanelLayout == Horizontal && cfgMan->mWinListShowTitles) {
                    updateWinTitlesLength(parentPanel);
                }
            }
            else {
                if (desktopInfo.isOnCurrentDesktop()) {
                    if (parentPanel->mPanelLayout == Horizontal && cfgMan->mWinListShowTitles) {
                        KWindowInfo nameInfo(*it, NET::WMName);
                        QString newName = nameInfo.name();
                        if (mFullTitleByWId[*it] != newName) {
                            updateWinTitlesLength(parentPanel);
                            mFullTitleByWId[*it] = newName;
                            mWinWidgets[*it]->setToolTip(newName);
                            mWinWidgets[*it]->setText(shortenTitle(parentPanel,
                                                                   mWinWidgets[*it],
                                                                   newName));
                        }
                    }
                }
                else {
                    delete mWinWidgets[*it];
                    mWinWidgets.remove(*it);
                    if (parentPanel->mPanelLayout == Horizontal && cfgMan->mWinListShowTitles) {
                        updateWinTitlesLength(parentPanel);
                    }
                }
            }
        }
    }
}

void WindowListApplet::removeOldButtons(ConfigManager* cfgMan, Panel* parentPanel) {
    getWinList();
    QList<WId> keys = mWinWidgets.keys();
    foreach (WId id, keys) {
        if (!mWIDList.contains(id)) {
            delete mWinWidgets[id];
            mWinWidgets.remove(id);
            if (parentPanel->mPanelLayout == Horizontal && cfgMan->mWinListShowTitles) {
                updateWinTitlesLength(parentPanel);
            }
        }
    }
}

void WindowListApplet::updateWinTitlesLength(Panel* parentPanel) {
    for (auto it = mWIDList.cbegin(), end = mWIDList.cend(); it != end; ++it) {
        KWindowInfo pIDInfo(*it, NET::WMPid);
        if (pIDInfo.pid() != parentPanel->mPanelPID) {
            if (mWinWidgets.contains(*it)) {
                QString title = KWindowSystem::readNameProperty(*it, 39);
                if (getTitleSize(parentPanel, title) > mButtonSizeByWId[*it]) {
                    mWinWidgets[*it]->setText(shortenTitle(parentPanel,
                                                           mWinWidgets[*it],
                                                           title));
                }
            }
        }
    }
}

void WindowListApplet::updateWinTitles(Panel* parentPanel) {
    for (auto it = mWIDList.cbegin(), end = mWIDList.cend(); it != end; ++it) {
        KWindowInfo pIDInfo(*it, NET::WMPid);
        if (pIDInfo.pid() != parentPanel->mPanelPID) {
            if (mWinWidgets.contains(*it)) {
                QString title = KWindowSystem::readNameProperty(*it, 39);
                if (mFullTitleByWId[*it] != title) {
                    mFullTitleByWId[*it] = title;
                    mWinWidgets[*it]->setToolTip(title);
                    mWinWidgets[*it]->setText(shortenTitle(parentPanel,
                                                           mWinWidgets[*it],
                                                           title));
                    updateWinTitlesLength(parentPanel);
                }
            }
        }
    }
}

QString WindowListApplet::shortenTitle(Panel* parentPanel,
                                       QPushButton* button,
                                       QString title) {
    int buttonSize = button->geometry().width() - 6;
    int dataSize = getTitleSize(parentPanel, title);
    bool edited = false;
    while ((dataSize > buttonSize && title.length() > 0) || title.length() > 15) {
        edited = true;
        title.chop(1);
        dataSize = getTitleSize(parentPanel, title) +
                   parentPanel->mFontMetrics->horizontalAdvance("...");
    }
    if (edited) {
        title.append("...");
    }
    return title;
}

int WindowListApplet::getTitleSize(Panel* parentPanel, QString title) {
    int iconSize = mIconSize;
    int dataSize = iconSize + 3 + parentPanel->mFontMetrics->horizontalAdvance(title);
    return dataSize;
}

void WindowListApplet::activate(ConfigManager* cfgMan,
                                Panel* parentPanel) {
    if (parentPanel->mPanelLayout == Horizontal && cfgMan->mWinListShowTitles) {
        mInterval = 1500;
        mUpdateTitlesTimer = new QTimer(this);
        mUpdateTitlesTimer->setInterval(mInterval);
        connect(mUpdateTitlesTimer, &QTimer::timeout, this, [this, parentPanel]() {
            updateWinTitles(parentPanel);
        });
        mUpdateTitlesTimer->start();
    }
}

void WindowListApplet::accentActiveWindow(ConfigManager* cfgMan) {
    WId activeWinID = KWindowSystem::activeWindow();
    foreach (QPushButton* button, mWinWidgets) {
        button->setStyleSheet("");
        if (cfgMan->mTransparent) {
            button->setFlat(true);
        }
    }
    if (activeWinID != 0 && mWinWidgets.contains(activeWinID)) {
        QString buttonStyle = QString("background-color: %1; color: #ffffff;").arg(cfgMan->mAccent);
        mWinWidgets[activeWinID]->setStyleSheet(buttonStyle);
        if (cfgMan->mTransparent) {
            mWinWidgets[activeWinID]->setFlat(false);
        }
    }
}

void WindowListApplet::getWinList() {
    mWIDList = KWindowSystem::windows();
}

WindowListApplet::WindowListApplet(ConfigManager* cfgMan,
                                   Panel* parentPanel,
                                   QString additionalInfo) : Applet(cfgMan,
                                                                    parentPanel,
                                                                    additionalInfo) {
    mCfgMan = cfgMan;
    mIconSize = cfgMan->mWinListIconSize;
}

WindowListApplet::~WindowListApplet() {
    if (mCfgMan->mWinListShowTitles) {
        mUpdateTitlesTimer->stop();
    }

    foreach (QPushButton* button, mWinWidgets) {
        delete button;
    }
    mWinWidgets.clear();
}
