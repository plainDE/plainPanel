#include "workspaces.h"

void WorkspacesApplet::addButtons() {
    mCountWorkspaces = KWindowSystem::numberOfDesktops();
    for (int i = 1; i <= mCountWorkspaces; ++i) {
        QString buttonText = getButtonText(i);
        QPushButton* workspaceButton = new QPushButton(buttonText);
        if (!mCfgMan->mShowDesktopNames) {
            workspaceButton->setMaximumWidth(
                mParentPanel->mFontMetrics->horizontalAdvance("100")
            );
        }
        else {
            int textSize = mParentPanel->mFontMetrics->horizontalAdvance(buttonText);
            int space = mParentPanel->mFontMetrics->horizontalAdvance("10");
            int minSize = mParentPanel->mFontMetrics->horizontalAdvance("100");
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

QString WorkspacesApplet::getButtonText(int n) {
    if (mCfgMan->mShowDesktopNames) {
        return KWindowSystem::desktopName(n);
    }
    else {
        return QString::number(n);
    }
}

void WorkspacesApplet::externalWidgetSetup() {
    if (mParentPanel->mPanelLayout == Horizontal) {
        mExternalWidget = new QFrame();
        static_cast<QFrame*>(mExternalWidget)->setFrameStyle(QFrame::NoFrame | QFrame::Plain);

        QHBoxLayout* layout = new QHBoxLayout(mExternalWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(mParentPanel->mSpacing);
        addButtons();
    }
    else {  // Vertical
        mExternalWidget = new QPushButton();
        mExternalWidget->setStyleSheet(mActiveStylesheet);
    }


    // Make connections
    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged,
            this, [this]() {
        accentCurrentDesktop();
    });

    connect(KWindowSystem::self(), &KWindowSystem::desktopNamesChanged,
            this, [this] {
        for (int i = 0; i < mButtonList.count(); ++i) {
            QString buttonText = getButtonText(i + 1);
            this->mButtonList.at(i)->setText(buttonText);
        }
    });

    if (mParentPanel->mPanelLayout == Horizontal) {
        connect(KWindowSystem::self(), &KWindowSystem::numberOfDesktopsChanged,
                this, [this]() {
            foreach (QPushButton* button, mButtonList) {
                delete button;
            }
            mButtonList.clear();
            addButtons();
        });
    }

    accentCurrentDesktop();
}

void WorkspacesApplet::accentCurrentDesktop() {
    int currentDesktop = KWindowSystem::currentDesktop();
    if (mParentPanel->mPanelLayout == Horizontal) {
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
        QString buttonText = getButtonText(KWindowSystem::currentDesktop());
        static_cast<QPushButton*>(mExternalWidget)->setText(buttonText);
    }
}

void WorkspacesApplet::setWorkspace(int n) {
    KWindowSystem::setCurrentDesktop(n);
}

WorkspacesApplet::WorkspacesApplet(ConfigManager* cfgMan,
                                   Panel* parentPanel) : StaticApplet(
                                                             "org.plainDE.workspaces",
                                                             cfgMan,
                                                             parentPanel
                                                         ) {
    mAccent = cfgMan->mAccent;
    mInactiveStylesheet = "background-color: #9a9996; color: #000000;";
    mActiveStylesheet = QString("background-color: %1; color: #ffffff;").arg(cfgMan->mAccent);
}

WorkspacesApplet::~WorkspacesApplet() {

}
