cmake_minimum_required(VERSION 3.1)
# SQLite3 support required for MBTiles.

set(SRC
    src/Backup.cpp
    src/Column.cpp
    src/Database.cpp
    src/Exception.cpp
    src/Statement.cpp
    src/Transaction.cpp
)
# list of header files of the library
set(INCL
    include/SQLiteCpp/SQLiteCpp.h
    include/SQLiteCpp/Assertion.h
    include/SQLiteCpp/Backup.h
    include/SQLiteCpp/Column.h
    include/SQLiteCpp/Database.h
    include/SQLiteCpp/Exception.h
    include/SQLiteCpp/Statement.h
    include/SQLiteCpp/Transaction.h
    include/SQLiteCpp/Utils.h
    include/SQLiteCpp/VariadicBind.h
)
ADD_LIBRARY(SQLiteCpp STATIC ${SRC} ${INCL})
target_include_directories(SQLiteCpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_property(TARGET SQLiteCpp PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
