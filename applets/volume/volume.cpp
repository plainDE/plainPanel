#include "volume.h"

void VolumeApplet::externalWidgetSetup() {
    mExternalWidget = new QFrame();
    mExternalWidget->setObjectName("volumeFrame");
    static_cast<QFrame*>(mExternalWidget)->setFrameStyle(
        QFrame::NoFrame | QFrame::Plain
    );
    QBoxLayout* layout;

    if (mParentPanel->mPanelLayout == Horizontal) {
        layout = new QHBoxLayout(mExternalWidget);
    }
    else {  // Vertical
        layout = new QVBoxLayout(mExternalWidget);
    }

    layout->setSpacing(mParentPanel->mSpacing);
    layout->setContentsMargins(0, 0, 0, 0);

    QDial* volumeDial = new QDial();
    volumeDial->setMinimum(0);
    volumeDial->setMaximum(100);
    if (mCfgMan->mEnableOveramplification) {
        volumeDial->setMaximum(150);
    }
    volumeDial->setValue(mCfgMan->mDefaultVolume);
    if (mParentPanel->mPanelLayout == Horizontal) {
        volumeDial->setMaximumWidth(25);
    }
    else {  // Vertical
        volumeDial->setMaximumHeight(25);
    }
    layout->addWidget(volumeDial);

    QLabel* volumeLabel = new QLabel();
    volumeLabel->setFont(mCfgMan->mFont);
    if (mParentPanel->mPanelLayout == Horizontal) {
        volumeLabel->setMaximumWidth(mParentPanel->mFontMetrics->horizontalAdvance("150%"));
    }
    QString text = QString("%1%").arg(QString::number(mCfgMan->mDefaultVolume));
    volumeLabel->setText(text);
    volumeLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(volumeLabel);

    setVolume(mCfgMan->mDefaultVolume, mCfgMan->mVolumeAdjMethod);

    // Make connections
    this->connect(volumeDial, &QDial::valueChanged, this, [this, volumeDial,
                                                           volumeLabel]() {
        setVolume(volumeDial->value(), mCfgMan->mVolumeAdjMethod);
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
                           Panel* parentPanel) : StaticApplet(
                                                    "org.plainDE.volume",
                                                     cfgMan,
                                                     parentPanel
                                                 ) {

}

VolumeApplet::~VolumeApplet() {

}
