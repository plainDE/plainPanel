#include "localipv4.h"


void LocalIPv4Applet::externalWidgetSetup(ConfigManager* cfgMan,
                                          Panel* parentPanel) {
    QGraphicsScene* ipScene = new QGraphicsScene();
    mTextItem = ipScene->addText("0.0.0.0");
    mTextItem->setFont(cfgMan->mFont);

    QString color = cfgMan->mIPAddrColor;
    QList<int> rgb;
    QString block;
    for (int i = 1; i < 6; i += 2) {
        block = QString(color.at(i)) + QString(color.at(i+1));
        rgb.append(block.toInt(NULL, 16));
    }
    mTextItem->setDefaultTextColor(QColor::fromRgb(rgb[0], rgb[1], rgb[2]));

    mExternalWidget = new QGraphicsView(ipScene);
    mExternalWidget->setStyleSheet("background: transparent");

    if (parentPanel->mPanelLayout == Vertical) {
        if (parentPanel->mPanelLocation == Left) {
            mExternalWidget->rotate(90);
            mIPAngle = 90;
        }
        else {
            mExternalWidget->rotate(270);
            mIPAngle = 270;
        }
        mExternalWidget->setMaximumHeight(
            parentPanel->mFontMetrics->horizontalAdvance("0.0.0.0"));
        mExternalWidget->setMinimumHeight(
            parentPanel->mFontMetrics->horizontalAdvance("0.0.0.0"));
    }
    else {
        mExternalWidget->setMaximumWidth(
            parentPanel->mFontMetrics->horizontalAdvance("0.0.0.0"));
        mExternalWidget->setMinimumWidth(
            parentPanel->mFontMetrics->horizontalAdvance("0.0.0.0"));
    }

    mExternalWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mExternalWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mExternalWidget->setToolTip(cfgMan->mNetworkInterface);
}

void LocalIPv4Applet::repeatingAction(ConfigManager* cfgMan,
                                      Panel* parentPanel) {
    QString newIP = getLocalIP(cfgMan->mNetworkInterface);
    if (newIP != mLastIP) {
        mLastIP = newIP;
        mTextItem->setPlainText(newIP);
        if (parentPanel->mPanelLayout == Horizontal) {
            short maxWidth = parentPanel->mFontMetrics->horizontalAdvance(newIP) + parentPanel->mFontMetrics->averageCharWidth();
            mExternalWidget->setMaximumWidth(maxWidth);
            mExternalWidget->setMinimumWidth(maxWidth);
        }
        else {
            short maxHeight = parentPanel->mFontMetrics->horizontalAdvance(newIP) + parentPanel->mFontMetrics->averageCharWidth();
            if (mIPAngle == 270) {
                maxHeight += parentPanel->mFontMetrics->horizontalAdvance("A");
            }
            mExternalWidget->setMaximumHeight(maxHeight);
            mExternalWidget->setMinimumHeight(maxHeight);
        }
    }
}

void LocalIPv4Applet::activate(ConfigManager* cfgMan, Panel* parentPanel) {
    mInterval = 15000;
    mTimer = new QTimer(this);
    mTimer->setInterval(mInterval);
    connect(mTimer, &QTimer::timeout, this, [this, cfgMan, parentPanel]() {
        repeatingAction(cfgMan, parentPanel);
    });
    mTimer->start();
}

QString LocalIPv4Applet::getLocalIP(QString ifname) {
    mNetIface = QNetworkInterface::interfaceFromName(ifname);
    mAddrEntries = mNetIface.addressEntries();

    if (!mAddrEntries.isEmpty()) {
        return mAddrEntries.at(0).ip().toString();
    }
    else {
        return "0.0.0.0";
    }
}

LocalIPv4Applet::LocalIPv4Applet(ConfigManager* cfgMan,
                                 Panel* parentPanel,
                                 QString additionalInfo) : Applet(cfgMan,
                                                                  parentPanel,
                                                                  additionalInfo) {

}

LocalIPv4Applet::~LocalIPv4Applet() {

}

