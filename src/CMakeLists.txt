#
# Build base library (static)
#
set(libdawn_SOURCES siphash24.c logger.cc option_parser.cc devicesettings.cc
    dawn.cc logmessagetable.cc dawndiscover.cc)
set(libdawn_MOC_HEADERS
    dawn.hh logmessagetable.hh dawndiscover.hh)
qt5_wrap_cpp(libdawn_MOC_SOURCES ${libdawn_MOC_HEADERS})
set(libdawn_HEADERS ${qfcgi_HEADERS} siphash.h logger.hh option_parser.hh  devicesettings.hh
    ${libdawn_MOC_HEADERS})

add_library(libdawn STATIC ${libdawn_SOURCES} ${libdawn_MOC_SOURCES})
set_target_properties(libdawn PROPERTIES OUTPUT_NAME "dawn")
target_link_libraries(libdawn ${LIBS})


#
# Build GUI application
#
set(dawn_SOURCES main.cc
    portdialog.cc mainwindow.cc configdialog.cc dayofweekwidget.cc alarmsettingwidget.cc
    logmessagetable.cc)
set(dawn_MOC_HEADERS
    portdialog.hh mainwindow.hh configdialog.hh dayofweekwidget.hh alarmsettingwidget.hh
    logmessagetable.hh)
qt5_wrap_cpp(dawn_MOC_SOURCES ${dawn_MOC_HEADERS})
set(dawn_HEADERS siphash.h logger.hh ${dawn_MOC_HEADERS} )

SET(LANGUAGES de_DE)
SET(LANGUAGE_TS_FILES)
FOREACH(LANGUAGE ${LANGUAGES})
  SET(TS_FILE "${CMAKE_SOURCE_DIR}/i18n/dawn_${LANGUAGE}.ts")
  SET(LANGUAGE_TS_FILES ${LANGUAGE_TS_FILES} ${TS_FILE})
  SET_SOURCE_FILES_PROPERTIES(${TS_FILE} PROPERTIES OUTPUT_LOCATION
    "${CMAKE_SOURCE_DIR}/i18n")
ENDFOREACH(LANGUAGE ${LANGUAGES})

IF(LANGUAGE_TS_FILES)
  QT5_CREATE_TRANSLATION(dawn_QM_FILES ${dawn_SOURCES} ${LANGUAGE_TS_FILES})
ENDIF(LANGUAGE_TS_FILES)

qt5_add_resources(dawn_RCC_SOURCES ../resources.qrc)

add_executable(dawn ${dawn_SOURCES} ${dawn_MOC_SOURCES} ${dawn_RCC_SOURCES} ${dawn_QM_FILES})
target_link_libraries(dawn libdawn ${LIBS})

# Create "Application Bundle" under MacOS X
if(UNIX AND APPLE)
 install(TARGETS dawn DESTINATION /Applications/Dawn.app/Contents/MacOS)
 install(FILES ../shared/icons.icns DESTINATION /Applications/Dawn.app/Contents)
 install(FILES ../shared/Info.plist DESTINATION /Applications/Dawn.app/Contents)
endif(UNIX AND APPLE)


#
# Build command line tool
#

set(dawn_cmd_SOURCES dawncmd.cc)
set(dawn_cmd_MOC_HEADERS )
set(dawn_cmd_HEADERS ${dawn_MOC_HEADERS})

add_executable(dawncmd ${dawn_cmd_SOURCES} ${dawn_cmd_MOC_SOURCES} ${dawn_cmd_RCC_SOURCES}
  ${dawn_cmd_QM_FILES})
target_link_libraries(dawncmd libdawn ${LIBS})

install(TARGETS dawncmd DESTINATION bin/)


#
# Build FCGI application
#
set(qfcgi_SOURCES qfcgi/connection.cpp qfcgi/fcgi.cpp qfcgi/fdbuilder.cpp qfcgi/localbuilder.cpp
    qfcgi/record.cpp qfcgi/request.cpp qfcgi/stream.cpp qfcgi/tcpbuilder.cpp)
set(qfcgi_MOC_HEADERS qfcgi/connection.h qfcgi/fcgi.h qfcgi/fdbuilder.h qfcgi/localbuilder.h
    qfcgi/request.h qfcgi/stream.h qfcgi/tcpbuilder.h qfcgi/builder.h)
set(qfcgi_HEADERS qfcgi.h qfcgi/record.h)

set(dawnd_SOURCES dawnd.cc)
set(dawnd_MOC_HEADERS dawnd.hh)
set(dawnd_HEADERS ${dawnd_MOC_HEADERS})

qt5_wrap_cpp(dawnd_MOC_SOURCES ${dawnd_MOC_HEADERS} ${qfcgi_MOC_HEADERS})
add_executable(dawnd ${dawnd_SOURCES} ${qfcgi_SOURCES} ${dawnd_MOC_SOURCES} ${dawnd_RCC_SOURCES}
  ${dawnd_QM_FILES})
target_link_libraries(dawnd libdawn ${LIBS})

install(TARGETS dawnd DESTINATION bin/)
