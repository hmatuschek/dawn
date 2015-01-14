set(lampe_SOURCES main.cc portdialog.cc dawn.cc dayofweekdelegate.cc timedelegate.cc
    mainwindow.cc configdialog.cc siphash24.c)
set(lampe_MOC_HEADERS portdialog.hh dawn.hh dayofweekdelegate.hh timedelegate.hh
    mainwindow.hh configdialog.hh)
qt5_wrap_cpp(lampe_MOC_SOURCES ${lampe_MOC_HEADERS})

set(lampe_HEADERS siphash.h ${lampe_MOC_HEADERS} )

qt5_add_resources(lampe_RCC_SOURCES ../resources.qrc)

add_executable(lampe ${lampe_SOURCES} ${lampe_MOC_SOURCES} ${lampe_RCC_SOURCES})
target_link_libraries(lampe ${LIBS})

