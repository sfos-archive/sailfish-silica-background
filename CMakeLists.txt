cmake_minimum_required(VERSION 3.10)
project(sailfishsilicabackground)

# Find Qt5 packages
find_package(Qt5 COMPONENTS Core Gui REQUIRED)

# Find mlite5 package
find_package(PkgConfig REQUIRED)
pkg_check_modules(MLITE5 REQUIRED IMPORTED_TARGET mlite5)

# Set version
set(VERSION_MAJOR 0)
set(VERSION_MINOR 9)
set(VERSION_PATCH 16)

# Process Qt resource files
qt5_add_resources(QRC_SOURCES resources.qrc)

# Add library
add_library(sailfishsilicabackground-qt5 SHARED
    colorlookup.cpp
    gaussianblurcalculator.cpp
    sailfishsilicabackground.cpp
    ${QRC_SOURCES}
)

# Link Qt5 libraries
target_link_libraries(sailfishsilicabackground-qt5
    Qt5::Core
    Qt5::Gui
    PkgConfig::MLITE5
)

# Set library properties
set_target_properties(sailfishsilicabackground-qt5 PROPERTIES
    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
    SOVERSION ${VERSION_MAJOR}
    OUTPUT_NAME sailfishsilicabackground-qt5
)

# Install rules (optional)
install(TARGETS sailfishsilicabackground-qt5
    LIBRARY DESTINATION lib
)