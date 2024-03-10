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

QDBusConnection mMPRISBus = QDBusConnection::sessionBus();

void MPRISApplet::externalWidgetSetup() {
    mExternalWidget = new QPushButton("⏯");  // U+23EF - play/pause icon
    mExternalWidget->setObjectName("mprisButton");
    mExternalWidget->setToolTip(tr("Playback control"));
    static_cast<QPushButton*>(mExternalWidget)->setFlat(true);

    if (mParentPanel->mPanelLayout == Horizontal) {
        mExternalWidget->setMaximumWidth(mParentPanel->mFontMetrics->horizontalAdvance("⏯"));
    }
    else {  // Vertical
        mExternalWidget->setMaximumHeight(mParentPanel->mFontMetrics->horizontalAdvance("⏯"));
    }

    // Make connections
    connect(static_cast<QPushButton*>(mExternalWidget),
            &QPushButton::clicked, this, [this]() {
        if (!mInternalWidget->isVisible()) {
            mDestructed = false;
            addCards();
            setSize();
            if (mCfgMan->mTransparent) {
                setBlurredBackground();
            }
            mInternalWidget->show();
        }
        else {
            mInternalWidget->hide();
            destructCards();
        }
    });
}

void MPRISApplet::setSize() {
    int width = 300;
    int height = 200;
    if (mHasCards) {
        height = (150 + mParentPanel->mSpacing) * mCards.length();
    }
    preliminaryInternalWidgetSetup(width, height, true);
}

void MPRISApplet::internalWidgetSetup() {
    mInternalWidget = new QWidget();

    // Geometry
    setSize();

    mInternalWidget->setObjectName("mprisApplet");
    QVBoxLayout* layout = new QVBoxLayout(mInternalWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(mParentPanel->mSpacing);
}

QWidget* MPRISApplet::createPlayerCard(QString serviceName) {
    QWidget* playerCard = new QWidget;
    QVBoxLayout* cardLayout = new QVBoxLayout;
    playerCard->setLayout(cardLayout);
    playerCard->setFont(mCfgMan->mFont);
    playerCard->setObjectName("mprisCard");
    playerCard->setMaximumWidth(300);
    playerCard->setMinimumHeight(100);

    // Layout for icon and label
    QHBoxLayout* cardIdentity = new QHBoxLayout;

    QLabel* iconLabel = new QLabel();
    QLabel* titleLabel = new QLabel();

    updateIdentity(serviceName, iconLabel, titleLabel);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);
    titleLabel->setFont(mCfgMan->mFont);

    cardIdentity->addWidget(iconLabel);
    cardIdentity->addWidget(titleLabel);

    cardLayout->addLayout(cardIdentity);

    // Layout for controls (previous, play/pause, next)
    QFont controlsFont;
    controlsFont.setPointSize(24);
    QHBoxLayout* controlsLayout = new QHBoxLayout;

    QPushButton* previousPushButton = new QPushButton("⏮");  // U+23EE - previous
    previousPushButton->setStyleSheet(
        "background-color: " + mCfgMan->mAccent + "; color: #ffffff;");
    previousPushButton->setFont(controlsFont);
    previousPushButton->setMaximumHeight(40);
    previousPushButton->setToolTip(tr("Backward"));
    controlsLayout->addWidget(previousPushButton);

    QPushButton* playPausePushButton = new QPushButton("⏯");  // U+23EF - play/pause
    playPausePushButton->setStyleSheet(
        "background-color: " + mCfgMan->mAccent + "; color: #ffffff;");
    playPausePushButton->setFont(controlsFont);
    playPausePushButton->setMaximumHeight(40);
    playPausePushButton->setToolTip(tr("Play / Pause"));
    controlsLayout->addWidget(playPausePushButton);

    QPushButton* nextPushButton = new QPushButton("⏭");  // U+23ED - next
    nextPushButton->setStyleSheet(
        "background-color: " + mCfgMan->mAccent + "; color: #ffffff;");
    nextPushButton->setFont(controlsFont);
    nextPushButton->setMaximumHeight(40);
    nextPushButton->setToolTip(tr("Forward"));
    controlsLayout->addWidget(nextPushButton);

    cardLayout->addLayout(controlsLayout);

    // Make connections
    playerCard->connect(previousPushButton, &QPushButton::clicked, playerCard,
                        [this, serviceName, iconLabel, titleLabel]() {
        QtConcurrent::run(this, &MPRISApplet::previous, serviceName, iconLabel, titleLabel);
    });

    playerCard->connect(playPausePushButton, &QPushButton::clicked, playerCard, [this, serviceName, iconLabel, titleLabel]() {
        playPause(serviceName, iconLabel, titleLabel);
    });

    playerCard->connect(nextPushButton, &QPushButton::clicked, playerCard, [this, serviceName, iconLabel, titleLabel] {
        QtConcurrent::run(this, &MPRISApplet::next, serviceName, iconLabel, titleLabel);
    });

    return playerCard;
}

void MPRISApplet::addCards() {
    QStringList dbusServices = mMPRISBus.interface()->registeredServiceNames().value();
    unsigned short countPlayers = 0;
    foreach (QString currentService, dbusServices) {
        if (currentService.startsWith("org.mpris.MediaPlayer2")) {
            ++countPlayers;
            QWidget* currentPlayerCard = createPlayerCard(currentService);
            mCards.append(currentPlayerCard);
            mInternalWidget->layout()->addWidget(currentPlayerCard);
            mHasCards = true;
        }
    }

    if (countPlayers == 0) {
        mHasCards = false;
        QLabel* noPlayersLabel = new QLabel("Nothing is playing at the moment.");
        noPlayersLabel->setStyleSheet(mInternalWidget->styleSheet());
        noPlayersLabel->setFont(mCfgMan->mFont);
        noPlayersLabel->setAlignment(Qt::AlignCenter);
        mCards.append(noPlayersLabel);
        mInternalWidget->layout()->addWidget(noPlayersLabel);
    }
}

void MPRISApplet::updateIdentity(QString serviceName,
                                 QLabel* iconLabel,
                                 QLabel* titleLabel) {
    if (!mDestructed) {
        QVariantMap metadata = getMetadata(serviceName);

        // Title & Artist (if possible)
        QString mprisTitle, mprisArtist;
        if (metadata.contains("xesam:title")) {
            mprisTitle = metadata["xesam:title"].toString();
            if (metadata.contains("xesam:artist")) {
                mprisArtist = metadata["xesam:artist"].toString();
            }
        }

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
}

QVariantMap MPRISApplet::getMetadata(QString playerService) {
    // https://github.com/openwebos/qt/blob/master/tools/qdbus/qdbus/qdbus.cpp

    QDBusInterface iface(playerService,
                         "/org/mpris/MediaPlayer2",
                         "org.freedesktop.DBus.Properties");
    mResponse = iface.call("Get", "org.mpris.MediaPlayer2.Player", "Metadata");
    foreach (QVariant v, mResponse.arguments()) {
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
    mResponse = iface.call("Get", propertyInterface, propertyName);
    foreach (QVariant v, mResponse.arguments()) {
        if (v.userType() == qMetaTypeId<QDBusVariant>()) {
            return qvariant_cast<QDBusVariant>(v).variant().toString();
        }
    }
    return QString();
}

void MPRISApplet::playPause(QString serviceName,
                            QLabel* iconLabel,
                            QLabel* titleLabel) {
    mRequest = QDBusMessage::createMethodCall(serviceName,
                                              "/org/mpris/MediaPlayer2",
                                              "org.mpris.MediaPlayer2.Player",
                                              "PlayPause");
    mMPRISBus.call(mRequest);
    updateIdentity(serviceName, iconLabel, titleLabel);
}

void MPRISApplet::previous(QString serviceName,
                           QLabel* iconLabel,
                           QLabel* titleLabel) {
    mRequest = QDBusMessage::createMethodCall(serviceName,
                                              "/org/mpris/MediaPlayer2",
                                              "org.mpris.MediaPlayer2.Player",
                                              "Previous");
    mMPRISBus.call(mRequest);
    QThread::sleep(1);
    updateIdentity(serviceName, iconLabel, titleLabel);
}

void MPRISApplet::next(QString serviceName,
                       QLabel* iconLabel,
                       QLabel* titleLabel) {
    mRequest = QDBusMessage::createMethodCall(serviceName,
                                              "/org/mpris/MediaPlayer2",
                                              "org.mpris.MediaPlayer2.Player",
                                              "Next");
    mMPRISBus.call(mRequest);
    QThread::sleep(1);
    updateIdentity(serviceName, iconLabel, titleLabel);
}

void MPRISApplet::destructCards() {
    foreach (QWidget* card, mCards) {
        delete card;
    }
    mCards.clear();
    mDestructed = true;
    qDebug() << "cards destructed";
}

MPRISApplet::MPRISApplet(ConfigManager* cfgMan,
                         Panel* parentPanel) : StaticApplet(
                                                   "org.plainDE.mpris",
                                                   cfgMan,
                                                   parentPanel
                                               ) {

}

MPRISApplet::~MPRISApplet() {

}
