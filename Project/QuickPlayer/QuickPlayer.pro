SOURCES += \
    main.cpp \
    mainwindow.cpp \
    player.cpp \
    surface.cpp \
    register.cpp

HEADERS += \
    mainwindow.h \
    player.h \
    surface.h \
    register.h \
    def.h

QT += widgets gui multimedia network
LIBS +=  -pthread -lavdevice -lavfilter -lswscale -lpostproc -lavformat -lavcodec \
-lxcb-xfixes -lxcb-render -lxcb-shape   -lxcb -lXext -lXv -lX11 -lasound \
 -lx264 -lpthread -ldl -lfaac -lz -lswresample -lavutil -lm
