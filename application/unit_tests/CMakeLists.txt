file(GLOB_RECURSE SRC *.cpp)
idf_component_register(
  SRCS
  ${SRC}
  PRIV_REQUIRES console spi_flash nvs_flash bt
  INCLUDE_DIRS
  ""
  REQUIRES
  main 
  WHOLE_ARCHIVE)

include(FetchContent)

set(INSTALL_GTEST
    OFF
    CACHE BOOL "" FORCE)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG main)
FetchContent_MakeAvailable(googletest)

target_link_libraries(${COMPONENT_LIB} INTERFACE GTest::gtest GTest::gmock)

# set(GTEST_DIR /home/dnl/Documents/git/googletest/googletest/include)
# set(GTEST_LIB /home/dnl/Documents/git/googletest/build/lib/libgtest.a /home/dnl/Documents/git/googletest/build/lib/libgtest_main.a)

# include_directories(${GTEST_DIR})
# target_link_directories(${COMPONENT_LIB} INTERFACE ${GTEST_LIB})
# 
