#include "usermenu.h"

void UserMenu::createUI(Panel* parentPanel,
                        QObject* execHolder,
                        PanelLocation panelLocation,
                        int panelThickness,
                        int screenWidth,
                        int screenHeight,
                        QFont font,
                        int buttonCoord1,
                        int buttonCoord2,
                        QString stylesheet,
                        double opacity) {
    this->setObjectName("userMenu");

    // Window flags
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    QFontMetrics fm(font);

    // Geometry
    int width = fm.horizontalAdvance("About plainDE") + 25;
    int height = 150;
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


    // Font
    this->setFont(font);

    // Theme
    QFile stylesheetReader("/usr/share/plainDE/styles/" + stylesheet);
    stylesheetReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream styleSheet(&stylesheetReader);
    this->setStyleSheet(styleSheet.readAll() + "QPushButton { text-align: left; }");

    // Opacity
    this->setWindowOpacity(opacity);

    QVBoxLayout* userMenuLayout = new QVBoxLayout(this);
    userMenuLayout->setContentsMargins(1, 1, 1, 1);

    QPushButton* settingsEntry = new QPushButton();
    settingsEntry->setFlat(true);
    settingsEntry->setText(tr("Settings"));
    settingsEntry->setIcon(QIcon::fromTheme("preferences-system"));
    settingsEntry->setFont(font);
    userMenuLayout->addWidget(settingsEntry);

    QPushButton* aboutEntry = new QPushButton();
    aboutEntry->setFlat(true);
    aboutEntry->setText(tr("About plainDE"));
    aboutEntry->setIcon(QIcon::fromTheme("help-about"));
    aboutEntry->setFont(font);
    userMenuLayout->addWidget(aboutEntry);

    QPushButton* logOutEntry = new QPushButton();
    logOutEntry->setFlat(true);
    logOutEntry->setText(tr("Log Out"));
    logOutEntry->setIcon(QIcon::fromTheme("system-log-out"));
    logOutEntry->setFont(font);
    userMenuLayout->addWidget(logOutEntry);

    QPushButton* suspendEntry = new QPushButton();
    suspendEntry->setFlat(true);
    suspendEntry->setText(tr("Suspend"));
    suspendEntry->setIcon(QIcon::fromTheme("system-suspend"));
    suspendEntry->setFont(font);
    userMenuLayout->addWidget(suspendEntry);

    QPushButton* rebootEntry = new QPushButton();
    rebootEntry->setFlat(true);
    rebootEntry->setText("Reboot");
    rebootEntry->setIcon(QIcon::fromTheme("system-reboot"));
    rebootEntry->setFont(font);
    userMenuLayout->addWidget(rebootEntry);

    QPushButton* powerOffEntry = new QPushButton();
    powerOffEntry->setFlat(true);
    powerOffEntry->setText("Power Off");
    powerOffEntry->setIcon(QIcon::fromTheme("system-shutdown"));
    powerOffEntry->setFont(font);
    userMenuLayout->addWidget(powerOffEntry);

    // Make connections
    this->connect(powerOffEntry, &QPushButton::clicked, this,
                            [this, parentPanel]() {
        this->hide();

        QMessageBox powerOffMsg(this);
        powerOffMsg.setWindowTitle("Power Off");
        powerOffMsg.setText("Are you sure you want to shut down your system?");
        powerOffMsg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        powerOffMsg.setIcon(QMessageBox::Question);



        if (powerOffMsg.exec() == QMessageBox::Yes) {
            emit panelShouldQuit();
            this->connect(parentPanel, &Panel::animationFinished, this, [](){
                system("systemctl poweroff");
            });
        }
    });

    this->connect(rebootEntry, &QPushButton::clicked, this,
                            [this, parentPanel]() {
        this->hide();

        QMessageBox powerOffMsg(this);
        powerOffMsg.setWindowTitle("Reboot");
        powerOffMsg.setText("Are you sure you want to reboot your system?");
        powerOffMsg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        powerOffMsg.setIcon(QMessageBox::Question);

        if (powerOffMsg.exec() == QMessageBox::Yes) {
            emit panelShouldQuit();
            this->connect(parentPanel, &Panel::animationFinished, this, [](){
                system("systemctl reboot");
            });
        }
    });

    this->connect(logOutEntry, &QPushButton::clicked, this,
                            [this, parentPanel]() {
        this->hide();
        QMessageBox powerOffMsg(this);
        powerOffMsg.setWindowTitle("Log Out");
        powerOffMsg.setText("Are you sure you want to log out?");
        powerOffMsg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        powerOffMsg.setIcon(QMessageBox::Question);

        if (powerOffMsg.exec() == QMessageBox::Yes) {
            emit panelShouldQuit();
            this->connect(parentPanel, &Panel::animationFinished, this, [](){
                system("loginctl kill-user $USER");
            });
        }
    });

    this->connect(suspendEntry, &QPushButton::clicked, this,
                            [this]() {
        this->hide();
        system("systemctl suspend");
    });

    this->connect(settingsEntry, &QPushButton::clicked, this,
                            [execHolder, this]() {
        this->hide();

        QProcess* process = new QProcess(execHolder);
        process->start("plainControlCenter");
    });

    this->connect(aboutEntry, &QPushButton::clicked, this,
                            [execHolder, this]() {
        this->hide();

        QProcess* process = new QProcess(execHolder);
        process->start("plainAbout --plainPanel");
    });
}

UserMenu::UserMenu(Panel* parentPanel,
                   QObject* execHolder,
                   PanelLocation panelLocation,
                   int panelThickness,
                   int screenWidth,
                   int screenHeight,
                   QFont font,
                   int buttonCoord1,
                   int buttonCoord2,
                   QString stylesheet,
                   double opacity) : QWidget(nullptr) {
    createUI(parentPanel,
             execHolder,
             panelLocation,
             panelThickness,
             screenWidth,
             screenHeight,
             font,
             buttonCoord1,
             buttonCoord2,
             stylesheet,
             opacity);
}

UserMenu::~UserMenu() {

}
