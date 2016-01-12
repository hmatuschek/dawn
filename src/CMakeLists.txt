set(dawn_SOURCES main.cc portdialog.cc dawn.cc dayofweekdelegate.cc timedelegate.cc
    mainwindow.cc configdialog.cc siphash24.c logger.cc dayofweekwidget.cc)
set(dawn_MOC_HEADERS portdialog.hh dawn.hh dayofweekdelegate.hh timedelegate.hh
    mainwindow.hh configdialog.hh dayofweekwidget.hh)
qt5_wrap_cpp(dawn_MOC_SOURCES ${dawn_MOC_HEADERS})

set(dawn_HEADERS siphash.h logger.hh ${dawn_MOC_HEADERS} )

qt5_add_resources(dawn_RCC_SOURCES ../resources.qrc)

if(NOT ${SECRET})
  message(STATUS "No shared secret specified (via -DSECRET=...).")
  exec_program(${PYTHON_EXECUTABLE}
               ARGS ${CMAKE_SOURCE_DIR}/gen_secret.py
               OUTPUT_VARIABLE SECRET)
  message(STATUS "Using generated shared secret: ${SECRET}")
else(NOT ${SECRET})
  message(STATUS "Using user supplied shared secret: ${SECRET}")
endif(NOT ${SECRET})

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/secret.h
  COMMAND ${PYTHON_EXECUTABLE}
  ARGS ${CMAKE_SOURCE_DIR}/gen_secret.py ${SECRET} ${CMAKE_BINARY_DIR}/secret.h
  DEPENDS ${CMAKE_SOURCE_DIR}/gen_secret.py)

set_property(SOURCE main.cc APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_BINARY_DIR}/secret.h)
add_executable(dawn ${dawn_SOURCES} ${dawn_MOC_SOURCES} ${dawn_RCC_SOURCES})
target_link_libraries(dawn ${LIBS})

