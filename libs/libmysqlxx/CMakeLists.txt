add_library (mysqlxx ${LINK_MODE}
    src/Connection.cpp
    src/Exception.cpp
    src/Query.cpp
    src/ResultBase.cpp
    src/StoreQueryResult.cpp
    src/UseQueryResult.cpp
    src/Row.cpp
    src/Value.cpp
    src/Pool.cpp
    src/PoolWithFailover.cpp

    include/mysqlxx/Connection.h
    include/mysqlxx/Exception.h
    include/mysqlxx/Manip.h
    include/mysqlxx/mysqlxx.h
    include/mysqlxx/Null.h
    include/mysqlxx/Pool.h
    include/mysqlxx/PoolWithFailover.h
    include/mysqlxx/Query.h
    include/mysqlxx/ResultBase.h
    include/mysqlxx/Row.h
    include/mysqlxx/StoreQueryResult.h
    include/mysqlxx/Transaction.h
    include/mysqlxx/Types.h
    include/mysqlxx/UseQueryResult.h
    include/mysqlxx/Value.h
)

target_include_directories (mysqlxx PUBLIC include)

if (USE_INTERNAL_MYSQL_LIBRARY)

else ()
    set(PLATFORM_LIBRARIES ${CMAKE_DL_LIBS})

    if (USE_MYSQL)
        target_include_directories (mysqlxx SYSTEM PRIVATE ${MYSQL_INCLUDE_DIR})
    endif ()

    if (APPLE)
        find_library (ICONV_LIBRARY iconv)
        set (MYSQLCLIENT_LIBRARIES ${MYSQLCLIENT_LIBRARIES} ${STATIC_MYSQLCLIENT_LIB} ${ICONV_LIBRARY})
    elseif (USE_STATIC_LIBRARIES AND STATIC_MYSQLCLIENT_LIB)
        set (MYSQLCLIENT_LIBRARIES ${STATIC_MYSQLCLIENT_LIB})
    endif ()
endif ()

target_link_libraries (mysqlxx common ${Poco_Util_LIBRARY} ${MYSQLCLIENT_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} ${PLATFORM_LIBRARIES})
if (NOT USE_INTERNAL_MYSQL_LIBRARY)
    target_include_directories (mysqlxx SYSTEM PRIVATE ${OPENSSL_INCLUDE_DIR})
endif ()

if (ENABLE_TESTS)
    add_subdirectory (src/tests)
endif ()
