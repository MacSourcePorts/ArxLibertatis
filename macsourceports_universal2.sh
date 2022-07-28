# game/app specific values
export APP_VERSION="1.2.1"
export ICONSDIR="data/icons"
export ICONSFILENAME="arx-libertatis"
export PRODUCT_NAME="arx-libertatis"
export EXECUTABLE_NAME="arx"
export PKGINFO="APPLARX"
export COPYRIGHT_TEXT="Arx Fatalis © 2002 Arkane Studios. All rights reserved."

#constants
source ../MSPScripts/constants.sh

export HIGH_RESOLUTION_CAPABLE="true"

rm -rf ${BUILT_PRODUCTS_DIR}

# create makefiles with cmake, perform builds with make
rm -rf ${X86_64_BUILD_FOLDER}
mkdir ${X86_64_BUILD_FOLDER}
mkdir -p ${X86_64_BUILD_FOLDER}/${EXECUTABLE_FOLDER_PATH}
cd ${X86_64_BUILD_FOLDER}
cmake \
-DCMAKE_OSX_ARCHITECTURES=x86_64 \
-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
-DCMAKE_PREFIX_PATH=/usr/local \
-DCMAKE_INSTALL_PREFIX=/usr/local \
..
make
mv arx ${EXECUTABLE_FOLDER_PATH}

cd ..
rm -rf ${ARM64_BUILD_FOLDER}
mkdir ${ARM64_BUILD_FOLDER}
mkdir -p ${ARM64_BUILD_FOLDER}/${EXECUTABLE_FOLDER_PATH}
cd ${ARM64_BUILD_FOLDER}
cmake  \
-DCMAKE_OSX_ARCHITECTURES=arm64 \
-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
-DCMAKE_PREFIX_PATH=/opt/Homebrew \
-DCMAKE_INSTALL_PREFIX=/opt/Homebrew \
..
make
mv arx ${EXECUTABLE_FOLDER_PATH}


cd ..

# create the app bundle
"../MSPScripts/build_app_bundle.sh"

source ../MSPScripts/signing_values.local

#sign and notarize
"../MSPScripts/sign_and_notarize.sh" "$1" 