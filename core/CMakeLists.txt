
 # ***** BEGIN LICENSE BLOCK *****
 # Version: MPL 1.1/GPL 2.0/LGPL 2.1
 #
 # The contents of this file are subject to the Mozilla Public License Version
 # 1.1 (the "License"); you may not use this file except in compliance with
 # the License. You may obtain a copy of the License at
 # http://www.mozilla.org/MPL/
 #
 # Software distributed under the License is distributed on an "AS IS" basis,
 # WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 # for the specific language governing rights and limitations under the
 # License.
 #
 # The Original Code is cmgui
 #
 # The Initial Developer of the Original Code is
 # Auckland Uniservices Ltd, Auckland, New Zealand.
 # Portions created by the Initial Developer are Copyright (C) 2011
 # the Initial Developer. All Rights Reserved.
 #
 # Contributor(s):
 #
 # Alternatively, the contents of this file may be used under the terms of
 # either the GNU General Public License Version 2 or later (the "GPL"), or
 # the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 # in which case the provisions of the GPL or the LGPL are applicable instead
 # of those above. If you wish to allow use of your version of this file only
 # under the terms of either the GPL or the LGPL, and not to allow others to
 # use your version of this file under the terms of the MPL, indicate your
 # decision by deleting the provisions above and replace them with the notice
 # and other provisions required by the GPL or the LGPL. If you do not delete
 # the provisions above, a recipient may use your version of this file under
 # the terms of any one of the MPL, the GPL or the LGPL.
 #
 # ***** END LICENSE BLOCK ***** */

# Set the minimum version of cmake required to 2.8.1
# The FATAL_ERROR is needed if using cmake < 2.4

# This is the project name and shows up in ides
PROJECT( ZincCore )

IF( MSVC )
    SET( ZINC_C_INLINE __inline )
ELSE()
    SET( ZINC_C_INLINE "static inline" )
ENDIF()

SET( ZINC_CONFIGURE ${PROJECT_BINARY_DIR}/source/api/zinc/zincconfigure.h )
SET( ZINC_SHARED_OBJECT ${PROJECT_BINARY_DIR}/source/api/zinc/zincsharedobject.h )
CONFIGURE_FILE( ${PROJECT_SOURCE_DIR}/source/configure/zincconfigure.h.cmake
    ${ZINC_CONFIGURE} )
CONFIGURE_FILE( ${PROJECT_SOURCE_DIR}/source/configure/zincsharedobject.h.cmake
    ${ZINC_SHARED_OBJECT} )
SET( CONFIGURE_HDRS ${ZINC_CONFIGURE} ${ZINC_SHARED_OBJECT} )

# Load in source files, each source file list may contain several
# CMake variables see comments for a list of _SRCS included from
# a particular file (similarly that file defines a _HDRS variable).
# Defines API_SRCS
INCLUDE( source/cmake/api_source_files.cmake )

# Defines APIPP_SRCS
INCLUDE( source/cmake/apipp_source_files.cmake )

# Defines COMMAND_SRCS, CONTEXT_SRCS
INCLUDE( source/cmake/command_source_files.cmake )

# Defines COMPUTED_FIELD_CORE_SRCS, COMPUTED_FIELD_GRAPHICS_SRCS,
# COMPUTED_FIELD_SRCS (contains previous two)
INCLUDE( source/cmake/computed_field_source_files.cmake )

# Defines FINITE_ELEMENT_CORE_SRCS, FINITE_ELEMENT_GRAPHICS_SRCS,
# FINITE_ELEMENT_ADDITIONAL_SRCS, FINITE_ELEMENT_SRCS
# (group definition which includes the previous three)
INCLUDE( source/cmake/finite_element_source_files.cmake )

# Defines GENERAL_SRCS
INCLUDE( source/cmake/general_source_files.cmake )

# Defines GRAPHICS_SRCS
INCLUDE( source/cmake/graphics_source_files.cmake )

# Defines IMAGE_PROCESSING_SRCS
INCLUDE( source/cmake/image_processing_source_files.cmake )

# Defines COMFILE_SRCS, CURVE_SRCS, ELEMENT_SRCS, EMOTER_SRCS
# FIELD_IO_SRCS, INTERACTION_SRCS, IO_DEVICES_SRCS, MESH_SRCS, NODE_SRCS,
# REGION_SRCS, SELECTION_SRCS, THREE_D_DRAWING_SRCS, TIME_SRCS
INCLUDE( source/cmake/misc_source_files.cmake )

# Defines OPENCASCADE_SRCS
INCLUDE( source/cmake/opencascade_source_files.cmake )

# Defines ZINC_LIB_SRCS, ZINC_LIB_HDRS
INCLUDE( source/cmake/zinc_lib_source_files.cmake )

PRINT_LIST_DUPLICATIONS( ZINC_LIB_SRCS )
PRINT_LIST_DUPLICATIONS( ZINC_LIB_HDRS )

INCLUDE_DIRECTORIES( source source/api ${CMAKE_CURRENT_BINARY_DIR}/source
    ${CMAKE_CURRENT_BINARY_DIR}/source/api ${INCLUDE_DIRS} )

IF( MSVC_IDE )
    # I want to group the source files by the directory layout for Visual Studio
    GROUP_SOURCE_FILES(${ZINC_LIB_SRCS} ${ZINC_LIB_HDRS} ${CONFIGURE_HDRS})
    ADD_DEFINITIONS( "/wd4244 /nologo" )
ENDIF()

IF( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
    SET_SOURCE_FILES_PROPERTIES( ${ZINC_LIB_SRCS}
        PROPERTIES COMPILE_FLAGS "-fPIC" )
ENDIF( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )

SET( ZINC_LIBRARY_NAME zinc )
IF( ZINC_BUILD_SHARED_LIBRARY )
    SET( ZINC_SHARED_TARGET ${ZINC_LIBRARY_NAME} )
    ADD_LIBRARY( ${ZINC_SHARED_TARGET} SHARED ${ZINC_LIB_SRCS} ${ZINC_LIB_HDRS} ${CONFIGURE_HDRS} )
    LIST( APPEND ZINC_SHARED_DEFINITIONS -DZINC_SHARED_OBJECT )
    SET( ZINC_SHARED_DEFINITIONS ${ZINC_SHARED_DEFINITIONS} PARENT_SCOPE )
    SET_TARGET_PROPERTIES( ${ZINC_SHARED_TARGET} PROPERTIES COMPILE_DEFINITIONS "ZINC_SHARED_OBJECT"
        DEFINE_SYMBOL "ZINC_EXPORTS" )
    TARGET_LINK_LIBRARIES( ${ZINC_LIBRARY_NAME} LINK_PRIVATE ${DEPENDENT_LIBS} )
    IF( WIN32 )
        TARGET_LINK_LIBRARIES( ${ZINC_LIBRARY_NAME} LINK_PUBLIC winmm.lib )
    ENDIF()
ENDIF()
IF( ZINC_BUILD_STATIC_LIBRARY )
    SET( ZINC_STATIC_TARGET ${ZINC_LIBRARY_NAME}-static )
    ADD_LIBRARY( ${ZINC_STATIC_TARGET} STATIC ${ZINC_LIB_SRCS} ${ZINC_LIB_HDRS} ${CONFIGURE_HDRS} )
    SET_TARGET_PROPERTIES(${ZINC_STATIC_TARGET} PROPERTIES OUTPUT_NAME "${ZINC_LIBRARY_NAME}" PREFIX "lib")
    TARGET_LINK_LIBRARIES( ${ZINC_STATIC_TARGET} ${DEPENDENT_LIBS} )
    IF( WIN32 )
        TARGET_LINK_LIBRARIES( ${ZINC_STATIC_TARGET} winmm.lib )
    ENDIF()
ENDIF()

SET_TARGET_PROPERTIES( ${ZINC_SHARED_TARGET} ${ZINC_STATIC_TARGET} PROPERTIES DEBUG_POSTFIX "d"
    VERSION "${ZINC_MAJOR_VERSION}.${ZINC_MINOR_VERSION}.${ZINC_PATCH_VERSION}"
    SOVERSION ${ZINC_MAJOR_VERSION} )

INSTALL( TARGETS ${ZINC_SHARED_TARGET} ${ZINC_STATIC_TARGET} EXPORT ${ZINC_LIBRARY_DEPENDS_GROUP}
    ARCHIVE DESTINATION ${ZINC_INSTALL_LIB_DIR}
    LIBRARY DESTINATION ${ZINC_INSTALL_LIB_DIR}
    RUNTIME DESTINATION ${ZINC_INSTALL_BIN_DIR} )

IF( ZINC_SHARED_TARGET )
    SET( ZINC_SHARED_TARGET ${ZINC_SHARED_TARGET} PARENT_SCOPE )
ENDIF()
IF( ZINC_STATIC_TARGET )
    SET( ZINC_STATIC_TARGET ${ZINC_STATIC_TARGET} PARENT_SCOPE )
ENDIF()


