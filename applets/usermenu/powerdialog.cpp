#include "powerdialog.h"

QString PowerDialog::getInfoText() {
    return mStringByType[mType].arg(mSecondsLeft);
}

PowerDialog::PowerDialog(Type type,
                         Panel* parentPanel,
                         ConfigManager* cfgMan) {
    // Icon theme
    QIcon::setThemeName(cfgMan->mIconTheme);

    mIconByType[Type::PowerOff] = "system-shutdown";
    mIconByType[Type::Reboot] = "system-reboot";
    mIconByType[Type::LogOut] = "system-log-out";

    mActionByType[Type::PowerOff] = "Shut Down";
    mActionByType[Type::Reboot] = "Reboot";
    mActionByType[Type::LogOut] = "Log Out";

    mStringByType[Type::PowerOff] = QString("Are you sure you want to shut down\nyour system?\n\n"
                                            "If you do nothing, the system will shut down\n"
                                            "automatically in %1 sec");
    mStringByType[Type::Reboot] = QString("Are you sure you want to reboot\nyour system?\n\n"
                                          "If you do nothing, the system will reboot\n"
                                          "automatically in %1 sec");
    mStringByType[Type::LogOut] = QString("Are you sure you want to quit all applications\n"
                                          "and log out?\n\n"
                                          "If you do nothing, you will be logged out\n"
                                          "automatically in %1 sec");

    mParentPanel = parentPanel;
    mType = type;

    mSecondsLeft = cfgMan->mSecondsUntilPowerOff;
    mTimer = new QTimer(this);
    mTimer->setInterval(1000);

    QWidget* dialog = new QWidget();
    dialog->setObjectName("powerDialog");

    // Window flags
    dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    // Font
    dialog->setFont(cfgMan->mFont);

    // Theme
    QFile stylesheetReader("/usr/share/plainDE/styles/" + cfgMan->mStylesheet);
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    dialog->setStyleSheet(styleSheet.readAll());

    // Opacity
    dialog->setWindowOpacity(parentPanel->mOpacity);

    // UI
    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
    QHBoxLayout* infoLayout = new QHBoxLayout();

    infoLayout->addSpacerItem(new QSpacerItem(0, 0,
                                              QSizePolicy::MinimumExpanding,
                                              QSizePolicy::Ignored));

    QLabel* iconLabel = new QLabel();
    iconLabel->setPixmap(QIcon::fromTheme(mIconByType[type]).pixmap(64, 64));
    iconLabel->setAlignment(Qt::AlignCenter);
    infoLayout->addWidget(iconLabel);

    QLabel* infoLabel = new QLabel(mStringByType[type].arg(mSecondsLeft));
    //infoLabel->setWordWrap(true);
    infoLayout->addWidget(infoLabel);

    infoLayout->addSpacerItem(new QSpacerItem(0, 0,
                                              QSizePolicy::MinimumExpanding,
                                              QSizePolicy::Ignored));

    mainLayout->addLayout(infoLayout);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addSpacerItem(new QSpacerItem(0, 0,
                                                 QSizePolicy::MinimumExpanding,
                                                 QSizePolicy::Ignored));

    QPushButton* cancelPushButton = new QPushButton("Cancel");
    connect(cancelPushButton, &QPushButton::clicked, this, [this, dialog]() {
        delete dialog;
        delete mTimer;
        emit cancelled();
    });
    buttonsLayout->addWidget(cancelPushButton);

    QPushButton* actionPushButton = new QPushButton(mActionByType[type]);
    connect(actionPushButton, &QPushButton::clicked, this, [this, dialog]() {
        delete dialog;
        delete mTimer;
        emit actionRequested();
    });

    if (cfgMan->mSecondsUntilPowerOff > -1) {
        connect(mTimer, &QTimer::timeout, this, [this, infoLabel]() {
            if (mSecondsLeft > 0) {
                --mSecondsLeft;
                infoLabel->setText(getInfoText());
            }
            else {
                emit actionRequested();
            }
        });
    }
    else {
        QString text = getInfoText();
        infoLabel->setText(text.split("\n\n")[0]);
    }

    buttonsLayout->addWidget(actionPushButton);
    mainLayout->addLayout(buttonsLayout);

    QScreen* screen = dialog->screen();
    QRect scrGeometry = screen->geometry();
    dialog->setGeometry(scrGeometry.width() / 2 - 210,
                        scrGeometry.height() / 2 - 75,
                        420,
                        150);
    dialog->setMinimumSize(420, 150);
    dialog->setMaximumSize(420, 150);

    // Set logout sound
    mPlayer = new QMediaPlayer();
    QString path = cfgMan->mLogoutSound;
    qDebug() << path;
    if (!path.isEmpty() && QFile::exists(path)) {
        mPlayer->setMedia(QUrl::fromLocalFile(path));
        mPlayLogoutSound = true;
    }

    // Transparency
    if (cfgMan->mTransparent) {
        setTransparency(dialog,
                        dialog->geometry().x(),
                        dialog->geometry().y(),
                        420,
                        150);
    }

    dialog->show();
}

void PowerDialog::startTimer() {
    mTimer->start();
}

void PowerDialog::setTransparency(QWidget* widget,
                                  int ax,
                                  int ay,
                                  int width,
                                  int height) {
    QScreen* screen = widget->screen();
    QPixmap pixmap = screen->grabWindow(0, ax, ay, width, height);
    QGraphicsBlurEffect* blurEffect = new QGraphicsBlurEffect();
    blurEffect->setBlurRadius(15);
    blurEffect->setBlurHints(QGraphicsBlurEffect::QualityHint);

    QGraphicsScene* scene = new QGraphicsScene();
    QGraphicsPixmapItem item;
    item.setPixmap(pixmap);
    item.setGraphicsEffect(blurEffect);
    scene->addItem(&item);
    QImage res(QSize(width, height), QImage::Format_ARGB32);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene->render(&ptr, QRectF(), QRectF(0, 0, width, height));

    QPalette palette;
    palette.setBrush(widget->backgroundRole(),
                     QBrush(QPixmap::fromImage(res)));
    widget->setPalette(palette);
}
