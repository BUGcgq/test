cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# 设置项目名
project(test)

# 设置编译器，如果提示CXX错误 运行 export CXX=/usr/local/arm_linux_4.8/bin/arm-linux-g++
set(CMAKE_C_COMPILER "/usr/local/arm_linux_4.8/bin/arm-linux-gcc")
set(CMAKE_CXX_COMPILER /usr/local/arm_linux_4.8/bin/arm-linux-g++)


# 设置C++标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")#可以加-g
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/target)

# 设置CMake模块路径
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")


# 设置库文件路径
set(MODBUS_LIBRARY_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/lib/libmodbus-3.1.6-nuc970)
set(CURL_LIBRARY_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcurl)
set(MQTT_LIBRARY_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/lib/libpaho-mqtt3)
set(EVENT_LIBRARY_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/lib/libevent)
set(SSH2_LIBRARY_DIRS  ${CMAKE_CURRENT_SOURCE_DIR}lib/libssh2)
set(OPENSSL_LIBRARY_DIRS /opt/openssl-1.1.0g-nuc970)
set(SQLITE3_LIBRARY_DIRS /opt/sqlite-3.33-nuc970)

# 添加链接库搜索路径
link_directories(
    ${OPENSSL_LIBRARY_DIRS}/lib
    ${SSH2_LIBRARY_DIRS}/lib
    ${CURL_LIBRARY_DIRS}/lib
    ${MQTT_LIBRARY_DIRS}/lib
    ${EVENT_LIBRARY_DIRS}/lib
    ${MODBUS_LIBRARY_DIRS}/lib
    ${SQLITE3_LIBRARY_DIRS}/lib
)

# 添加头文件搜索路径
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/inc
    ${OPENSSL_LIBRARY_DIRS}/include
    ${CURL_LIBRARY_DIRS}/include
    ${MQTT_LIBRARY_DIRS}/include
    ${EVENT_LIBRARY_DIRS}/include
    ${MODBUS_LIBRARY_DIRS}/include
    ${SQLITE3_LIBRARY_DIRS}/include
    ${SSH2_LIBRARY_DIRS}/include
)


# 添加宏定义
add_definitions(-DNETWORK -DCGQ)  # 添加名为DEBUG的宏定义


# 添加 src 源文件
set(SRC_SOURCES
    src/main.c
#    src/APIS_CcuComm.c
#    src/IncComType.c
#    src/GBBMSComm.c
#   src/thpool.c
#   src/ToolFunc.c
   src/log.c
#   src/tlog.c
#    src/ocpp_chargeOrder.c
#   src/event_framework.c
#   src/CFG_prontocolinfo.c
#   src/CJSON.c
#   src/APIS_Json.c
   src/ftp.c
#   src/base64.c
#   src/mqtt.c
#   src/list.c
)


# 生成可执行文件
add_executable(${PROJECT_NAME}
    ${SRC_SOURCES}
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    pthread
    z
    m
    dl
#    event
#    paho-mqtt3cs
    ssl
    crypto
    ssh2
    curl
#    sqlite3

)
