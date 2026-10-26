cmake_minimum_required(VERSION 3.16)

# This project builds a native Vita executable through VitaSDK. It intentionally
# packages the selected LiveArea artwork; game scripts, archives and music are
# loaded from the user's data directory.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "VitaSDK toolchain")
  else()
    message(FATAL_ERROR "Set VITASDK to a VitaSDK installation before configuring this project.")
  endif()
endif()

project(TsukihimeVita LANGUAGES C CXX)

include("${VITASDK}/share/vita.cmake" REQUIRED)

# `onsjh` is the native ONScripter-JH interpreter. Its Vita entry point is
# configured to load user-provided data from ux0:data/tsukihime/game.
add_subdirectory(engine/src/onsjh)

set(TSUKIHIME_APP_NAME "Tsukihime")
set(TSUKIHIME_TITLE_ID "TSUKI0001")
set(TSUKIHIME_VERSION "01.04")
set(TSUKIHIME_VPK "${CMAKE_BINARY_DIR}/TsukihimeVita.vpk")

# VitaSDK's macro changed the custom-target name in CMake 3.20.
if(${CMAKE_VERSION} VERSION_LESS "3.20.0")
  set(TSUKIHIME_RUNTIME_TARGET onsjh.self)
else()
  set(TSUKIHIME_RUNTIME_TARGET onsjh.self-self)
endif()

add_custom_target(vpk
  COMMAND vita-mksfoex
    -s APP_VER=${TSUKIHIME_VERSION}
    -s TITLE_ID=${TSUKIHIME_TITLE_ID}
    "${TSUKIHIME_APP_NAME}"
    "${CMAKE_BINARY_DIR}/param.sfo"
  COMMAND vita-pack-vpk
    -s "${CMAKE_BINARY_DIR}/param.sfo"
    -b "${CMAKE_BINARY_DIR}/engine/src/onsjh/onsjh.self"
    -a "${CMAKE_CURRENT_SOURCE_DIR}/assets/livearea/icon0.png=sce_sys/icon0.png"
    -a "${CMAKE_CURRENT_SOURCE_DIR}/assets/livearea/bg.png=sce_sys/livearea/contents/bg.png"
    -a "${CMAKE_CURRENT_SOURCE_DIR}/assets/livearea/startup.png=sce_sys/livearea/contents/startup.png"
    -a "${CMAKE_CURRENT_SOURCE_DIR}/assets/livearea/template.xml=sce_sys/livearea/contents/template.xml"
    "${TSUKIHIME_VPK}"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${TSUKIHIME_VPK}" "${CMAKE_BINARY_DIR}/TsukihimeVita-${TSUKIHIME_VERSION}.vpk"
  DEPENDS ${TSUKIHIME_RUNTIME_TARGET}
  COMMENT "Packaging the native Tsukihime Vita VPK"
  VERBATIM
)
