if (CMAKE_INSTALL_PREFIX STREQUAL /usr/local)
    set(CMAKE_INSTALL_PREFIX ${HOME_DIR}/output CACHE STRING "path for install()" FORCE)
    message(STATUS "No install prefix selected, default to ${CMAKE_INSTALL_PREFIX}.")
endif()

set(MOCKCPP_CXXFLAGS "-fPIC -std=c++17 -Wno-unused-parameter")
set(MOCKCPP_LDFLAGS "-Wl,-z,relro,-z,now,-z,noexecstack")
set(MOCKCPP_LINKER_FLAGS "")

include(ExternalProject)
ExternalProject_Add(mockcpp_build
    SOURCE_DIR ${THIRD_PARTY_DIR}/mockcpp
    CONFIGURE_COMMAND ${CMAKE_COMMAND}
        -G "${CMAKE_GENERATOR}"
        -DCMAKE_CXX_FLAGS=${MOCKCPP_CXXFLAGS}
        -DCMAKE_SHARED_LINKER_FLAGS=${MOCKCPP_LINKER_FLAGS}
        -DCMAKE_EXE_LINKER_FLAGS=${MOCKCPP_LINKER_FLAGS}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/mockcpp
        <SOURCE_DIR>
    BUILD_COMMAND $(MAKE)
    BUILD_ALWAYS TRUE
    EXCLUDE_FROM_ALL TRUE
)