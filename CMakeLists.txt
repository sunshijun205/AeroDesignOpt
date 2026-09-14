cmake_minimum_required(VERSION 3.16)

project(AeroDesignOpt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        ui/theme.h
        ui/uihelpers.h
        ui/uihelpers.cpp
        ui/datapage.h
        ui/datapage.cpp
        ui/ductdesignpage.h
        ui/ductdesignpage.cpp
        ui/visibilitypage.h
        ui/visibilitypage.cpp
        ui/surrogatepage.h
        ui/surrogatepage.cpp
        ui/optimizationpage.h
        ui/optimizationpage.cpp
        ui/resultspage.h
        ui/resultspage.cpp
        model/ductdataset.h
        model/ductparams.h
        service/datasetservice.h
        service/datasetservice.cpp
        service/ductdesignservice.h
        service/ductdesignservice.cpp
        controller/datapresenter.h
        controller/datapresenter.cpp
        controller/ductdesignpresenter.h
        controller/ductdesignpresenter.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AeroDesignOpt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(AeroDesignOpt SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(AeroDesignOpt
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(AeroDesignOpt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(AeroDesignOpt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/model
    ${CMAKE_CURRENT_SOURCE_DIR}/service
    ${CMAKE_CURRENT_SOURCE_DIR}/controller
)

if(MINGW)
    target_compile_options(AeroDesignOpt PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.AeroDesignOpt)
endif()
set_target_properties(AeroDesignOpt PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS AeroDesignOpt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AeroDesignOpt)
endif()
