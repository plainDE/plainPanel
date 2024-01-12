#include "volume.h"

void VolumeApplet::externalWidgetSetup(ConfigManager* cfgMan,
                                       Panel* parentPanel) {
    mExternalWidget = new QFrame();
    mExternalWidget->setObjectName("volumeFrame");
    mExternalWidget->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    QBoxLayout* layout;

    if (parentPanel->mPanelLayout == Horizontal) {
        layout = new QHBoxLayout(mExternalWidget);
    }
    else {  // Vertical
        layout = new QVBoxLayout(mExternalWidget);
    }

    layout->setSpacing(parentPanel->mSpacing);
    layout->setContentsMargins(0, 0, 0, 0);

    QDial* volumeDial = new QDial();
    volumeDial->setMinimum(0);
    volumeDial->setMaximum(100);
    if (cfgMan->mEnableOveramplification) {
        volumeDial->setMaximum(150);
    }
    volumeDial->setValue(cfgMan->mDefaultVolume);
    if (parentPanel->mPanelLayout == Horizontal) {
        volumeDial->setMaximumWidth(25);
    }
    else {  // Vertical
        volumeDial->setMaximumHeight(25);
    }
    layout->addWidget(volumeDial);

    QLabel* volumeLabel = new QLabel();
    volumeLabel->setFont(cfgMan->mFont);
    if (parentPanel->mPanelLayout == Horizontal) {
        volumeLabel->setMaximumWidth(parentPanel->mFontMetrics->horizontalAdvance("150%"));
    }
    QString text = QString("%1%").arg(QString::number(cfgMan->mDefaultVolume));
    volumeLabel->setText(text);
    volumeLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(volumeLabel);

    setVolume(cfgMan->mDefaultVolume, cfgMan->mVolumeAdjMethod);

    // Make connections
    this->connect(volumeDial, &QDial::valueChanged, this, [this, volumeDial,
                                                           cfgMan, volumeLabel]() {
        setVolume(volumeDial->value(), cfgMan->mVolumeAdjMethod);
        volumeLabel->setText(QString::number(volumeDial->value()) + "%");
    });
}

void VolumeApplet::setVolume(int newVolume, VolumeAdjustMethod method) {
    QString exec;
    if (method == ALSA) {
        exec = "amixer -q -D default sset Master " + QString::number(newVolume) + "%";
    }
    else if (method == PulseAudio) {
        exec = "pactl set-sink-volume @DEFAULT_SINK@ " + QString::number(newVolume) + "%";
    }

    QProcess* process = new QProcess;
    process->start(exec);
}

VolumeApplet::VolumeApplet(ConfigManager* cfgMan,
                           Panel* parentPanel,
                           QString additionalInfo) : Applet(cfgMan,
                                                            parentPanel,
                                                            additionalInfo) {

}

VolumeApplet::~VolumeApplet() {

}
