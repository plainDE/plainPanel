#include "splitter.h"

void SplitterApplet::externalWidgetSetup() {
    mExternalWidget = new QLabel();

    mExternalWidget->setFont(QFont(mCfgMan->mFontFamily, mCfgMan->mFontSize));
    static_cast<QLabel*>(mExternalWidget)->setAlignment(Qt::AlignCenter);

    QString splitter = "";
    if (mParentPanel->mPanelLayout == Horizontal) {
        splitter = "|";
    }
    else {
        splitter = "—";
    }
    static_cast<QLabel*>(mExternalWidget)->setText(splitter);
}

SplitterApplet::SplitterApplet(ConfigManager* cfgMan,
                               Panel* parentPanel) : StaticApplet(
                                                         "org.plainDE.splitter",
                                                         cfgMan,
                                                         parentPanel
                                                     ) {

}

SplitterApplet::~SplitterApplet() {

}
