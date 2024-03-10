#include "battery.h"

QDBusConnection mBatteryBus = QDBusConnection::systemBus();

bool BatteryApplet::deviceHasBattery() {
    QDir powerSupplyDir("/sys/class/power_supply");
    QStringList devices = powerSupplyDir.entryList();
    foreach (QString device, devices) {
        if (device.startsWith("BAT")) {
            return true;
        }
    }
    return false;
}

void BatteryApplet::externalWidgetSetup() {
    mExternalWidget = new QFrame();
    static_cast<QFrame*>(mExternalWidget)->setFrameStyle(
        QFrame::NoFrame | QFrame::Plain);
    QBoxLayout* layout;

    if (mParentPanel->mPanelLayout == Horizontal) {
        layout = new QHBoxLayout(mExternalWidget);
    }
    else {  // Vertical
        layout = new QVBoxLayout(mExternalWidget);
    }

    layout->setSpacing(mParentPanel->mSpacing);
    layout->setContentsMargins(0, 0, 0, 0);

    //updateBatteryState();

    mIconLabel = new QLabel();
    mIconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(mIconLabel);
    mTextLabel = new QLabel();
    mTextLabel->setFont(mCfgMan->mFont);
    mTextLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(mTextLabel);

    // mBatteryBus.connect("org.freedesktop.UPower",
    //                     "/org/freedesktop/UPower/devices/battery_" + mBatteryName,
    //                     "org.freedesktop.DBus.Properties",
    //                     "PropertiesChanged",
    //                     this,
    //                     SLOT(updateBatteryInfo(QVariant)));
}

void BatteryApplet::setBatteryName() {
    QDir powerSupplyDir("/sys/class/power_supply");
    QStringList devices = powerSupplyDir.entryList();
    foreach (QString device, devices) {
        if (device.startsWith("BAT")) {
            mBatteryName = device;
            break;
        }
    }
}

void BatteryApplet::repeatingAction() {
    // https://github.com/openwebos/qt/blob/master/tools/qdbus/qdbus/qdbus.cpp
    QString path = "/org/freedesktop/UPower/devices/battery_" + mBatteryName;
    QDBusInterface iface("org.freedesktop.UPower",
                         path,
                         "org.freedesktop.DBus.Properties",
                         mBatteryBus);
    mResponse = iface.call("Get", "org.freedesktop.UPower.Device", "Percentage");
    foreach (QVariant v, mResponse.arguments()) {
        if (v.userType() == qMetaTypeId<QDBusVariant>()) {
            mBattery.percentage = qvariant_cast<QDBusVariant>(v).variant().toInt();
        }
    }

    mResponse = iface.call("Get", "org.freedesktop.UPower.Device", "IconName");
    foreach (QVariant v, mResponse.arguments()) {
        if (v.userType() == qMetaTypeId<QDBusVariant>()) {
            mBattery.iconName = qvariant_cast<QDBusVariant>(v).variant().toString();
        }
    }

    mTextLabel->setText(QString("%1%").arg(QString::number(mBattery.percentage)));
    QString iconName = mBattery.iconName;
    mIconLabel->setToolTip(iconName);
    mTextLabel->setToolTip(iconName);
    if (mDark) {
        iconName += "-dark";
    }
    mIconLabel->setPixmap(mCache[iconName]);
}

void BatteryApplet::cacheIcons() {
    QDir iconsDir("/usr/share/plainDE/icons");
    QStringList iconList = iconsDir.entryList();
    foreach (QString iconName, iconList) {
        QString name = iconName;
        name.chop(4);  // removing '.png' at the end of the filename
        mCache[name] = QIcon(QString("/usr/share/plainDE/icons/%1.png").arg(
                                 name)).pixmap(mIconSize, mIconSize);
        mCache[name + "-dark"] = QIcon(QString("/usr/share/plainDE/icons/%1-dark.png").arg(
                                           name)).pixmap(mIconSize, mIconSize);
    }
}

BatteryApplet::BatteryApplet(ConfigManager* cfgMan,
                             Panel* parentPanel) : DynamicApplet(
                                                       "org.plainDE.battery",
                                                       cfgMan,
                                                       parentPanel,
                                                       5000
                                                   ) {
    mIconSize = parentPanel->mPanelThickness / 1.45;
    mDark = cfgMan->mStylesheet.contains("dark");
    setBatteryName();
    cacheIcons();
}

BatteryApplet::~BatteryApplet() {

}
