SET(TARGET EnhanceLib2)

SET(WINPR_PATH    ${CMAKE_SOURCE_DIR}/3rdparty/winpr)
SET(WINPR_INCLUDE ${WINPR_PATH}/include)

INCLUDE_DIRECTORIES(
    ./
    ${CMAKE_SOURCE_DIR}/public/inc/EnhanceLib2/Interface
    ${CMAKE_SOURCE_DIR}/public/inc/EnhanceLib2/Interface/Linux
    ${CMAKE_SOURCE_DIR}/public/src/EnhanceLib2
    ${CMAKE_SOURCE_DIR}/public/inc/CTLogEx
    ${WINPR_INCLUDE}
    )

SET(CMAKE_C_FLAGS "-fPIC")
SET(CMAKE_CXX_FLAGS "-fPIC")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")

LIST(APPEND SRC_FILES
    ../Socket/SocketInterface.cpp
    ../Socket/SocketClient.cpp
    ../Socket/SocketServer.cpp
    ../Socket/SocketBase.cpp
    ../Socket/SocketHelper.cpp
    ../Common/Opini.cpp	
    ../Common/IOpini.cpp		
    ../Common/Thread.cpp
    ../Common/IThread.cpp
    ../Common/Cache.cpp
    ../Common/ICache.cpp
    ../Log/LogExBase.cpp
    ../Common/Communication.cpp
    ../Common/Buffer.cpp
    ../Common/IBuffer.cpp
    ../Common/IShareMemory.cpp
    ../Base/BaseObject.cpp	
    ../Linux/ShareMemory.cpp
    )

ADD_LIBRARY(${TARGET} STATIC ${SRC_FILES})



