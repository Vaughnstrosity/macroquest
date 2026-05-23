# Generated from MQ2Cast.vcxproj
# This file is designed to work with add_subdirectory()

# ---------------------------------------------------------------------
# Props file includes
# ---------------------------------------------------------------------
include("../../src/Plugin.cmake")

# ---------------------------------------------------------------------
# Header files
# ---------------------------------------------------------------------
set(MQ2Cast_HEADERS
    "resource.h"
)

source_groups("Header Files" ${MQ2Cast_HEADERS})

# ---------------------------------------------------------------------
# Other files
# ---------------------------------------------------------------------
set(MQ2Cast_OTHERFILES
    "MQ2Cast.rc"
    "README.md"
)

source_group("Resource Files" FILES ${MQ2Cast_OTHERFILES})

# ---------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------
set(MQ2Cast_SOURCES
    "MQ2Cast.cpp"
    "../../src/plugins/pluginapi/pluginapi.cpp"
)

source_groups("Source Files" ${MQ2Cast_SOURCES})

# ---------------------------------------------------------------------
# Target definition
# ---------------------------------------------------------------------
add_library(MQ2Cast SHARED
    ${MQ2Cast_HEADERS}
    ${MQ2Cast_OTHERFILES}
    ${MQ2Cast_SOURCES}
)

set_target_properties(MQ2Cast PROPERTIES FOLDER "external")

# ---------------------------------------------------------------------
# Apply props file configurations
# ---------------------------------------------------------------------
target_Plugin_props(MQ2Cast)

# ---------------------------------------------------------------------
# PRE_BUILD events
# ---------------------------------------------------------------------

add_custom_command(TARGET MQ2Cast PRE_BUILD
    COMMAND powershell.exe -ExecutionPolicy Bypass -NoProfile -NonInteractive -File "${CMAKE_SOURCE_DIR}/tools/build_scripts/Plugin_Versioning.ps1" -ProjectName "${PROJECT_NAME}" -ProjectDirectory "${CMAKE_CURRENT_SOURCE_DIR}/"
    COMMENT "Updating version resource for ${PROJECT_NAME}..."
    VERBATIM
)

# ============================================================================
# MANUAL CONVERSION NOTES
# ============================================================================
#
# - Review and adjust as needed
