set(msntest_SRCS
    msntest.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR})

add_executable (msntest ${msntest_SRCS} ) 
target_link_libraries(msntest crypto msn ssl)

########### install files ###############
install(TARGETS msntest DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

