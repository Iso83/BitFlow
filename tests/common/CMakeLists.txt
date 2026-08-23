# ============================================================================
# Source files
# ============================================================================

file(GLOB_RECURSE MODULE_PUBLIC_HEADERS CONFIGURE_DEPENDS
    include/*.h
)

# ============================================================================
# Target
# ============================================================================

string(REPLACE "/" "_" MODULE_NAME "tests/common")

set(MODULE_TARGET "${PROJECT_NAME}_${MODULE_NAME}")

add_library(${MODULE_TARGET} INTERFACE)

target_sources(${MODULE_TARGET}
    INTERFACE
        ${MODULE_PUBLIC_HEADERS}
)

add_library(${PROJECT_NAME}::${MODULE_NAME} ALIAS ${MODULE_TARGET})

# ============================================================================
# Build configuration
# ============================================================================

target_include_directories(${MODULE_TARGET}
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include/BitFlow/tests"
)
