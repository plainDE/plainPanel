#include "usermenu.h"

#include <QDebug>

userMenuUI UserMenuApplet::__createUI__(PanelLocation location, short panelHeight, QFont font,
                                        short buttonX, short buttonXRight, QString theme,
                                        qreal opacity) {
    QWidget* userMenuWidget = new QWidget;
    userMenuWidget->setObjectName("userMenu");
    QFontMetrics fm(font);

    // Window flags
    userMenuWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                                   Qt::X11BypassWindowManagerHint);

    // Geometry
    QScreen* primaryScreen = QGuiApplication::primaryScreen();

    short userMenuWidth = fm.horizontalAdvance("About plainDE") + 20;
    short userMenuHeight = 150;
    short ax = 0, ay = 0;
    if (location == top) {
        ay = panelHeight + 5;
    }
    else {
        ay = primaryScreen->geometry().height() - panelHeight - userMenuHeight - 5;
    }
    if (primaryScreen->geometry().width() - buttonX >= userMenuWidth) {
        ax = buttonX;
    }
    else {
        ax = buttonXRight - userMenuWidth;
    }
    userMenuWidget->setFixedSize(userMenuWidth, userMenuHeight);
    userMenuWidget->move(ax, ay);

    // Theme
    QFile stylesheetReader("/usr/share/plainDE/styles/" + theme);
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    userMenuWidget->setStyleSheet(styleSheet.readAll() + "QPushButton { text-align: left; }");

    // Opacity
    userMenuWidget->setWindowOpacity(opacity);

    // Set font
    userMenuWidget->setFont(font);

    // UI
    QVBoxLayout* userMenuLayout = new QVBoxLayout;
    userMenuLayout->setContentsMargins(1, 1, 1, 1);
    userMenuWidget->setLayout(userMenuLayout);

    QPushButton* settingsEntry = new QPushButton;
    settingsEntry->setFlat(true);
    settingsEntry->setText("Settings");
    settingsEntry->setIcon(QIcon::fromTheme("preferences-system"));
    settingsEntry->setFont(font);
    userMenuWidget->layout()->addWidget(settingsEntry);

    QPushButton* aboutEntry = new QPushButton;
    aboutEntry->setFlat(true);
    aboutEntry->setText("About plainDE");
    aboutEntry->setIcon(QIcon::fromTheme("help-about"));
    aboutEntry->setFont(font);
    userMenuWidget->layout()->addWidget(aboutEntry);

    QPushButton* logOutEntry = new QPushButton;
    logOutEntry->setFlat(true);
    logOutEntry->setText("Log Out");
    logOutEntry->setIcon(QIcon::fromTheme("system-log-out"));
    logOutEntry->setFont(font);
    userMenuWidget->layout()->addWidget(logOutEntry);

    QPushButton* suspendEntry = new QPushButton;
    suspendEntry->setFlat(true);
    suspendEntry->setText("Suspend");
    suspendEntry->setIcon(QIcon::fromTheme("system-suspend"));
    suspendEntry->setFont(font);
    userMenuWidget->layout()->addWidget(suspendEntry);

    QPushButton* rebootEntry = new QPushButton;
    rebootEntry->setFlat(true);
    rebootEntry->setText("Reboot");
    rebootEntry->setIcon(QIcon::fromTheme("system-reboot"));
    rebootEntry->setFont(font);
    userMenuWidget->layout()->addWidget(rebootEntry);

    QPushButton* powerOffEntry = new QPushButton;
    powerOffEntry->setFlat(true);
    powerOffEntry->setText("Power Off");
    powerOffEntry->setIcon(QIcon::fromTheme("system-shutdown"));
    powerOffEntry->setFont(font);
    userMenuWidget->layout()->addWidget(powerOffEntry);


    // Make connections
    userMenuWidget->connect(powerOffEntry, &QPushButton::clicked, userMenuWidget,
                            [userMenuWidget]() {
        userMenuWidget->hide();

        QMessageBox powerOffMsg;
        powerOffMsg.setWindowTitle("Power Off");
        powerOffMsg.setText("Are you sure you want to shut down your system?");
        powerOffMsg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        powerOffMsg.setIcon(QMessageBox::Question);

        if (powerOffMsg.exec() == QMessageBox::Yes) {
            system("systemctl poweroff");
        }
    });

    userMenuWidget->connect(rebootEntry, &QPushButton::clicked, userMenuWidget,
                            [userMenuWidget]() {
        userMenuWidget->hide();

        QMessageBox powerOffMsg;
        powerOffMsg.setWindowTitle("Reboot");
        powerOffMsg.setText("Are you sure you want to reboot your system?");
        powerOffMsg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        powerOffMsg.setIcon(QMessageBox::Question);

        if (powerOffMsg.exec() == QMessageBox::Yes) {
            system("systemctl reboot");
        }
    });

    userMenuWidget->connect(logOutEntry, &QPushButton::clicked, userMenuWidget,
                            [userMenuWidget]() {
        userMenuWidget->hide();

        QMessageBox powerOffMsg;
        powerOffMsg.setWindowTitle("Log Out");
        powerOffMsg.setText("Are you sure you want to log out?");
        powerOffMsg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        powerOffMsg.setIcon(QMessageBox::Question);

        if (powerOffMsg.exec() == QMessageBox::Yes) {
            system("loginctl kill-user $USER");
        }
    });

    userMenuWidget->connect(suspendEntry, &QPushButton::clicked, userMenuWidget,
                            [userMenuWidget]() {
        userMenuWidget->hide();
        system("systemctl suspend");
    });

    userMenuWidget->connect(settingsEntry, &QPushButton::clicked, userMenuWidget,
                            [userMenuWidget]() {
        userMenuWidget->hide();

        QProcess* process = new QProcess(userMenuWidget);
        process->start("plainControlCenter");
    });

    userMenuWidget->connect(aboutEntry, &QPushButton::clicked, userMenuWidget,
                            [userMenuWidget]() {
        userMenuWidget->hide();

        QProcess* process = new QProcess(userMenuWidget);
        process->start("plainAbout --plainPanel");
    });


    return {userMenuWidget, settingsEntry, aboutEntry, logOutEntry, powerOffEntry, rebootEntry};
}

