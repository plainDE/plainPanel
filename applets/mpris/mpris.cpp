#include "mpris.h"

#include <QGuiApplication>
#include <QScreen>
#include <QFile>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusConnectionInterface>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QPushButton>
#include <QProcess>
#include <QDBusInterface>
#include <QDebug>

QDBusConnection bus = QDBusConnection::sessionBus();
QDBusMessage request, response;


QVariantMap MPRISApplet::getMetadata(QString playerService) {
    // https://github.com/openwebos/qt/blob/master/tools/qdbus/qdbus/qdbus.cpp

    QDBusInterface iface(playerService, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties");
    response = iface.call("Get", "org.mpris.MediaPlayer2.Player", "Metadata");

    foreach (QVariant v, response.arguments()) {
        return qdbus_cast<QVariantMap>(qvariant_cast<QDBusArgument>(qvariant_cast<QDBusVariant>(v).variant()));
    }

    return QVariantMap();
}

QString MPRISApplet::getProperty(QString playerService,
                                 QString playerPath,
                                 QString propertyInterface,
                                 QString propertyName) {
    // https://github.com/openwebos/qt/blob/master/tools/qdbus/qdbus/qdbus.cpp

    QDBusInterface iface(playerService, playerPath, "org.freedesktop.DBus.Properties");
    response = iface.call("Get", propertyInterface, propertyName);

    foreach (QVariant v, response.arguments()) {
        if (v.userType() == qMetaTypeId<QDBusVariant>()) {
            return qvariant_cast<QDBusVariant>(v).variant().toString();
        }
    }

    return QString();
}

void MPRISApplet::updateIdentity(QString serviceName,
                                 QLabel* iconLabel,
                                 QLabel* titleLabel) {
    QVariantMap metadata = getMetadata(serviceName);

    // Title & Artist (if possible)
    QString mprisTitle, mprisArtist;
    if (metadata.contains("xesam:title")) {
        mprisTitle = metadata["xesam:title"].toString();
        if (metadata.contains("xesam:artist")) {
            mprisArtist = metadata["xesam:artist"].toString();
        }
    }
    //QString mprisTitle, mprisArtist;

    QString finalTitle;

    // App name & icon (in case of inability to get title & artist)
    QString appName = serviceName.split('.').at(3);
    QIcon appIcon = QIcon::fromTheme(appName.toLower());

    iconLabel->setPixmap(appIcon.pixmap(64, 64));

    if (!mprisTitle.isEmpty()) {
        finalTitle = mprisTitle;
        if (!mprisArtist.isEmpty()) {
            finalTitle += " - " + mprisArtist;
        }
    }
    else {
        finalTitle = getProperty(serviceName,
                                 "/org/mpris/MediaPlayer2",
                                 "org.mpris.MediaPlayer2",
                                 "Identity");
    }

    titleLabel->setText(finalTitle);
}

void MPRISApplet::playPauseMedia(QString serviceName,
                                 QLabel* iconLabel,
                                 QLabel* titleLabel) {
    request = QDBusMessage::createMethodCall(serviceName,
                                             "/org/mpris/MediaPlayer2",
                                             "org.mpris.MediaPlayer2.Player",
                                             "PlayPause");
    bus.call(request);
    updateIdentity(serviceName, iconLabel, titleLabel);
}

void MPRISApplet::previous(QString serviceName,
                           QLabel* iconLabel,
                           QLabel* titleLabel) {
    request = QDBusMessage::createMethodCall(serviceName,
                                             "/org/mpris/MediaPlayer2",
                                             "org.mpris.MediaPlayer2.Player",
                                             "Previous");
    bus.call(request);
    updateIdentity(serviceName, iconLabel, titleLabel);
}

void MPRISApplet::next(QString serviceName,
                       QLabel* iconLabel,
                       QLabel* titleLabel) {
    request = QDBusMessage::createMethodCall(serviceName,
                                             "/org/mpris/MediaPlayer2",
                                             "org.mpris.MediaPlayer2.Player",
                                             "Next");
    bus.call(request);
    updateIdentity(serviceName, iconLabel, titleLabel);
}

QWidget* MPRISApplet::createPlayerCard(QString serviceName,
                                       QFont font,
                                       QString theme,
                                       QString accent) {
    QWidget* playerCard = new QWidget;
    QVBoxLayout* cardLayout = new QVBoxLayout;
    playerCard->setLayout(cardLayout);
    playerCard->setFont(font);
    playerCard->setObjectName("mprisCard");
    playerCard->setMaximumWidth(300);
    playerCard->setMinimumHeight(150);

    // Style
    playerCard->setStyleSheet(theme);

    // Layout for icon and label
    QHBoxLayout* cardIdentity = new QHBoxLayout;

    QLabel* iconLabel = new QLabel();
    QLabel* titleLabel = new QLabel();

    updateIdentity(serviceName, iconLabel, titleLabel);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);
    titleLabel->setFont(font);

    cardIdentity->addWidget(iconLabel);
    cardIdentity->addWidget(titleLabel);

    cardLayout->addLayout(cardIdentity);

    // Layout for controls (backward, play/pause, forward)
    QFont controlsFont;
    controlsFont.setPointSize(24);
    QHBoxLayout* controlsLayout = new QHBoxLayout;

    QPushButton* previousPushButton = new QPushButton("⏮");  // U+23EE - previous
    previousPushButton->setStyleSheet(
                "background-color: " + accent + "; color: #ffffff;");
    previousPushButton->setFont(controlsFont);
    previousPushButton->setMaximumHeight(40);
    controlsLayout->addWidget(previousPushButton);

    QPushButton* playPausePushButton = new QPushButton("⏯");  // U+23EF - play/pause
    playPausePushButton->setStyleSheet(
                "background-color: " + accent + "; color: #ffffff;");
    playPausePushButton->setFont(controlsFont);
    playPausePushButton->setMaximumHeight(40);
    controlsLayout->addWidget(playPausePushButton);

    QPushButton* nextPushButton = new QPushButton("⏭");  // U+23ED - next
    nextPushButton->setStyleSheet(
                "background-color: " + accent + "; color: #ffffff;");
    nextPushButton->setFont(controlsFont);
    nextPushButton->setMaximumHeight(40);
    controlsLayout->addWidget(nextPushButton);

    cardLayout->addLayout(controlsLayout);

    // Make connections
    playerCard->connect(previousPushButton, &QPushButton::clicked, playerCard, [this, serviceName, iconLabel, titleLabel]() {
        this->previous(serviceName, iconLabel, titleLabel);
    });

    playerCard->connect(playPausePushButton, &QPushButton::clicked, playerCard, [this, serviceName, iconLabel, titleLabel]() {
        this->playPauseMedia(serviceName, iconLabel, titleLabel);
    });

    playerCard->connect(nextPushButton, &QPushButton::clicked, playerCard, [this, serviceName, iconLabel, titleLabel] {
        this->next(serviceName, iconLabel, titleLabel);
    });

    return playerCard;
}

void MPRISApplet::setPlayerCards(QList<QWidget*>* mprisCards,
                                 QFont font,
                                 QString stylesheet,
                                 QString accent) {
    QStringList dbusServices = QDBusConnection::sessionBus().interface()->registeredServiceNames().value();
    unsigned short countPlayers = 0;
    foreach (QString currentService, dbusServices) {
        if (currentService.startsWith("org.mpris.MediaPlayer2")) {
            ++countPlayers;
            QWidget* currentPlayerCard = createPlayerCard(currentService, font, stylesheet, accent);
            mprisCards->append(currentPlayerCard);
            this->layout()->addWidget(currentPlayerCard);
        }
    }

    if (countPlayers == 0) {
        QLabel* noPlayersLabel = new QLabel("Nothing is playing at the moment.");
        noPlayersLabel->setStyleSheet(this->styleSheet());
        noPlayersLabel->setFont(font);
        noPlayersLabel->setAlignment(Qt::AlignCenter);
        mprisCards->append(noPlayersLabel);
        this->layout()->addWidget(noPlayersLabel);
    }
}

void MPRISApplet::createUI(PanelLocation panelLocation,
                           int panelThickness,
                           int screenWidth,
                           int screenHeight,
                           int buttonCoord1,
                           int buttonCoord2,
                           QString theme,
                           double opacity) {
    QVBoxLayout* mprisLayout = new QVBoxLayout(this);
    mprisLayout->setContentsMargins(0, 0, 0, 0);
    this->setObjectName("mprisApplet");

    // Set window flags
    this->setWindowFlags(Qt::FramelessWindowHint |
                         Qt::WindowStaysOnTopHint |
                         Qt::X11BypassWindowManagerHint);


    // Geometry
    int width = 300, height = 200;
    int ax = 0, ay = 0;
    switch (panelLocation) {
        case Top:
            ax = (screenWidth - buttonCoord1 >= width) ? buttonCoord1 : buttonCoord2 - width;
            ay = panelThickness + 5;
        break;

        case Bottom:
            ax = (screenWidth - buttonCoord1 >= width) ? buttonCoord1 : buttonCoord2 - width;
            ay = screenHeight - panelThickness - height - 5;
        break;

        case Left:
            ax = panelThickness + 5;
            ay = (screenHeight - buttonCoord1 >= height) ? buttonCoord1 : buttonCoord2 - height;
        break;

        case Right:
            ax = screenWidth - panelThickness - width - 5;
            ay = (screenHeight - buttonCoord1 >= height) ? buttonCoord1 : buttonCoord2 - height;
        break;
    }
    this->setFixedSize(width, height);
    this->move(ax, ay);


    // Theme
    QFile stylesheetReader("/usr/share/plainDE/styles/" + theme);
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    this->setStyleSheet(styleSheet.readAll());

    // Opacity
    this->setWindowOpacity(opacity);
}

MPRISApplet::MPRISApplet(PanelLocation panelLocation,
                         int panelThickness,
                         int screenWidth,
                         int screenHeight,
                         int buttonCoord1,
                         int buttonCoord2,
                         QString theme,
                         double opacity) {
    createUI(panelLocation,
             panelThickness,
             screenWidth,
             screenHeight,
             buttonCoord1,
             buttonCoord2,
             theme,
             opacity);
}
