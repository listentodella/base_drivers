adb wait-for-device root
adb remount

adb shell rm -rf /mnt/vendor/persist/sensors/registry/registry/*

adb push ./spi/kona_hdk_demo_0.json /vendor/etc/sensors/config/
adb push ./spi/kona_hdk_demo_1.json /vendor/etc/sensors/config/
adb shell sync

adb wait-for-device
adb root