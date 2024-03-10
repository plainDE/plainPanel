#include "kblayout.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

void KbLayoutApplet::externalWidgetSetup() {
    mExternalWidget = new QPushButton();
    mExternalWidget->setFont(mCfgMan->mFont);
    mExternalWidget->setObjectName("kbLayoutButton");

    int buttonWidth = mParentPanel->mFontMetrics->horizontalAdvance("AA");
    if (mParentPanel->mPanelLayout == Horizontal) {
        mExternalWidget->setMaximumWidth(buttonWidth);
    }
    else {
        mExternalWidget->setMaximumHeight(mParentPanel->mFontMetrics->height());
    }

    static_cast<QPushButton*>(mExternalWidget)->setFlat(true);
    int iconSize = mParentPanel->mPanelThickness / 2 + 2;
    static_cast<QPushButton*>(mExternalWidget)->setIconSize(
        QSize(iconSize, iconSize)
    );

    connect(static_cast<QPushButton*>(mExternalWidget),
            &QPushButton::clicked, this, [this]() {
        if (!mInternalWidget->isVisible()) {
            if (mCfgMan->mTransparent) {
                setBlurredBackground();
            }
            setChooserIndication();
            mInternalWidget->show();
        }
        else {
            mInternalWidget->hide();
            resetChooserIndication();
        }
    });
}

void KbLayoutApplet::internalWidgetSetup() {
    mInternalWidget = new QWidget();
    QFont font = QFont(mCfgMan->mFontFamily, mCfgMan->mFontSize);

    // Geometry
    int width = 200;
    int height = mFlagByCode.count() * (mParentPanel->mFontMetrics->height() + 10) + 5;
    preliminaryInternalWidgetSetup(width, height, true);

    mInternalWidget->setObjectName("layoutChooser");

    QVBoxLayout* layout = new QVBoxLayout(mInternalWidget);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(1);

    QVariantHash layouts = mLayoutCodes.toVariantHash();
    foreach (QString kbLayout, mCfgMan->mKbLayouts.split(',')) {
        QString humanReadableName = " " + layouts.key(QVariant(kbLayout));
        QPushButton* layoutButton = new QPushButton(humanReadableName);
        layoutButton->setFont(font);
        layoutButton->setIcon(mFlagByCode[kbLayout]);
        layoutButton->setFlat(true);
        layoutButton->setStyleSheet("text-align: left;");
        mButtonByCode[kbLayout] = layoutButton;
        layout->addWidget(layoutButton);

        connect(layoutButton, &QPushButton::clicked, this, [this, kbLayout]() {
            /* This approach is buggy because here we change order
             * of keyboard layouts that we supply to setxkbmap, while
             * Toggle Keystroke changes pointer to the layout and not
             * the order.
             *
             * We should find a better solution (i.e., directly use
             * XKB library for setting a keyboard layout). */

            QStringList layouts = mCfgMan->mKbLayouts.split(',');
            int currentLayoutIndex = layouts.indexOf(kbLayout);
            for (int i = 0; i < currentLayoutIndex; ++i) {
                layouts.append(layouts.at(i));
            }
            for (int i = 0; i < currentLayoutIndex; ++i) {
                layouts.removeAt(0);
            }
            QString finalLayouts = layouts.join(',');

            setxkbmap(finalLayouts, mCfgMan->mKbToggleMethod);
            mInternalWidget->hide();
            resetChooserIndication();
            qDebug() << finalLayouts;
        });
    }
}

void KbLayoutApplet::setChooserIndication() {
    QString text = "✓ " + mButtonByCode[mLayoutCode]->text();
    mButtonByCode[mLayoutCode]->setText(text);
    mButtonByCode[mLayoutCode]->setStyleSheet("text-align: left; background-color: " + mCfgMan->mAccent + ";");
    mButtonByCode[mLayoutCode]->setFlat(false);
}

void KbLayoutApplet::resetChooserIndication() {
    QVariantHash layouts = mLayoutCodes.toVariantHash();
    foreach (QString layout, mButtonByCode.keys()) {
        QString humanReadableName = layouts.key(QVariant(layout));
        mButtonByCode[layout]->setText(humanReadableName);
        mButtonByCode[layout]->setStyleSheet("text-align: left;");
        mButtonByCode[layout]->setFlat(true);
    }
}

void KbLayoutApplet::repeatingAction() {
    QString code = getCurrentLayoutISOCode();
    static_cast<QPushButton*>(mExternalWidget)->setIcon(mFlagByCode[code]);
    mExternalWidget->setToolTip(code);
}

void KbLayoutApplet::repeatingAction(bool) {
    QString code = getCurrentLayoutISOCode();
    static_cast<QPushButton*>(mExternalWidget)->setText(code);
}

void KbLayoutApplet::activate() {
    mTimer = new QTimer(this);
    mTimer->setInterval(mInterval);
    if (mCfgMan->mUseCountryFlag) {
        connect(mTimer, &QTimer::timeout, this, [this]() {
            repeatingAction();            
        });
    }
    else {
        connect(mTimer, &QTimer::timeout, this, [this]() {
            repeatingAction(false);
        });
    }
    mTimer->start();
}

void KbLayoutApplet::setxkbmap(QString layouts, QString toggleMethod) {
    QProcess* setxkbmapProcess = new QProcess(mParentPanel->mExecHolder);
    setxkbmapProcess->start("setxkbmap", {"-layout",
                                          layouts,
                                          "-option",
                                          toggleMethod});
}

void KbLayoutApplet::setLayouts() {
    // Using setxkbmap to set keyboard layouts and toggle method
    QString layouts = mCfgMan->mKbLayouts;
    QString toggleMethod = mCfgMan->mKbToggleMethod;

    if (!layouts.isEmpty() && !toggleMethod.isEmpty()) {
        QProcess* setxkbmapProcess = new QProcess(mParentPanel->mExecHolder);
        setxkbmap(layouts, toggleMethod);
    }
    else {
        qDebug() << "Incorrect format of kbLayouts or kbToggleMethod "
                    "config entry. Couldn't run setxkbmap. Check these "
                    "entries in ~/.config/plainDE/config.json file.";
    }
}

void KbLayoutApplet::connectToXServer() {
    mKbDisplay = XkbOpenDisplay(getenv("DISPLAY"), NULL, NULL, NULL, NULL, &mDisplayResult);
    mKeyboard = XkbAllocKeyboard();
}

void KbLayoutApplet::setISOCodes() {
    QFile file("/usr/share/plainDE/layouts.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString data = file.readAll();
    file.close();
    mLayoutCodes = QJsonDocument::fromJson(data.toUtf8()).object();
}

void KbLayoutApplet::cacheFlagIcons() {
    foreach (QString layout, mCfgMan->mKbLayouts.split(',')) {
        mFlagByCode[layout] = QIcon("/usr/share/flags/" + layout + ".png");
    }
}

KbLayoutApplet::KbLayoutApplet(ConfigManager* cfgMan,
                               Panel* parentPanel) : DynamicApplet(
                                                         "org.plainDE.kbLayout",
                                                         cfgMan,
                                                         parentPanel,
                                                         350) {
    setLayouts();
    setISOCodes();
    cacheFlagIcons();
    connectToXServer();
}

QString KbLayoutApplet::getCurrentLayoutISOCode() {
    // Obtain symbolic names from the server
    XkbGetNames(mKbDisplay, XkbGroupNamesMask, mKeyboard);
    XkbGetState(mKbDisplay, XkbUseCoreKbd, &mState);

    // Get language code
    mLayout = XGetAtomName(mKbDisplay, mKeyboard->names->groups[mState.group]);

    // Returning converted ISO code
    mLayoutCode = mLayoutCodes[mLayout].toString();
    return mLayoutCode;
}

KbLayoutApplet::~KbLayoutApplet() {

}
