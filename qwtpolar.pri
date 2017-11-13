#-------------------------------------------------
#
# QwtPolar Library configuration file
#
#-------------------------------------------------

POLAR_VER = 1.1.1

contains(QWT_VER,^5\\..*\\..*) {
    POLAR_VER = 0.1.0
    VER_SFX     = 0
    UNIX_SFX    = -qt4
} else {
    VER_SFX     =
    UNIX_SFX    =
}

win32 {
    win32-x-g++ {
        POLAR_PATH = /usr/qwtpolar$${VER_SFX}-win
    } else {
        POLAR_PATH = D:/Dev/Qt/QwtPolar-$${POLAR_VER}
    }
    POLAR_INC_PATH = $${POLAR_PATH}/include
    CONFIG(debug,debug|release) {
        DEBUG_SFX = d
    } else {
        DEBUG_SFX =
    }
    POLAR_LIB = qwtpolar$${DEBUG_SFX}$${VER_SFX}
}

INCLUDEPATH += $${POLAR_INC_PATH}
LIBS += -L$${POLAR_PATH}/lib -l$${POLAR_LIB}
