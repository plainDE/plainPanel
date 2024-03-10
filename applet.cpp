#include "applet.h"

void Applet::setBlurredBackground() {
    QScreen* screen = mInternalWidget->screen();
    QRect screenGeometry = screen->geometry();
    QRect widgetGeometry = mInternalWidget->geometry();
    QPixmap pixmap = screen->grabWindow(0,
                                        widgetGeometry.x(),
                                        widgetGeometry.y(),
                                        widgetGeometry.width(),
                                        widgetGeometry.height());
    QGraphicsBlurEffect* blurEffect = new QGraphicsBlurEffect();
    blurEffect->setBlurRadius(15);
    blurEffect->setBlurHints(QGraphicsBlurEffect::QualityHint);

    QGraphicsScene* scene = new QGraphicsScene();
    QGraphicsPixmapItem item;
    item.setPixmap(pixmap);
    item.setGraphicsEffect(blurEffect);
    scene->addItem(&item);
    QImage res(QSize(widgetGeometry.width(), widgetGeometry.height()), QImage::Format_ARGB32);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene->render(&ptr, QRectF(), QRectF(0, 0, widgetGeometry.width(), widgetGeometry.height()));

    QPalette palette;
    palette.setBrush(mInternalWidget->backgroundRole(),
                     QBrush(QPixmap::fromImage(res)));
    mInternalWidget->setPalette(palette);
}

QPair<int,int> Applet::getButtonCoordinates() {
    int buttonCoord1, buttonCoord2;
    if (mParentPanel->mPanelLayout == Horizontal) {
        buttonCoord1 = mExternalWidget->x() + mParentPanel->mAxisShift;
        buttonCoord2 = mExternalWidget->geometry().topRight().x() +
                       mParentPanel->mAxisShift;
    }
    else {  // Vertical
        buttonCoord1 = mExternalWidget->y() + mParentPanel->mAxisShift;
        buttonCoord2 = mExternalWidget->geometry().bottomRight().y() +
                       mParentPanel->mAxisShift;
    }
    return qMakePair(buttonCoord1, buttonCoord2);
}

void Applet::preliminaryInternalWidgetSetup(int width,
                                            int height,
                                            bool canBeTransparent) {
    // Window flags
    mInternalWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    // Geometry
    QScreen* screen = mParentPanel->mPanelScreen;
    QRect screenGeometry = screen->geometry();
    int ax = 0, ay = 0;
    QPair<int,int> buttonCoords = getButtonCoordinates();
    int buttonCoord1 = buttonCoords.first, buttonCoord2 = buttonCoords.second;
    switch (mParentPanel->mPanelLocation) {
    case Top:
        ax = (screenGeometry.width() - buttonCoord1 >= width) ? buttonCoord1 : buttonCoord2 - width;
        ay = mParentPanel->mPanelThickness + 5;
        break;

    case Bottom:
        ax = (screenGeometry.width() - buttonCoord1 >= width) ? buttonCoord1 : buttonCoord2 - width;
        ay = screenGeometry.height() - mParentPanel->mPanelThickness - height - 5;
        break;

    case Left:
        ax = mParentPanel->mPanelThickness + 5;
        ay = (screenGeometry.height() - buttonCoord1 >= height) ? buttonCoord1 : buttonCoord2 - height;
        break;

    case Right:
        ax = screenGeometry.width() - mParentPanel->mPanelThickness - width - 5;
        ay = (screenGeometry.height() - buttonCoord1 >= height) ? buttonCoord1 : buttonCoord2 - height;
        break;
    }

    ax += screenGeometry.x();
    ay += screenGeometry.y();
    mInternalWidget->setFixedSize(width, height);
    mInternalWidget->move(ax, ay);

    // Font
    mInternalWidget->setFont(mCfgMan->mFont);

    // Theme
    QFile stylesheetReader("/usr/share/plainDE/styles/" + mCfgMan->mStylesheet);
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    mInternalWidget->setStyleSheet(styleSheet.readAll());
    if (mCfgMan->mTransparent && canBeTransparent) {
        setBlurredBackground();
    }

    // Opacity
    mInternalWidget->setWindowOpacity(mParentPanel->mOpacity);
}

void Applet::externalWidgetSetup() {

}

void Applet::internalWidgetSetup() {

}

Applet::Applet(QString appletID, ConfigManager* cfgMan, Panel* parentPanel) {
    mAppletID = appletID;
    mCfgMan = cfgMan;
    mParentPanel = parentPanel;
}
