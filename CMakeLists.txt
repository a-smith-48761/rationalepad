cmake_minimum_required(VERSION 3.21)
project(RationalePad VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)
qt_standard_project_setup()


qt_add_executable(RationalePad src/main.cpp)
target_link_libraries(RationalePad PRIVATE Qt6::Widgets Qt6::Sql)
# set_target_properties(RationalePad PROPERTIES WIN32_EXECUTABLE TRUE)

# Ensure the static Windows binary contains SQLite, not optional SQL drivers.
qt_import_plugins(RationalePad
    INCLUDE_BY_TYPE sqldrivers Qt6::QSQLiteDriverPlugin
)
install(TARGETS RationalePad RUNTIME DESTINATION .)
