# ExifOrientationFix

A simple tool to apply the EXIF orientation directly to images for use with web browsers or other tools that do not understand EXIF metatdata.


# Build Instructions

## Requirements
* CMAKE
* Essential build tools
* Qt5

## Steps
* compile or install Qt library + add bin directory to path
* run cmake on the project, set QT_ROOT_PREFIX to install directory of QT
* build
* optional (windows/mac): build package project (requires NSIS on windows)

# Usage

* just build the INSTALL target project or the PACKAGING target (needs NSIS on windows)

