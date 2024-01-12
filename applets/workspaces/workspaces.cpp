#include "workspaces.h"

void WorkspacesApplet::addButtons(ConfigManager* cfgMan, Panel* parentPanel) {
    mCountWorkspaces = KWindowSystem::numberOfDesktops();
    for (int i = 1; i <= mCountWorkspaces; ++i) {
        QString buttonText = getButtonText(cfgMan, i);
        QPushButton* workspaceButton = new QPushButton(buttonText);
        if (!cfgMan->mShowDesktopNames) {
            workspaceButton->setMaximumWidth(parentPanel->mFontMetrics->horizontalAdvance("100"));
        }
        else {
            int textSize = parentPanel->mFontMetrics->horizontalAdvance(buttonText);
            int space = parentPanel->mFontMetrics->horizontalAdvance("10");
            int minSize = parentPanel->mFontMetrics->horizontalAdvance("100");
            if (textSize < minSize) {
                workspaceButton->setMaximumWidth(minSize);
            }
            else {
                workspaceButton->setMaximumWidth(textSize + space);
            }
        }
        workspaceButton->setStyleSheet("background-color: #9a9996; color: #000000;");

        connect(workspaceButton, &QPushButton::clicked, this, [this, i]() {
            setWorkspace(i);
        });

        mButtonList.append(workspaceButton);
        mExternalWidget->layout()->addWidget(workspaceButton);
    }
}

QString WorkspacesApplet::getButtonText(ConfigManager* cfgMan, int n) {
    if (cfgMan->mShowDesktopNames) {
        return KWindowSystem::desktopName(n);
    }
    else {
        return QString::number(n);
    }
}

void WorkspacesApplet::externalWidgetSetup(ConfigManager* cfgMan,
                                           Panel* parentPanel) {
    if (parentPanel->mPanelLayout == Horizontal) {
        mExternalWidget = new QFrame();
        static_cast<QFrame*>(mExternalWidget)->setFrameStyle(QFrame::NoFrame | QFrame::Plain);

        QHBoxLayout* layout = new QHBoxLayout(mExternalWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(parentPanel->mSpacing);
        addButtons(cfgMan, parentPanel);
    }
    else {  // Vertical
        mExternalWidget = new QPushButton();
        mExternalWidget->setStyleSheet(mActiveStylesheet);
    }


    // Make connections
    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged,
            this, [this, cfgMan, parentPanel]() {
        accentCurrentDesktop(cfgMan, parentPanel);
    });

    connect(KWindowSystem::self(), &KWindowSystem::desktopNamesChanged,
            this, [this, cfgMan, parentPanel] {
        for (int i = 0; i <= mButtonList.count(); ++i) {
            QString buttonText = getButtonText(cfgMan, i);
            this->mButtonList.at(i)->setText(buttonText);
        }
    });

    if (parentPanel->mPanelLayout == Horizontal) {
        connect(KWindowSystem::self(), &KWindowSystem::numberOfDesktopsChanged,
                this, [this, cfgMan, parentPanel]() {
            foreach (QPushButton* button, mButtonList) {
                delete button;
            }
            mButtonList.clear();
            addButtons(cfgMan, parentPanel);
        });
    }

    accentCurrentDesktop(cfgMan, parentPanel);
}

void WorkspacesApplet::accentCurrentDesktop(ConfigManager* cfgMan,
                                            Panel* parentPanel) {
    int currentDesktop = KWindowSystem::currentDesktop();
    if (parentPanel->mPanelLayout == Horizontal) {
        for (int i = 0; i < mButtonList.count(); ++i) {
            if (i + 1 == currentDesktop) {
                mButtonList.at(i)->setStyleSheet(mActiveStylesheet);
            }
            else {
                mButtonList.at(i)->setStyleSheet(mInactiveStylesheet);
            }
        }
    }
    else {  // Vertical
        QString buttonText = getButtonText(cfgMan,
                                           KWindowSystem::currentDesktop());
        static_cast<QPushButton*>(mExternalWidget)->setText(buttonText);
    }
}

void WorkspacesApplet::setWorkspace(int n) {
    KWindowSystem::setCurrentDesktop(n);
}

WorkspacesApplet::WorkspacesApplet(ConfigManager* cfgMan,
                                   Panel* parentPanel,
                                   QString additionalInfo) : Applet(cfgMan,
                                                                    parentPanel,
                                                                    additionalInfo) {
    mAccent = cfgMan->mAccent;
    mInactiveStylesheet = "background-color: #9a9996; color: #000000;";
    mActiveStylesheet = QString("background-color: %1; color: #ffffff;").arg(cfgMan->mAccent);
}

WorkspacesApplet::~WorkspacesApplet() {

}
