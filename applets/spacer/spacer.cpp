#include "spacer.h"

void SpacerApplet::externalWidgetSetup() {
    mExternalWidget = new QWidget();

    QBoxLayout* layout;
    QSpacerItem* spacerItem;
    if (mParentPanel->mPanelLayout == Horizontal) {
        layout = new QHBoxLayout(mExternalWidget);
        spacerItem = new QSpacerItem(0, 0,
                                     QSizePolicy::Expanding,
                                     QSizePolicy::Ignored);
    }
    else {
        layout = new QVBoxLayout(mExternalWidget);
        spacerItem = new QSpacerItem(0, 0,
                                     QSizePolicy::Ignored,
                                     QSizePolicy::Expanding);
    }
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addItem(spacerItem);
}

SpacerApplet::SpacerApplet(ConfigManager* cfgMan,
                           Panel* parentPanel) : StaticApplet(
                                                     "org.plainDE.spacer",
                                                     cfgMan,
                                                     parentPanel
                                                 ) {

}

SpacerApplet::~SpacerApplet() {

}
