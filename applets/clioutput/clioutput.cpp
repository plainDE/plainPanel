#include "clioutput.h"


void CLIOutputApplet::readConfig(QString appletName) {
    QString homeDir = getenv("HOME");
    QFile configReader;
    QString data;
    configReader.setFileName(homeDir + "/.config/plainDE/clioutput-applets/" + appletName + ".json");
    configReader.open(QIODevice::ReadOnly | QIODevice::Text);
    data = configReader.readAll();
    configReader.close();
    mAppletConfig = QJsonDocument::fromJson(data.toUtf8()).object();
}

void CLIOutputApplet::setData() {
    if (!mAppletType.compare("condition")) {
        QStringList result;

        QString condType = mAppletConfig["conditionType"].toString();
        QJsonObject conditions = mAppletConfig["conditions"].toObject();
        if (!condType.compare("stdout")) {
            QString stdoutData = mProcess->readAllStandardOutput();
            result = conditions[stdoutData].toString().split(';');  // Icon + Text
        }
        else if (!condType.compare("exitcode")) {
            QString exitCode = QString::number(mProcess->exitCode());
            result = conditions[exitCode].toString().split(';');  // Icon + Text
        }

        if (QIcon::hasThemeIcon(result[0])) {
            mAppletButton->setIcon(QIcon::fromTheme(result[0]));
        }
        else {
            mAppletButton->setIcon(QIcon(result[0]));
        }
        mAppletButton->setText(result[1]);
    }
    else if (!mAppletType.compare("stdout")) {
        QString stdoutData = mProcess->readAllStandardOutput();
        mAppletButton->setText(stdoutData);
    }
}

void CLIOutputApplet::getData() {
    mProcess->start(mCommand);

    // Data that will be shown while command is running
    if (QIcon::hasThemeIcon(mWaitData[0])) {
        mAppletButton->setIcon(QIcon::fromTheme(mWaitData[0]));
    }
    else {
        mAppletButton->setIcon(QIcon(mWaitData[0]));
    }
    mAppletButton->setText(mWaitData[1]);
}

void CLIOutputApplet::activate() {
    getData();
    mTimer->start();
}

CLIOutputApplet::CLIOutputApplet(QWidget* parent,
                                 QString appletName) {
    readConfig(appletName);

    mAppletButton = new QPushButton();
    mAppletButton->setFlat(true);

    mWaitData = mAppletConfig["waitData"].toString().split(';');  // Icon + Text
    mAppletType = mAppletConfig["type"].toString();

    mCommand = mAppletConfig["command"].toString();
    mProcess = new QProcess(this);
    this->connect(mProcess,
                  static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                  this, [this] {
        setData();
    });

    mTimer = new QTimer(this);
    mTimer->setInterval(mAppletConfig["interval"].toInt());
    this->connect(mTimer, &QTimer::timeout, this, [this]() {
        getData();
    });
}

CLIOutputApplet::~CLIOutputApplet() {
    mTimer->stop();
    delete mTimer;
}
