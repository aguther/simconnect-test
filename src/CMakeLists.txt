
include_directories(
        "$ENV{MSFS_SDK}/SimConnect SDK/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

add_executable(
        SimConnectTest
        main.cpp
)
target_link_libraries(
        SimConnectTest PRIVATE
        SimConnect
)
add_custom_command(
        TARGET SimConnectTest
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/bin"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:SimConnectTest>/SimConnectTest.exe" "${CMAKE_SOURCE_DIR}/bin"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{MSFS_SDK}/SimConnect SDK/lib/SimConnect.dll" "${CMAKE_SOURCE_DIR}/bin"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/external/SimConnect/SimConnect.cfg" "${CMAKE_SOURCE_DIR}/bin"
)
