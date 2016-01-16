SET QT_PATH=C:/Qt/Qt5.3.2
SET QT_BIN_PATH=%QT_PATH%/5.3/mingw482_32/bin
SET QT_INCLUDE_PATH=%QT_PATH%/Tools/mingw482_32/i686-w64-mingw32/include
SET MINGW_PATH=C:/MinGW
SET MINGW_BIN_PATH=%MINGW_PATH%/bin
SET PATH=%MINGW_BIN_PATH%;%QT_BIN_PATH%;%PATH%

cmake ../.. -G "MinGW Makefiles" -DENABLE_FIRMWARE=OFF -DSECRET= -DCMAKE_CXX_FLAGS="-I%QT_INCLUDE_PATH%"
mingw32-make
