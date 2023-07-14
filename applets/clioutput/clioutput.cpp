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
            if (conditions.contains(stdoutData)) {
                result = conditions[stdoutData].toString().split(';');  // Icon + Text
            }
            else {
                result = mAppletConfig["elseCondition"].toString().split(';');  // Icon + Text
            }
        }
        else if (!condType.compare("exitcode")) {
            QString exitCode = QString::number(mProcess->exitCode());
            if (conditions.contains(exitCode)) {
                result = conditions[exitCode].toString().split(';');  // Icon + Text
            }
            else {
                result = mAppletConfig["elseCondition"].toString().split(';');  // Icon + Text
            }
        }

        if (QIcon::hasThemeIcon(result[0])) {
            this->setIcon(QIcon::fromTheme(result[0]));
        }
        else {
            this->setIcon(QIcon(result[0]));
        }
        this->setText(result[1]);
    }
    else if (!mAppletType.compare("stdout")) {
        QString stdoutData = mProcess->readAllStandardOutput();
        this->setText(stdoutData);
    }
    else if (!mAppletType.compare("data")) {
        QStringList dataToShow = QString(mProcess->readAllStandardOutput()).split(';');
        if (QIcon::hasThemeIcon(dataToShow[0])) {
            this->setIcon(QIcon::fromTheme(dataToShow[0]));
        }
        else {
            this->setIcon(QIcon(dataToShow[0]));
        }
        this->setText(dataToShow[1]);
    }
}

void CLIOutputApplet::getData() {
    mProcess->start(mCommand);

    // Data that will be shown while command is running
    if (QIcon::hasThemeIcon(mWaitData[0])) {
        this->setIcon(QIcon::fromTheme(mWaitData[0]));
    }
    else {
        this->setIcon(QIcon(mWaitData[0]));
    }
    this->setText(mWaitData[1]);
}

void CLIOutputApplet::activate() {
    getData();
    mTimer->start();
}

CLIOutputApplet::CLIOutputApplet(QObject* parent,
                                 QString appletName) : QPushButton(nullptr) {
    readConfig(appletName);

    this->setFlat(true);

    mWaitData = mAppletConfig["waitData"].toString().split(';');  // Icon + Text
    mAppletType = mAppletConfig["type"].toString();

    mCommand = mAppletConfig["command"].toString();
    mProcess = new QProcess(parent);
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

    connect(this, &QPushButton::clicked, this, [this]() {
        mTimer->stop();
        this->activate();
    });
}

CLIOutputApplet::~CLIOutputApplet() {

}
