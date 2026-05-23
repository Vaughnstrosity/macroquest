# Generated from MQ2AutoAccept.vcxproj
# This file is designed to work with add_subdirectory()

# ---------------------------------------------------------------------
# Props file includes
# ---------------------------------------------------------------------
include("../../src/Plugin.cmake")

# ---------------------------------------------------------------------
# Header files
# ---------------------------------------------------------------------
set(MQ2AutoAccept_HEADERS
    "resource.h"
)

source_groups("Header Files" ${MQ2AutoAccept_HEADERS})

# ---------------------------------------------------------------------
# Other files
# ---------------------------------------------------------------------
set(MQ2AutoAccept_OTHERFILES
    "MQ2AutoAccept.rc"
    "README.md"
)

source_group("Resource Files" FILES ${MQ2AutoAccept_OTHERFILES})

# ---------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------
set(MQ2AutoAccept_SOURCES
    "MQ2AutoAccept.cpp"
)

source_groups("Source Files" ${MQ2AutoAccept_SOURCES})

# ---------------------------------------------------------------------
# Target definition
# ---------------------------------------------------------------------
add_library(MQ2AutoAccept SHARED
    ${MQ2AutoAccept_HEADERS}
    ${MQ2AutoAccept_OTHERFILES}
    ${MQ2AutoAccept_SOURCES}
)

set_target_properties(MQ2AutoAccept PROPERTIES FOLDER "external")

# ---------------------------------------------------------------------
# Apply props file configurations
# ---------------------------------------------------------------------
target_Plugin_props(MQ2AutoAccept)

# ---------------------------------------------------------------------
# PRE_BUILD events
# ---------------------------------------------------------------------

add_custom_command(TARGET MQ2AutoAccept PRE_BUILD
    COMMAND powershell.exe -ExecutionPolicy Bypass -NoProfile -NonInteractive -File "${CMAKE_SOURCE_DIR}/tools/build_scripts/Plugin_Versioning.ps1" -ProjectName "${PROJECT_NAME}" -ProjectDirectory "${CMAKE_CURRENT_SOURCE_DIR}/"
    COMMENT "Updating version resource for ${PROJECT_NAME}..."
    VERBATIM
)

# ============================================================================
# MANUAL CONVERSION NOTES
# ============================================================================
#
# - Review and adjust as needed
