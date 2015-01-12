##---------------------------------------------------------------------------
## Author:      Sean D'Epagnier
## Copyright:   
## License:     GPLv3+
##---------------------------------------------------------------------------
 
# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(rotationctrl_pi)
SET(PACKAGE_NAME rotationctrl_pi)
SET(CPACK_PACKAGE_CONTACT "Sean D'Epagnier ")

SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "1")

INCLUDE("cmake/PluginConfigure.cmake")

SET(SRC_ROTATIONCTRL
            src/rotationctrl_pi.cpp
            src/PreferencesDialog.cpp
            src/RotationCtrlUI.cpp
            src/icons.cpp
	)
ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_ROTATIONCTRL})

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
