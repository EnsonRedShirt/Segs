SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/AdminServer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AdminDatabase.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AccountInfo.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/CharacterDatabase.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/CharacterDatabase.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AccountInfo.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AdminDatabase.h
${CMAKE_CURRENT_SOURCE_DIR}/include/AdminServer.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(target_DEPENDS "")

SEGS_ADD_COMPONENT(cox_server_common LIB)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

ADD_LIBRARY(adminserver_lib STATIC ${target_SOURCES})

TARGET_LINK_LIBRARIES(adminserver_lib ${target_DEPENDS})
ADD_DEPENDENCIES(adminserver_lib ${target_DEPENDS})

SEGS_REGISTER_COMPONENT(adminserver_lib "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )
