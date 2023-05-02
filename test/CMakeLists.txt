# adding the Google_Tests_run target
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-std=c++20 -Wall -Werror=return-type -Werror=unused-variable -Werror=unused-parameter")
enable_testing()
add_executable(cpp_test SystemTest/SimpleTest.cpp)
add_executable(mswjTest SystemTest/MSWJTest.cpp)
add_executable(iawjTest SystemTest/IAWJTest.cpp)
add_executable(imaTest SystemTest/IMATest.cpp)
add_executable(pecjTest SystemTest/PECJTest.cpp)
add_library(TestFunction SystemTest/TestFunction.cpp)
message(STATUS " Libraries: ${LIBRARIES}")
target_link_libraries(TestFunction
        IntelliStream
        )
target_link_libraries(cpp_test
        IntelliStream
        TestFunction
        )
target_link_libraries(mswjTest
        IntelliStream
        TestFunction
        )
target_link_libraries(iawjTest
        IntelliStream
        TestFunction
        )
target_link_libraries(imaTest
        IntelliStream
        TestFunction
        )
target_link_libraries(pecjTest
        IntelliStream
        TestFunction
        )
# avoid undefined reference to "main" in test.cpp

add_test(
        NAME catch_test
        COMMAND $<TARGET_FILE:cpp_test> --success
)
add_test(
        NAME catchmswj_test
        COMMAND $<TARGET_FILE:mswjTest> --success
)

add_test(
        NAME catchima_test
        COMMAND $<TARGET_FILE:imaTest> --success
)

add_test(
        NAME catchpecj_test
        COMMAND $<TARGET_FILE:pecjTest> --success
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config_IMA.csv
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config_Normal.csv
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config_fileDataLoader.csv
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/datasets/rTuple.csv
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/datasets/sTuple.csv
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

