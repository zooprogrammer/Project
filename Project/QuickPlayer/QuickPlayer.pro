SOURCES += \
    main.cpp \
    mainwindow.cpp \
    player.cpp \
    surface.cpp

HEADERS += \
    mainwindow.h \
    player.h \
    surface.h

QT += widgets gui multimedia
LIBS +=  -pthread -lavdevice -lavfilter -lswscale -lpostproc -lavformat -lavcodec \
-lxcb-xfixes -lxcb-render -lxcb-shape   -lxcb -lXext -lXv -lX11 -lasound \
 -lx264 -lpthread -ldl -lfaac -lz -lswresample -lavutil -lm
