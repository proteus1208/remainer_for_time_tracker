cmake_minimum_required(VERSION 3.16)

project(Remainer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core)

set(PROJECT_SOURCES
        main.cpp
        eventmonitor.cpp
        eventmonitor.h
        lighteffectwidget.cpp
        lighteffectwidget.h
        spinningcircleswidget.cpp
        spinningcircleswidget.h
        blastwidget.cpp
        blastwidget.h
        opacityoverlaywidget.cpp
        opacityoverlaywidget.h
        coloredwidget.cpp
        coloredwidget.h
        settingsdialog.cpp
        settingsdialog.h
        remainerapp.cpp
        remainerapp.h
)

# Add Windows icon resource file if icon.ico exists
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/icon.ico")
    list(APPEND PROJECT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/app_icon.rc")
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Remainer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Remainer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Remainer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Remainer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Remainer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Remainer)
endif()
set_target_properties(Remainer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Windows executable icon is embedded via app_icon.rc file

include(GNUInstallDirs)
install(TARGETS Remainer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Remainer)
endif()

# Copy icon file to build directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/icon.ico")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/icon.ico" 
                   "${CMAKE_CURRENT_BINARY_DIR}/icon.ico" 
                   COPYONLY)
endif()
