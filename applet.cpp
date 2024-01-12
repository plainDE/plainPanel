#include "applet.h"

void Applet::setBlurredBackground(QWidget* internalWidget) {
    QScreen* screen = internalWidget->screen();
    QRect widgetGeometry = internalWidget->geometry();
    QPixmap pixmap = screen->grabWindow(0,
                                        widgetGeometry.x(),
                                        widgetGeometry.y(),
                                        widgetGeometry.width(),
                                        widgetGeometry.height());
    qDebug() << widgetGeometry;
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
    palette.setBrush(internalWidget->backgroundRole(),
                     QBrush(QPixmap::fromImage(res)));
    internalWidget->setPalette(palette);
}

QPair<int,int> Applet::getButtonCoordinates(QWidget* externalWidget, Panel* parentPanel) {
    int buttonCoord1, buttonCoord2;
    if (parentPanel->mPanelLayout == Horizontal) {
        buttonCoord1 = externalWidget->x() + parentPanel->mAxisShift;
        buttonCoord2 = externalWidget->geometry().topRight().x() + parentPanel->mAxisShift;
    }
    else {  // Vertical
        buttonCoord1 = externalWidget->y() + parentPanel->mAxisShift;
        buttonCoord2 = externalWidget->geometry().bottomRight().y() + parentPanel->mAxisShift;
    }
    return qMakePair(buttonCoord1, buttonCoord2);
}

void Applet::externalWidgetSetup(ConfigManager*, Panel*) {

}

void Applet::preliminaryInternalWidgetSetup(QWidget* internalWidget,
                                            QWidget* externalWidget,
                                            ConfigManager* cfgMan,
                                            Panel* parentPanel,
                                            int width,
                                            int height,
                                            bool canBeTransparent) {
    // Window flags
    internalWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    // Geometry
    QScreen* screen = parentPanel->mPanelScreen;
    QRect screenGeometry = screen->geometry();
    int ax = 0, ay = 0;
    QPair<int,int> buttonCoords = getButtonCoordinates(externalWidget, parentPanel);
    int buttonCoord1 = buttonCoords.first, buttonCoord2 = buttonCoords.second;
    switch (parentPanel->mPanelLocation) {
    case Top:
        ax = (screenGeometry.width() - buttonCoord1 >= width) ? buttonCoord1 : buttonCoord2 - width;
        ay = parentPanel->mPanelThickness + 5;
        break;

    case Bottom:
        ax = (screenGeometry.width() - buttonCoord1 >= width) ? buttonCoord1 : buttonCoord2 - width;
        ay = screenGeometry.height() - parentPanel->mPanelThickness - height - 5;
        break;

    case Left:
        ax = parentPanel->mPanelThickness + 5;
        ay = (screenGeometry.height() - buttonCoord1 >= height) ? buttonCoord1 : buttonCoord2 - height;
        break;

    case Right:
        ax = screenGeometry.width() - parentPanel->mPanelThickness - width - 5;
        ay = (screenGeometry.height() - buttonCoord1 >= height) ? buttonCoord1 : buttonCoord2 - height;
        break;
    }

    ax += screenGeometry.x();
    ay += screenGeometry.y();
    internalWidget->setFixedSize(width, height);
    internalWidget->move(ax, ay);

    // Font
    internalWidget->setFont(cfgMan->mFont);

    // Theme
    QFile stylesheetReader("/usr/share/plainDE/styles/" + cfgMan->mStylesheet);
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    internalWidget->setStyleSheet(styleSheet.readAll());
    if (cfgMan->mTransparent && canBeTransparent) {
        setBlurredBackground(internalWidget);
    }

    // Opacity
    internalWidget->setWindowOpacity(parentPanel->mOpacity);
}

Applet::Applet(ConfigManager*, Panel*, QString) {

}
