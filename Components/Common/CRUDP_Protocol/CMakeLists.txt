SET(target_CPP
${CMAKE_CURRENT_SOURCE_DIR}/src/CRUDP_Packet.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/CRUDP_Protocol.cpp
#${CMAKE_CURRENT_SOURCE_DIR}/src/GameProtocol.cpp
#${CMAKE_CURRENT_SOURCE_DIR}/src/GameProtocolHandler.cpp
#${CMAKE_CURRENT_SOURCE_DIR}/src/PacketBase.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/PacketCodec.cpp
)
SET(target_INCLUDE
${CMAKE_CURRENT_SOURCE_DIR}/include/CRUDP_Packet.h
${CMAKE_CURRENT_SOURCE_DIR}/include/CRUDP_Protocol.h
#${CMAKE_CURRENT_SOURCE_DIR}/include/GameProtocol.h
#${CMAKE_CURRENT_SOURCE_DIR}/include/GameProtocolHandler.h
#${CMAKE_CURRENT_SOURCE_DIR}/include/PacketBase.h
${CMAKE_CURRENT_SOURCE_DIR}/include/PacketCodec.h
)


SET(target_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

SEGS_ADD_COMPONENT(Bitstream SOURCE)

SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ACE_path})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

SEGS_REGISTER_COMPONENT(CRUDP_Protocol "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )


