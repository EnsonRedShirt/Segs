SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/EventProcessor.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/SEGSTimer.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/EventProcessor.h
${CMAKE_CURRENT_SOURCE_DIR}/include/SEGSEvent.h
${CMAKE_CURRENT_SOURCE_DIR}/include/LinkLevelEvent.h
${CMAKE_CURRENT_SOURCE_DIR}/include/SEGSTimer.h
)

SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)


INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ACE_path})

ADD_LIBRARY(EventSystem STATIC ${target_SOURCES})
add_dependencies(EventSystem ACE_BUILD)

SEGS_REGISTER_COMPONENT(EventSystem "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )


