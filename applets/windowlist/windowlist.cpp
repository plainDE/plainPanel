#include "windowlist.h"

void WindowListApplet::externalWidgetSetup() {
    mExternalWidget = new QWidget();
    QBoxLayout* layout;
    if (mParentPanel->mPanelLayout == Horizontal) {
        layout = new QHBoxLayout(mExternalWidget);
    }
    else {  // Vertical
        layout = new QVBoxLayout(mExternalWidget);
    }
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(mParentPanel->mSpacing);

    // Make connections
    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, [this]() {
        addButtons();
    });
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, [this]() {
        removeOldButtons();
    });
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, [this]() {
        accentActiveWindow();
    });
    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, [this]() {
        addButtons();
    });
}

void WindowListApplet::addButtons() {
    getWinList();
    for (auto it = mWIDList.cbegin(), end = mWIDList.cend(); it != end; ++it) {
        KWindowInfo pIDInfo(*it, NET::WMPid);
        if (pIDInfo.pid() != mParentPanel->mPanelPID) {
            KWindowInfo desktopInfo(*it, NET::WMDesktop);
            if (!mWinWidgets.contains(*it) && desktopInfo.isOnCurrentDesktop()) {
                KWindowInfo nameInfo(*it, NET::WMName);
                QPixmap icon = KWindowSystem::icon(*it, -1, mParentPanel->mPanelThickness, true);
                QString winName = nameInfo.name();

                QPushButton* windowButton = new QPushButton();
                if (mParentPanel->mPanelLayout == Horizontal && mCfgMan->mWinListShowTitles) {
                    windowButton->setText(winName);
                    windowButton->setFont(mCfgMan->mFont);
                }
                windowButton->setIcon(icon);
                windowButton->setIconSize(QSize(mIconSize, mIconSize));
                windowButton->setToolTip(winName);
                if (mCfgMan->mTransparent) {
                    windowButton->setFlat(true);
                }

                mWinWidgets[*it] = windowButton;
                mExternalWidget->layout()->addWidget(windowButton);

                if (mParentPanel->mPanelLayout == Horizontal && mCfgMan->mWinListShowTitles) {
                    mFullTitleByWId[*it] = winName;
                    windowButton->setText(shortenTitle(windowButton, winName));
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

                if (mParentPanel->mPanelLayout == Horizontal && mCfgMan->mWinListShowTitles) {
                    updateWinTitlesLength();
                }
            }
            else {
                if (desktopInfo.isOnCurrentDesktop()) {
                    if (mParentPanel->mPanelLayout == Horizontal && mCfgMan->mWinListShowTitles) {
                        KWindowInfo nameInfo(*it, NET::WMName);
                        QString newName = nameInfo.name();
                        if (mFullTitleByWId[*it] != newName) {
                            updateWinTitlesLength();
                            mFullTitleByWId[*it] = newName;
                            mWinWidgets[*it]->setToolTip(newName);
                            mWinWidgets[*it]->setText(shortenTitle(mWinWidgets[*it],
                                                                   newName));
                        }
                    }
                }
                else {
                    delete mWinWidgets[*it];
                    mWinWidgets.remove(*it);
                    if (mParentPanel->mPanelLayout == Horizontal && mCfgMan->mWinListShowTitles) {
                        updateWinTitlesLength();
                    }
                }
            }
        }
    }
}

void WindowListApplet::removeOldButtons() {
    getWinList();
    QList<WId> keys = mWinWidgets.keys();
    foreach (WId id, keys) {
        if (!mWIDList.contains(id)) {
            delete mWinWidgets[id];
            mWinWidgets.remove(id);
            if (mParentPanel->mPanelLayout == Horizontal && mCfgMan->mWinListShowTitles) {
                updateWinTitlesLength();
            }
        }
    }
}

void WindowListApplet::updateWinTitlesLength() {
    for (auto it = mWIDList.cbegin(), end = mWIDList.cend(); it != end; ++it) {
        KWindowInfo pIDInfo(*it, NET::WMPid);
        if (pIDInfo.pid() != mParentPanel->mPanelPID) {
            if (mWinWidgets.contains(*it)) {
                QString title = KWindowSystem::readNameProperty(*it, 39);
                if (getTitleSize(title) > mButtonSizeByWId[*it]) {
                    mWinWidgets[*it]->setText(shortenTitle(mWinWidgets[*it],
                                                           title));
                }
            }
        }
    }
}

void WindowListApplet::repeatingAction() {
    for (auto it = mWIDList.cbegin(), end = mWIDList.cend(); it != end; ++it) {
        KWindowInfo pIDInfo(*it, NET::WMPid);
        if (pIDInfo.pid() != mParentPanel->mPanelPID) {
            if (mWinWidgets.contains(*it)) {
                QString title = KWindowSystem::readNameProperty(*it, 39);
                if (mFullTitleByWId[*it] != title) {
                    mFullTitleByWId[*it] = title;
                    mWinWidgets[*it]->setToolTip(title);
                    mWinWidgets[*it]->setText(shortenTitle(mWinWidgets[*it],
                                                           title));
                    updateWinTitlesLength();
                }
            }
        }
    }
}

QString WindowListApplet::shortenTitle(QPushButton* button, QString title) {
    int buttonSize = button->geometry().width() - 6;
    int dataSize = getTitleSize(title);
    bool edited = false;
    while ((dataSize > buttonSize && title.length() > 0) || title.length() > 15) {
        edited = true;
        title.chop(1);
        dataSize = getTitleSize(title) +
                   mParentPanel->mFontMetrics->horizontalAdvance("...");
    }
    if (edited) {
        title.append("...");
    }
    return title;
}

int WindowListApplet::getTitleSize(QString title) {
    int iconSize = mIconSize;
    int dataSize = iconSize + 3 + mParentPanel->mFontMetrics->horizontalAdvance(title);
    return dataSize;
}

void WindowListApplet::activate() {
    if (mParentPanel->mPanelLayout == Horizontal && mCfgMan->mWinListShowTitles) {
        mTimer = new QTimer(this);
        mTimer->setInterval(mInterval);
        connect(mTimer, &QTimer::timeout, this, [this]() {
            repeatingAction();
        });
        mTimer->start();
    }
    addButtons();
    accentActiveWindow();
}

void WindowListApplet::accentActiveWindow() {
    WId activeWinID = KWindowSystem::activeWindow();
    foreach (QPushButton* button, mWinWidgets) {
        button->setStyleSheet("");
        if (mCfgMan->mTransparent) {
            button->setFlat(true);
        }
    }
    if (activeWinID != 0 && mWinWidgets.contains(activeWinID)) {
        QString buttonStyle = QString("background-color: %1; color: #ffffff;").arg(mCfgMan->mAccent);
        mWinWidgets[activeWinID]->setStyleSheet(buttonStyle);
        if (mCfgMan->mTransparent) {
            mWinWidgets[activeWinID]->setFlat(false);
        }
    }
}

void WindowListApplet::getWinList() {
    mWIDList = KWindowSystem::windows();
}

WindowListApplet::WindowListApplet(ConfigManager* cfgMan,
                                   Panel* parentPanel) : DynamicApplet(
                                                              "org.plainDE.windowList",
                                                               cfgMan,
                                                               parentPanel,
                                                               1500
                                                         ) {
    mCfgMan = cfgMan;
    mIconSize = mCfgMan->mWinListIconSize;
}

WindowListApplet::~WindowListApplet() {
    if (mCfgMan->mWinListShowTitles) {
        mTimer->stop();
    }

    foreach (QPushButton* button, mWinWidgets) {
        delete button;
    }
    mWinWidgets.clear();
}
