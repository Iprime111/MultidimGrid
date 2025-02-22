set(SOURCES 
    tests.cpp
)

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

target_sources(multidim-grid-tests PRIVATE ${SOURCES}) 
