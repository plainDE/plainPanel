#include "clioutput.h"

void CLIOutputApplet::readConfig() {
    QString homeDir = QDir::homePath();
    QString filename = QString("%1/.config/plainDE/clioutput-applets/%2.json").arg(homeDir, mAppletName);
    QFile configReader(filename);
    configReader.open(QIODevice::ReadOnly | QIODevice::Text);
    QString data = configReader.readAll();
    configReader.close();
    mAppletConfig = QJsonDocument::fromJson(data.toUtf8()).object();

    mCommand = mAppletConfig["command"].toString();
    mWaitData = mAppletConfig["waitData"].toString().split(';');  // Icon + Text
    mAppletType = mAppletConfig["type"].toString();
    mInterval = mAppletConfig["interval"].toInt();
}

void CLIOutputApplet::externalWidgetSetup(ConfigManager* cfgMan,
                                          Panel* parentPanel) {
    mExternalWidget = new QPushButton();
    mExternalWidget->setFont(cfgMan->mFont);
    mExternalWidget->setFlat(true);
    mExternalWidget->setObjectName("cliOutputButton");

    connect(mExternalWidget, &QPushButton::clicked, this, [this, cfgMan, parentPanel]() {
        qDebug() << "CLI Output applet clicked";
        mTimer->stop();
        repeatingAction(cfgMan, parentPanel);
        activate(cfgMan, parentPanel);
    });
}

void CLIOutputApplet::activate(ConfigManager* cfgMan, Panel* parentPanel) {
    mTimer->setInterval(mInterval);
    connect(mTimer, &QTimer::timeout, this, [this, cfgMan, parentPanel]() {
        repeatingAction(cfgMan, parentPanel);
    });
    mTimer->start();
}

void CLIOutputApplet::repeatingAction(ConfigManager*, Panel*) {
    mProcess->start(mCommand);

    // Data that will be shown while command is running
    if (QIcon::hasThemeIcon(mWaitData[0])) {
        mExternalWidget->setIcon(QIcon::fromTheme(mWaitData[0]));
    }
    else {
        mExternalWidget->setIcon(QIcon(mWaitData[0]));
    }
    mExternalWidget->setText(mWaitData[1]);
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
            mExternalWidget->setIcon(QIcon::fromTheme(result[0]));
        }
        else {
            mExternalWidget->setIcon(QIcon(result[0]));
        }
        mExternalWidget->setText(result[1]);
    }
    else if (!mAppletType.compare("stdout")) {
        QString stdoutData = mProcess->readAllStandardOutput();
        mExternalWidget->setText(stdoutData);
    }
    else if (!mAppletType.compare("data")) {
        QStringList dataToShow = QString(mProcess->readAllStandardOutput()).split(';');
        if (QIcon::hasThemeIcon(dataToShow[0])) {
            mExternalWidget->setIcon(QIcon::fromTheme(dataToShow[0]));
        }
        else {
            mExternalWidget->setIcon(QIcon(dataToShow[0]));
        }
        mExternalWidget->setText(dataToShow[1]);
    }
    else {
        qDebug() << "Unknown" << mAppletName << "CLI Output Applet type (" << mAppletType << ")!";
    }
}

CLIOutputApplet::CLIOutputApplet(ConfigManager* cfgMan,
                                 Panel* parentPanel,
                                 QString additionalInfo) : Applet(cfgMan,
                                                                  parentPanel,
                                                                  additionalInfo) {
    mAppletName = additionalInfo;
    readConfig();

    mTimer = new QTimer(this);

    mProcess = new QProcess(parentPanel->mExecHolder);
    connect(mProcess,
            static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this] {
                setData();
            }
    );
}

CLIOutputApplet::~CLIOutputApplet() {

}
