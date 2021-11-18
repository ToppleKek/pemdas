cmake_minimum_required(VERSION 3.16)
project(pemdas)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(PEMDAS_ENABLE_DISCORDRPC "Enable Discord rich presence" ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Taglib REQUIRED)
find_package(CURL REQUIRED)
find_library(mpdclient REQUIRED)

include_directories(${TAGLIB_INCLUDES} ${CURL_INCLUDE_DIRS} src src/discord/include)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS forms)

add_executable(pemdas
        src/main.cpp
        src/pemdas.cpp
        src/pemdas.hpp
        src/player.cpp
        src/player.hpp
        src/mpd.hpp
        forms/pemdas.ui
        forms/albumwidget.ui
        src/statusbar.hpp
        src/statusbar.cpp
        src/utils.hpp
        res/res.qrc
        src/musicpage.cpp
        src/musicpage.hpp
        src/queue.cpp
        src/queue.hpp
        src/tablistwidget.hpp
        src/sidebar.cpp
        src/sidebar.hpp
        src/playlistpage.cpp
        src/playlistpage.hpp
        src/discord/discordrpc.cpp
        src/discord/discordrpc.hpp
        src/settingspage.cpp
        src/settingspage.hpp
        src/albumwidget.cpp
        src/albumwidget.hpp
        src/toolwidgetlabel.hpp)

set(libs Qt6::Widgets mpdclient ${TAGLIB_LIBRARIES} ${CURL_LIBRARIES})

if(PEMDAS_ENABLE_DISCORDRPC)
    message("Enabling DiscordRPC")
    message(${CMAKE_SOURCE_DIR})
    add_library(discordrpc SHARED IMPORTED)
    set_target_properties(discordrpc PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/discord/lib/libdiscord-rpc.so)
    set(libs ${libs} discordrpc)
    add_definitions(-DPEMDAS_ENABLE_DISCORDRPC)
endif()

target_link_libraries(pemdas ${libs})
