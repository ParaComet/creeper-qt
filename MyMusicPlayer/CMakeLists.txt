cmake_minimum_required(VERSION 3.28)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(MyMusicPlayer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_AUTOMOC ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    Debug
    Release
    RelWithDebInfo
    MinSizeRel
)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia Sql)

if(NOT TARGET creeper-qt::widgets)
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../CMakeLists.txt"
       AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/../src/creeper-qt/core/application.hh")
        add_subdirectory(
            "${CMAKE_CURRENT_LIST_DIR}/.."
            "${CMAKE_BINARY_DIR}/_deps/creeper-qt-local"
            EXCLUDE_FROM_ALL
        )
    endif()
endif()

if(NOT TARGET creeper-qt::widgets)
    include(FetchContent)
    FetchContent_Declare(
        creeper-qt
        GIT_REPOSITORY https://github.com/creeper5820/creeper-qt.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(creeper-qt)
endif()

add_executable(${PROJECT_NAME}
    src/main.cc
    src/mycomponet/PlaylistItem.cc
    src/mycomponet/SongItem.cc
    src/services/library_service.cc
    src/widgets/all_music_page.cc
    src/widgets/home_window.cc
    src/widgets/playlist_page.cc
    src/widgets/player_bar.cc
    src/widgets/scan_page.cc
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Sql
    creeper-qt::widgets
)

include(CheckIPOSupported)
option(MYMUSICPLAYER_ENABLE_IPO "Enable interprocedural optimization for release-like builds" ON)

if(MYMUSICPLAYER_ENABLE_IPO)
    check_ipo_supported(RESULT mymusicplayer_ipo_supported OUTPUT mymusicplayer_ipo_error)
    if(mymusicplayer_ipo_supported)
        set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
        set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
    else()
        message(STATUS "IPO/LTO not enabled for ${PROJECT_NAME}: ${mymusicplayer_ipo_error}")
    endif()
endif()
