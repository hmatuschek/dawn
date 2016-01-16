set(dawn_SOURCES main.cc portdialog.cc dawn.cc timedelegate.cc
    mainwindow.cc configdialog.cc siphash24.c logger.cc dayofweekwidget.cc alarmsettingwidget.cc)
set(dawn_MOC_HEADERS portdialog.hh dawn.hh timedelegate.hh
    mainwindow.hh configdialog.hh dayofweekwidget.hh alarmsettingwidget.hh)
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
target_link_libraries(dawn ${LIBS})

# Create "Application Bundle" under MacOS X
if(UNIX AND APPLE)
 install(TARGETS dawn DESTINATION /Applications/Dawn.app/Contents/MacOS)
 install(FILES ../shared/icons.icns DESTINATION /Applications/Dawn.app/Contents)
 install(FILES ../shared/Info.plist DESTINATION /Applications/Dawn.app/Contents)
endif(UNIX AND APPLE)
