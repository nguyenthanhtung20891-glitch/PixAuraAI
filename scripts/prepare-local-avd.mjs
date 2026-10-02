// Isolated test emulator; never modify a user's existing AVD.
import fs from 'node:fs';
import path from 'node:path';
const avdRoot = path.resolve('build/android-avd');
const directory = path.join(avdRoot, 'pixaura-shell.avd');
const sdkPath = process.env.ANDROID_HOME;
if (!sdkPath) throw new Error('Set ANDROID_HOME to the installed Android SDK');
const imagePath = path.resolve(sdkPath, 'system-images/android-36.1/google_apis_playstore/x86_64');
if (!fs.existsSync(path.join(imagePath, 'system.img'))) throw new Error('Install Android 36.1 Google Play x86_64 image');
fs.mkdirSync(directory, { recursive: true });
fs.writeFileSync(path.join(avdRoot, 'pixaura-shell.ini'), `avd.ini.encoding=UTF-8\npath=${directory}\ntarget=android-36.1\n`);
fs.writeFileSync(path.join(directory, 'config.ini'), `avd.ini.encoding=UTF-8
AvdId=pixaura-shell
avd.ini.displayname=PixAura Phase 1 Test
image.sysdir.1=${imagePath.replaceAll('\\', '/')}/
abi.type=x86_64
tag.id=google_apis_playstore
tag.display=Google Play
target=android-36.1
hw.cpu.arch=x86_64
hw.cpu.ncore=2
hw.ramSize=2048
hw.lcd.width=480
hw.lcd.height=800
hw.lcd.density=160
hw.gpu.enabled=yes
hw.gpu.mode=swiftshader_indirect
hw.keyboard=yes
hw.mainKeys=no
disk.dataPartition.size=2G
fastboot.forceColdBoot=yes
PlayStore.enabled=true
`);
