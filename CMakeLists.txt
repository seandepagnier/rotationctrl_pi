##---------------------------------------------------------------------------
## Author:      Sean D'Epagnier
## Copyright:   
## License:     GPLv3+
##---------------------------------------------------------------------------
 
# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

SET(VERBOSE_NAME RotationCtrl)
SET(TITLE_NAME RotationCtrl)
PROJECT(rotationctrl_pi)
SET(PACKAGE_NAME rotationctrl_pi)
SET(VERBOSE_NAME RotationCtrl)
SET(TITLE_NAME ROTATIONCTRL)
SET(CPACK_PACKAGE_CONTACT "Sean D'Epagnier ")

INCLUDE("cmake/PluginConfigure.cmake")

SET(SRC_ROTATIONCTRL
            src/rotationctrl_pi.cpp
            src/PreferencesDialog.cpp
            src/RotationCtrlUI.cpp
            src/icons.cpp
	)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_ROTATIONCTRL})

INCLUDE("cmake/PluginNMEA.cmake")
INCLUDE("cmake/PluginJSON.cmake")

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
