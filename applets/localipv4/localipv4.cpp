#include "localipv4.h"

void LocalIPv4Applet::externalWidgetSetup() {
    QGraphicsScene* ipScene = new QGraphicsScene();
    mTextItem = ipScene->addText("0.0.0.0");
    mTextItem->setFont(mCfgMan->mFont);

    QString color = mCfgMan->mIPAddrColor;
    QList<int> rgb;
    QString block;
    for (int i = 1; i < 6; i += 2) {
        block = QString(color.at(i)) + QString(color.at(i+1));
        rgb.append(block.toInt(NULL, 16));
    }
    mTextItem->setDefaultTextColor(QColor::fromRgb(rgb[0], rgb[1], rgb[2]));

    mExternalWidget = new QGraphicsView(ipScene);
    mExternalWidget->setStyleSheet("background: transparent");

    if (mParentPanel->mPanelLayout == Vertical) {
        if (mParentPanel->mPanelLocation == Left) {
            static_cast<QGraphicsView*>(mExternalWidget)->rotate(90);
            mIPAngle = 90;
        }
        else {
            static_cast<QGraphicsView*>(mExternalWidget)->rotate(270);
            mIPAngle = 270;
        }
        mExternalWidget->setMaximumHeight(
            mParentPanel->mFontMetrics->horizontalAdvance("0.0.0.0"));
        mExternalWidget->setMinimumHeight(
            mParentPanel->mFontMetrics->horizontalAdvance("0.0.0.0"));
    }
    else {
        mExternalWidget->setMaximumWidth(
            mParentPanel->mFontMetrics->horizontalAdvance("0.0.0.0"));
        mExternalWidget->setMinimumWidth(
            mParentPanel->mFontMetrics->horizontalAdvance("0.0.0.0"));
    }

    static_cast<QGraphicsView*>(mExternalWidget)->
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    static_cast<QGraphicsView*>(mExternalWidget)->
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mExternalWidget->setToolTip(mCfgMan->mNetworkInterface);
}

void LocalIPv4Applet::repeatingAction() {
    QString newIP = getLocalIP(mCfgMan->mNetworkInterface);
    if (newIP != mLastIP) {
        mLastIP = newIP;
        mTextItem->setPlainText(newIP);
        if (mParentPanel->mPanelLayout == Horizontal) {
            short maxWidth = mParentPanel->mFontMetrics->horizontalAdvance(newIP) +
                             mParentPanel->mFontMetrics->averageCharWidth();
            mExternalWidget->setMaximumWidth(maxWidth);
            mExternalWidget->setMinimumWidth(maxWidth);
        }
        else {
            short maxHeight = mParentPanel->mFontMetrics->horizontalAdvance(newIP) +
                              mParentPanel->mFontMetrics->averageCharWidth();
            if (mIPAngle == 270) {
                maxHeight += mParentPanel->mFontMetrics->horizontalAdvance("A");
            }
            mExternalWidget->setMaximumHeight(maxHeight);
            mExternalWidget->setMinimumHeight(maxHeight);
        }
    }
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
                                 Panel* parentPanel) : DynamicApplet(
                                                           "org.plainDE.localIPv4",
                                                            cfgMan,
                                                            parentPanel,
                                                            15000
                                                       ) {

}

LocalIPv4Applet::~LocalIPv4Applet() {

}

