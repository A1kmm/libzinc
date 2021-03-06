
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
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.1 FATAL_ERROR )

# This is the project name and shows up in IDEs
SET( ZINC_PROJECT_NAME ZincLibraries )
PROJECT( ${ZINC_PROJECT_NAME} )

# The name for the project seen in directory paths
SET( ZINC_BASE_NAME zinc )

INCLUDE( cmake/MacroDefinitions.cmake )
INCLUDE( cmake/FunctionDefinitions.cmake )

# Define the following if you want to see some debug information output.
#SET( ZINC_DEBUG_PRINT TRUE )
#SET( ZINC_DEBUG_PRINT_VERBOSE TRUE )

SET( ZINC_MAJOR_VERSION "3" )
SET( ZINC_MINOR_VERSION "0" )
SET( ZINC_PATCH_VERSION "0" )
SET( ZINC_VERSION ${ZINC_MAJOR_VERSION}.${ZINC_MINOR_VERSION}.${ZINC_PATCH_VERSION} )

IF( NOT DEFINED PREVIOUS_ZINC_DEPENDENCIES_INSTALL_PREFIX )
    SET( PREVIOUS_ZINC_DEPENDENCIES_INSTALL_PREFIX "${ZINC_DEPENDENCIES_INSTALL_PREFIX}" CACHE INTERNAL "Track changes to ZINC_DEPENDENCIES_INSTALL_PREFIX" )
ENDIF( NOT DEFINED PREVIOUS_ZINC_DEPENDENCIES_INSTALL_PREFIX )

STRING( COMPARE NOTEQUAL "x${ZINC_DEPENDENCIES_INSTALL_PREFIX}" "x${PREVIOUS_ZINC_DEPENDENCIES_INSTALL_PREFIX}" ZINC_DEPENDENCIES_INSTALL_PREFIX_CHANGED )
IF( ZINC_DEPENDENCIES_INSTALL_PREFIX_CHANGED )
    MESSAGE( STATUS "Detected change to ZINC_DEPENDENCIES_INSTALL_PREFIX, updating dependent variables automatically." )
    SET( PREVIOUS_ZINC_DEPENDENCIES_INSTALL_PREFIX ${ZINC_DEPENDENCIES_INSTALL_PREFIX} CACHE INTERNAL "Track changes to ZINC_DEPENDENCIES_INSTALL_PREFIX" )
    SET( ZINC_PNG_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/cmake" CACHE PATH "Where is the png CMake config file installed?" FORCE)
    SET( ZINC_fieldml_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/cmake" CACHE PATH "Where is the fieldml CMake config file installed?" FORCE)
    SET( ZINC_ImageMagick_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/cmake" CACHE PATH "Where is Image magick installed?" FORCE)
    SET( ZINC_OCE_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/cmake" CACHE PATH "Where is the OCE CMake config file installed?" FORCE)
    SET( ZINC_OpenCASCADE_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/cmake" CACHE PATH "Where is the OpenCASCADE CMake config file installed?" FORCE)
    SET( ZINC_ITK_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/cmake" CACHE PATH "Where are the ITK libraries installed" FORCE)
    SET( ZINC_MODULE_PATH "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/share/cmiss/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules" CACHE PATH "The location of the Cmiss CMake Modules." FORCE )
ENDIF()

CACHE_VAR_WITH_DEFAULT( ZINC_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}"
    PATH "Where do you want your Zinc libraries installed?" )
SET( CMAKE_INSTALL_PREFIX ${ZINC_INSTALL_PREFIX} CACHE INTERNAL "Internalise CMAKE_INSTALL_PREFIX, manipulate via ZINC_INSTALL_PREFIX" FORCE )

# Offer the user the choice of overriding the installation directories
CACHE_VAR_WITH_DEFAULT(ZINC_INSTALL_LIB_DIR "lib" PATH "Installation directory for libraries")
CACHE_VAR_WITH_DEFAULT(ZINC_INSTALL_BIN_DIR "bin" PATH "Installation directory for executables")
CACHE_VAR_WITH_DEFAULT(ZINC_INSTALL_INC_DIR "include" PATH "Installation directory for header files")
IF(WIN32 AND NOT CYGWIN)
    SET(DEFAULT_INSTALL_CMAKE_DIR cmake)
ELSE()
    SET(DEFAULT_INSTALL_CMAKE_DIR lib/cmake/${ZINC_BASE_NAME})
ENDIF()
CACHE_VAR_WITH_DEFAULT(ZINC_INSTALL_CMAKE_DIR ${DEFAULT_INSTALL_CMAKE_DIR} PATH "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
FOREACH( p LIB BIN INC CMAKE )
    SET( var ZINC_INSTALL_${p}_DIR )
    IF( NOT IS_ABSOLUTE "${${var}}" )
        SET( ${var} "${CMAKE_INSTALL_PREFIX}/${${var}}" )
    ENDIF()
ENDFOREACH()

CACHE_VAR_WITH_DEFAULT( ZINC_DEPENDENCIES_INSTALL_PREFIX "${ZINC_DEPENDENCIES_INSTALL_PREFIX}" PATH "Where are the Zinc third party libraries installed?" )
CACHE_VAR_WITH_DEFAULT( ZINC_MODULE_PATH "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/share/cmiss/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules" PATH "The location of the Cmiss CMake Modules." )

CACHE_VAR_WITH_DEFAULT( ZINC_BUILD_TYPE "Release"
    STRING "What configuration do you want to build: Debug, Release, MinSizeRel, RelWithDebugInfo" )
SET( CMAKE_BUILD_TYPE ${ZINC_BUILD_TYPE} CACHE INTERNAL "Internalise CMAKE_BUILD_TYPE, manipulate via ZINC_BUILD_TYPE" FORCE )

OPTION_WITH_DEFAULT( ZINC_BUILD_SHARED_LIBRARY "Build a shared zinc library." TRUE )
OPTION_WITH_DEFAULT( ZINC_BUILD_STATIC_LIBRARY "Build a static zinc library." FALSE )

IF( NOT ZINC_BUILD_SHARED_LIBRARY AND NOT ZINC_BUILD_STATIC_LIBRARY )
    MESSAGE( FATAL_ERROR "You have to build at least one of the types of libraries (shared or static)." )
ENDIF()

# Set user interface
CACHE_VAR_WITH_DEFAULT( ZINC_USER_INTERFACE "wx" STRING "What type of user interface is required" )
SET_USER_INTERFACE_DEFINES( )

CACHE_VAR_WITH_DEFAULT( ZINC_ImageMagick_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/cmake"
    PATH "Where is Image magick installed?" )
CACHE_VAR_WITH_DEFAULT( ZINC_OCE_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/cmake"
    PATH "Where is the OCE CMake config file installed?" )
CACHE_VAR_WITH_DEFAULT( ZINC_OpenCASCADE_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/cmake"
    PATH "Where is the OpenCASCADE CMake config file installed?" )
IF( EXISTS "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/InsightToolkit" )
    SET( ZINC_ITK_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/InsightToolkit" )
ELSE( EXISTS "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/InsightToolkit" )
    SET( ZINC_ITK_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib" )
ENDIF( EXISTS "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/InsightToolkit" )
CACHE_VAR_WITH_DEFAULT( ZINC_ITK_DIR "${ITK_DIR}"
    PATH "Where are the ITK libraries installed" )
CACHE_VAR_WITH_DEFAULT( ZINC_PNG_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/cmake"
    PATH "Where is the png CMake config file installed?" )
CACHE_VAR_WITH_DEFAULT( ZINC_fieldml_DIR "${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib/cmake"
    PATH "Where is the fieldml CMake config file installed?" )
OPTION( ZINC_BUILD_APPLICATION "Build Cmgui application." FALSE )
IF(ZINC_BUILD_APPLICATION)
    SET( ZINC_BUILD_STATIC_LIBRARY TRUE CACHE BOOL "Build a static zinc library." FORCE )
ENDIF()

SET( PNG_DIR ${ZINC_PNG_DIR} CACHE INTERNAL "Internalise PNG_DIR, manipulate via ZINC_PNG_DIR" FORCE )
SET( ImageMagick_DIR ${ZINC_ImageMagick_DIR} CACHE INTERNAL "Internalise ImageMagick_DIR, manipulate via ZINC_ImageMagick_DIR" FORCE )
SET( ITK_DIR ${ZINC_ITK_DIR} CACHE INTERNAL "Internalise ITK_DIR, manipulate via ZINC_ITK_DIR" FORCE )
SET( OpenCASCADE_DIR ${ZINC_OpenCASCADE_DIR} CACHE INTERNAL "Internalise OpenCASCADE_DIR, manipulate via ZINC_OpenCASCADE_DIR" FORCE )
SET( OCE_DIR ${ZINC_OCE_DIR} CACHE INTERNAL "Internalise OCE_DIR, manipulate via ZINC_OCE_DIR" FORCE )
SET( fieldml_DIR ${ZINC_fieldml_DIR} CACHE INTERNAL "Internalise fieldml_DIR, manipulate via ZINC_fieldml_DIR" FORCE )

# Locate dependent libraries
# I think I can get away with just this set
SET( CMAKE_PREFIX_PATH "${ZINC_DEPENDENCIES_INSTALL_PREFIX}" )
SET( CMAKE_MODULE_PATH "${ZINC_MODULE_PATH}" )

# Must have dependencies
#FIND_PACKAGE( LibXml2 REQUIRED ) <-- pulled in by fieldml
FIND_PACKAGE( fieldml REQUIRED )
FIND_PACKAGE( FTGL REQUIRED )
FIND_PACKAGE( Opt++ REQUIRED COMPONENTS opt newton base constraints gss pds utils newmat )

# Should have dependencies
FIND_PACKAGE( ZLIB QUIET )
FIND_PACKAGE( BZip2 QUIET )
FIND_PACKAGE( JPEG QUIET )
FIND_PACKAGE( NETGEN QUIET )
FIND_PACKAGE( GLEW QUIET )

# Could have dependencies
FIND_PACKAGE( OCE QUIET NO_MODULE )
FIND_PACKAGE( OpenCASCADE QUIET NO_MODULE )
FIND_PACKAGE( ImageMagick QUIET COMPONENTS MagickCore )
FIND_PACKAGE( ITK QUIET )
#FIND_PACKAGE( GDCM QUIET )
SET( PNG_NAMES png12_static libpng12_static )
FIND_PACKAGE( PNG QUIET NO_MODULE )
FIND_PACKAGE( TIFF QUIET )
SET( CMAKE_PREFIX_PATH )
SET( CMAKE_LIBRARY_PATH )
SET( CMAKE_INCLUDE_PATH )

# Define variables to false that need to be at least defined for passsing to option_with_default macro.
SET_FALSE_IF_NOT_DEFINED( ITK_FOUND )
SET_FALSE_IF_NOT_DEFINED( ImageMagick_FOUND )
SET_FALSE_IF_NOT_DEFINED( NETGEN_FOUND )
SET_FALSE_IF_NOT_DEFINED( GLEW_FOUND )
SET_FALSE_IF_NOT_DEFINED( ZLIB_FOUND )
SET_FALSE_IF_NOT_DEFINED( BZIP2_FOUND )
SET_FALSE_IF_NOT_DEFINED( JPEG_FOUND )
SET_FALSE_IF_NOT_DEFINED( OCE_FOUND )
SET_FALSE_IF_NOT_DEFINED( OPENCASCADE_FOUND )

# Use options
IF( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
    OPTION_WITH_DEFAULT( ZINC_BUILD_MEMORYCHECK "Do you want to build a memory check version" OFF )
ENDIF( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
# Dependent library options
# Must have dependent library options, so not really options just here for visiblity
# OPTION_WITH_DEFAULT( ZINC_USE_FIELDML "DO you want to use fieldml?" ON )
# OPTION_WITH_DEFAULT( ZINC_USE_LIBXML2 "Do you want to use xml2?" ${LIBXML2_FOUND} )
# Should have dependent library options
OPTION_WITH_DEFAULT( ZINC_USE_NETGEN "Do you want to use netgen?" ${NETGEN_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_GLEW "Do you want to use glew?" ${GLEW_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_ZLIB "Do you want to use zlib?" ${ZLIB_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_BZip2 "Do you want to use bzip2?" ${BZIP2_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_JPEG "Do you want to use jpeg?" ${JPEG_FOUND} )
# Could have dependent library options
OPTION_WITH_DEFAULT( ZINC_USE_IMAGEMAGICK "Do you want to use Image Magick?"
    ${ImageMagick_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_ITK "Do you want to use ITK?" ${ITK_FOUND} )
# GDCM is not used directly by Zinc so there is no need to define it here.  It should be
# dragged in by the dependencies of those libraries that direclty depend upon it.
#OPTION_WITH_DEFAULT( ZINC_USE_GDCM "Do you want to use libGDCM?" OFF )
OPTION_WITH_DEFAULT( ZINC_USE_OCE "Do you want to use OCE?" ${OCE_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_OPENCASCADE "Do you want to use OpenCASCADE?" FALSE )
IF( ITK_FOUND OR ImageMagick_FOUND )
    SET( PNG_AND_TIFF_REQUIRED TRUE )
ELSE( ITK_FOUND OR ImageMagick_FOUND )
    SET( PNG_AND_TIFF_REQUIRED FALSE )
ENDIF( ITK_FOUND OR ImageMagick_FOUND )
OPTION_WITH_DEFAULT( ZINC_USE_PNG "Do you want to use png?" ${PNG_AND_TIFF_REQUIRED} )
OPTION_WITH_DEFAULT( ZINC_USE_TIFF "Do you want to use tiff?" ${PNG_AND_TIFF_REQUIRED} )
OPTION_WITH_DEFAULT( ZINC_PRINT_CONFIG_SUMMARY "Do you want a configuration summary printed?" TRUE )

# Set general variables that manipulate the build
# Set graphics
IF( NOT DEFINED GRAPHICS_API )
    SET( GRAPHICS_API OPENGL_GRAPHICS )
ENDIF( NOT DEFINED GRAPHICS_API )

# Set build type dependent #Defines
IF( ${CMAKE_GENERATOR} MATCHES "Visual Studio" )
    PRN_MSG( "=== Visual Studio Project Creation ===" )
    SET( CREATING_FOR_MULTI_BUILD_TYPE TRUE )
ENDIF( ${CMAKE_GENERATOR} MATCHES "Visual Studio" )
IF( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
    SET( USE_PARAMETER_ON TRUE )
    IF( NOT ${COMPILER_NAME} MATCHES "msvc" )
        SET( REPORT_GL_ERRORS )
    ENDIF( NOT ${COMPILER_NAME} MATCHES "msvc" )
ELSE( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
    IF( CREATING_FOR_MULTI_BUILD_TYPE )
        SET( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DOPTIMISED" )
        SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DOPTIMISED" )
    ELSE( CREATING_FOR_MULTI_BUILD_TYPE )
        SET( OPTIMISED TRUE )
    ENDIF( CREATING_FOR_MULTI_BUILD_TYPE )
ENDIF( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )

IF( ZINC_BUILD_MEMORYCHECK )
    SET( MEMORY_CHECKING TRUE )
ENDIF( ZINC_BUILD_MEMORYCHECK )

# Set the configurable typedefs
SET( ZnReal double )
SET( FE_value ${ZnReal} )
SET( FE_VALUE_INPUT_STRING \"%lf\" )
SET( FE_VALUE_STRING \"22.15le\" )
SET( COLOUR_PRECISION ${ZnReal} )
SET( MATERIAL_PRECISION ${ZnReal} )


IF( APPLE )
    FIND_LIBRARY( ICONV_LIBRARY NAMES iconv )
    MARK_AS_ADVANCED( ICONV_LIBRARY )
    IF( ICONV_LIBRARY )
        SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${ICONV_LIBRARY} )
    ENDIF( ICONV_LIBRARY )
ENDIF( APPLE )

# Set Target name and global defines
SET_PLATFORM_DEFINES( )
SET_OPERATING_SYSTEM_DEFINES( ) # <-- mostly set automatically by cmake just adding WIN32_SYSTEM to be used instead of WIN32
SET_GRAPHICS_DEFINES( )

# Start loading in the dependent libraries
# fieldml, opt++
SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${FIELDML_LIBRARIES} ${OPTPP_LIBRARIES} ${FTGL_LIBRARIES} )
SET( INCLUDE_DIRS ${FIELDML_INCLUDE_DIRS} ${OPTPP_INCLUDE_DIRS} ${FTGL_INCLUDE_DIR} )
SET( DEPENDENT_DEFINITIONS ${OPTPP_DEFINITIONS} ${FIELDML_DEFINITIONS} ${FTGL_DEFINITIONS} )
SET( USE_FIELDML TRUE )
SET( USE_OPTPP TRUE )
SET( USE_FTGL TRUE )
SET( DEPENDENT_CONFIGS
    "${DEPENDENT_CONFIGS}\nINCLUDE(${fieldml_DIR}/fieldml-config.cmake)" )
PRN_MSG( "FieldML include dirs ${FIELDML_INCLUDE_DIRS}" )
PRN_MSG( "FieldML libraries ${FIELDML_LIBRARIES}" )

# xml2
SET( USE_LIBXML2 TRUE )
SET( DEPENDENT_DEFINITIONS ${DEPENDENT_DEFINITIONS} LIBXML_STATIC )
# Libxml2 libraries are pulled in by fieldml, check the fieldml interface
# libraries for the libxml2 location.
GET_TARGET_PROPERTY( LIBXML2_LIBRARIES fieldml IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE )

# zlib
IF( ZINC_USE_ZLIB )
    FIND_PACKAGE( ZLIB REQUIRED )
    SET( USE_ZLIB TRUE )
    SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${ZLIB_LIBRARIES} )
    PRN_MSG( "Z LIBS: ${ZLIB_LIBRARIES}" )
    SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} )
ENDIF( ZINC_USE_ZLIB )

# bzip2
IF( ZINC_USE_BZip2 )
    FIND_PACKAGE( BZip2 REQUIRED )
    SET( USE_BZIP2 TRUE )
    SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${BZIP2_LIBRARIES} )
    PRN_MSG( "BZ2 LIBS: ${BZIP2_LIBRARIES}" )
    SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${BZIP2_INCLUDE_DIRS} )
ENDIF( ZINC_USE_BZip2 )

IF( ZINC_USE_NETGEN )
    SET( CMAKE_PREFIX_PATH ${ZINC_DEPENDENCIES_INSTALL_PREFIX} )
    FIND_PACKAGE( NETGEN REQUIRED )
    SET( USE_NETGEN TRUE )
    SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${NETGEN_LIBRARIES} )
    PRN_MSG( "netgen LIBS: ${NETGEN_LIBRARIES}" )
    SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${NETGEN_INCLUDE_DIRS} )
    SET( CMAKE_PREFIX_PATH )
ENDIF( ZINC_USE_NETGEN )

IF( ZINC_USE_GLEW )
    SET( CMAKE_PREFIX_PATH ${ZINC_DEPENDENCIES_INSTALL_PREFIX} )
    FIND_PACKAGE( GLEW REQUIRED )
    SET( USE_GLEW TRUE )
    SET( USE_MSAA TRUE )
    IF( WIN32 )
        SET( GLEW_STATIC TRUE )
    ENDIF( WIN32 )
    SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${GLEW_LIBRARIES} )
    PRN_MSG( "GLEW LIBS: ${GLEW_LIBRARIES}" )
    SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} )
    SET( CMAKE_PREFIX_PATH )
ENDIF( ZINC_USE_GLEW )

IF( ZINC_USE_JPEG )
    SET( CMAKE_PREFIX_PATH ${ZINC_DEPENDENCIES_INSTALL_PREFIX} )
    FIND_PACKAGE( JPEG REQUIRED )
    SET( USE_JPEG TRUE )
    SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${JPEG_LIBRARIES} )
    PRN_MSG( "jpeg LIBS: ${JPEG_LIBRARIES}" )
    SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS} )
    SET( CMAKE_PREFIX_PATH )
ENDIF( ZINC_USE_JPEG )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
    FIND_PACKAGE( OpenGL REQUIRED )
    PRN_MSG( "OpenGL libs: " ${OPENGL_LIBRARIES} )
    SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${OPENGL_LIBRARIES} )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

IF( ZINC_USE_ITK )
    FIND_PACKAGE( ITK REQUIRED )
    MARK_AS_ADVANCED( ITK_DIR )
    PRN_MSG( "ITK libs: " ${ITK_LIBRARIES} )
    SET( USE_ITK TRUE )
    SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${ITK_LIBRARIES} )
    SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${ITK_INCLUDE_DIRS} )
    SET( DEPENDENT_CONFIGS "${DEPENDENT_CONFIGS}\nINCLUDE(${ITK_DIR}/ITKConfig.cmake)" )
ENDIF( ZINC_USE_ITK )

# OCE - is a config file therefore I cannot find twice
IF( ZINC_USE_OCE )
    IF( OCE_FOUND )
        MARK_AS_ADVANCED( OCE_DIR )
        SET( USE_OPENCASCADE TRUE )
        SET( DEPENDENT_DEFINITIONS ${DEPENDENT_DEFINITIONS} USE_CAD_FIELDS HAVE_CONFIG_H )
        SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${OCE_LIBRARIES} )
        SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${OCE_INCLUDE_DIRS} )
        PRN_MSG( "=================" )
        PRN_MSG( ${OCE_LIBRARIES} )
        PRN_MSG( "=================" )
    ELSE( OCE_FOUND )
        MESSAGE( FATAL_ERROR "OCE was requested but not found." )
    ENDIF( OCE_FOUND )
ENDIF( ZINC_USE_OCE )

IF( ZINC_USE_TIFF )
    FIND_PACKAGE( TIFF REQUIRED )
    PRN_MSG( "TIFF has been found " ${TIFF_LIBRARIES} )
    SET( USE_TIFF TRUE )
    SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${TIFF_LIBRARIES} )
    PRN_MSG( "TIFF LIBS: ${TIFF_LIBRARIES}" )
    SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${TIFF_INCLUDE_DIRS} )
ENDIF( ZINC_USE_TIFF )

IF( ZINC_USE_PNG )
    FIND_PACKAGE( PNG REQUIRED NO_MODULE )
    SET( USE_PNG TRUE )
    SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${PNG_LIBRARIES} )
    SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} )
    PRN_MSG( "png LIBS: ${PNG_LIBRARIES}" )
    SET( DEPENDENT_CONFIGS
        "${DEPENDENT_CONFIGS}\nINCLUDE(${PNG_DIR}/png-config.cmake)" )
ENDIF( ZINC_USE_PNG )

# OpenCASCADE - is a config file therefore I cannot find twice
IF( ZINC_USE_OPENCASCADE )
    IF( OPENCASCADE_FOUND )
        PRN_MSG( "OCC FOUND ${OPENCASCADE_FOUND}" )
        # Subset of OpenCASCADE libraries Zinc is dependent on
        # TKernel, TKMath, TKAdvTools, TKGeomBase, TKGeomAlgo, TKBRep, TKPrim,
        # TKBO, TKBool, TKG2d, TKG3d, PTKernel, TKOffset, TKFillet, TKMESH, TKIGES,
        # TKXCAF, TKCAF, TKV3d, TKLCAF, TKTopAlgo, TKCDF, TKV2d, TKHLR, TKService,
        # TKShHealing, TKXSBase, TKSTEP, TKSTEPBase, TKSTEPAttr, TKSTEP209,
        # TKXDEIGES, TKXDESTEP
        MARK_AS_ADVANCED( OpenCASCADE_DIR )
        SET( USE_OPENCASCADE TRUE )
        SET( OPENCASCADE_DEFINITIONS USE_CAD_FIELDS )
        IF( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" )
            # 64-bit architecture
            SET( OPENCASCADE_DEFINITIONS ${OPENCASCADE_DEFINITIONS} _OCC64 )
            SET( OPENCASCADE_CXX_FLAGS -m64 )
        ENDIF( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" )
        IF( WIN32 )
            SET( OPENCASCADE_DEFINITIONS ${OPENCASCADE_DEFINITIONS} WNT HAVE_NO_DLL )
        ELSE( WIN32 )
            SET( OPENCASCADE_DEFINITIONS ${OPENCASCADE_DEFINITIONS} HAVE_OCC_CONFIG_H )
        ENDIF( WIN32 )
        #SET( OPENCASCADE_INCLUDE_DIRS ${OPENCASCADE_PATH}/include/OpenCASCADE )
        PRN_MSG( "OCC INC: ${OPENCASCADE_INCLUDE_DIRS}" )
        #SET( OPENCASCADE_LIBRARY_DIRS ${OPENCASCADE_PATH}/lib/OpenCASCADE )
        PRN_MSG( "OCC LIB: ${OPENCASCADE_LIBRARIES}" )
        SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPENCASCADE_CXX_FLAGS}" )

        SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${OPENCASCADE_LIBRARIES} )
        SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${OPENCASCADE_INCLUDE_DIRS} )
        SET( DEPENDENT_CONFIGS "${DEPENDENT_CONFIGS}\nINCLUDE(${OpenCASCADE_DIR}/opencascade-config.cmake)" )
        SET( DEPENDENT_DEFINITIONS ${DEPENDENT_DEFINITIONS} ${OPENCASCADE_DEFINITIONS} )
    ELSE( OPENCASCADE_FOUND )
        MESSAGE( FATAL_ERROR "OpenCASCADE was requested but not found." )
    ENDIF( OPENCASCADE_FOUND )
ENDIF( ZINC_USE_OPENCASCADE )

# ImageMagick - is a config file therefore I cannot find twice
IF( ZINC_USE_IMAGEMAGICK )
    IF( ImageMagick_FOUND )
        MARK_AS_ADVANCED( ImageMagick_DIR )
        PRN_MSG( "Image magick found " ${ImageMagick_INCLUDE_DIRS} )
        SET( USE_IMAGEMAGICK TRUE )
        # For config summary print
        SET( IMAGEMAGICK_LIBRARIES ${ImageMagick_LIBRARIES} )
        SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} MagickCore )
        SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${ImageMagick_INCLUDE_DIRS} )

        SET( CMAKE_INCLUDE_PATH ${ZINC_DEPENDENCIES_INSTALL_PREFIX}/include )
        SET( CMAKE_LIBRARY_PATH ${ZINC_DEPENDENCIES_INSTALL_PREFIX}/lib )
        IF( NOT USE_BZIP2 )
            FIND_PACKAGE( BZip2 REQUIRED )
            SET( USE_BZIP2 TRUE )
            PRN_MSG( "im BZip2 has been found " ${BZIP2_LIBRARIES} )
            SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${BZIP2_LIBRARIES} )
            SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${BZIP2_INCLUDE_DIR} )
        ENDIF( NOT USE_BZIP2 )
        IF( NOT USE_JPEG )
            FIND_PACKAGE( JPEG REQUIRED )
            SET( USE_JPEG TRUE )
            SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${JPEG_LIBRARIES} )
            PRN_MSG( "im jpeg LIBS: ${JPEG_LIBRARIES}" )
            SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS} )
        ENDIF( NOT USE_JPEG )
        SET( CMAKE_LIBRARY_PATH )
        SET( CMAKE_INCLUDE_PATH )
        SET( DEPENDENT_CONFIGS
            "${DEPENDENT_CONFIGS}\nINCLUDE(${ImageMagick_DIR}/ImageMagick-config.cmake)" )
    ELSE( ImageMagick_FOUND )
        MESSAGE( FATAL_ERROR "ImageMagick was requested but not found." )
    ENDIF( ImageMagick_FOUND )
ENDIF( ZINC_USE_IMAGEMAGICK )

IF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    #SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Werror" )
    #SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Werror" )
    SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -fvisibility=hidden -Wl,--as-needed" )
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -fvisibility=hidden -Wl,--as-needed" )
    SET( CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} --as-needed" )
    IF( ${CMAKE_BUILD_TYPE} MATCHES [Rr]elease )
        SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter" )
        SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter" )
    ENDIF( ${CMAKE_BUILD_TYPE} MATCHES [Rr]elease )
ELSEIF( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
    IF( DEFINED MSVC )
        SET( PLATFORM_COMPILER_DEFINITIONS ${PLATFORM_COMPILER_DEFINITIONS} _CRT_SECURE_NO_WARNINGS _CRTDBG_MAP_ALLOC )
    ENDIF( DEFINED MSVC )
ENDIF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

TEST_FOR_VFSCANF( HAVE_VFSCANF )

FOREACH( DEF ${DEPENDENT_DEFINITIONS} ${PLATFORM_COMPILER_DEFINITIONS} )
    ADD_DEFINITIONS( -D${DEF} )
ENDFOREACH()
SET( ZINC_STATIC_DEFINITIONS )
FOREACH( DEF ${DEPENDENT_DEFINITIONS} )
    LIST( APPEND ZINC_STATIC_DEFINITIONS -D${DEF} )
ENDFOREACH()
SET( ZINC_SHARED_DEFINITIONS )

# Used in the generate version string cmake script at compile time.
SET( ZINC_SVN_REPOSITORY_DIR "${CMAKE_SOURCE_DIR}" )
Subversion_GET_REVISION( "${ZINC_SVN_REPOSITORY_DIR}" ZINC_REVISION )
PRN_MSG("svn repo revision is '${ZINC_REVISION}'")

INCLUDE_DIRECTORIES( ${PROJECT_BINARY_DIR}/core/source/api )

LIST( REMOVE_DUPLICATES INCLUDE_DIRS )
SET( ZINC_LIBRARY_DEPENDS_GROUP "ZincLibraryDepends" )
ADD_SUBDIRECTORY(core) # Defines ZINC_LIBRARY_TARGETS
IF( ZINC_BUILD_APPLICATION )
    ADD_SUBDIRECTORY(application)
ENDIF( ZINC_BUILD_APPLICATION )

STRING( REGEX REPLACE ";" " " INCLUDE_DIRS "${INCLUDE_DIRS}" )
STRING( REGEX REPLACE ";" " " DEPENDENT_DEFINITIONS "${DEPENDENT_DEFINITIONS}" )

# Create the ZincConfig.cmake and ZincConfigVersion.cmake files
FILE( RELATIVE_PATH CONF_REL_INCLUDE_DIR "${ZINC_INSTALL_CMAKE_DIR}"
    "${ZINC_INSTALL_INC_DIR}" )
SET( ZINC_CONFIG_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/ZincConfig.cmake" )
CONFIGURE_FILE( cmake/ZincConfig.in.cmake
    "${ZINC_CONFIG_FILE_NAME}" @ONLY )
SET( ZINC_CONFIG_VERSION_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/ZincConfigVersion.cmake" )
CONFIGURE_FILE( cmake/ZincConfigVersion.in.cmake
    "${ZINC_CONFIG_VERSION_FILE_NAME}" @ONLY )
# Create a ZincBuildTreeSettings.cmake file for the use from the build tree
CONFIGURE_FILE( cmake/ZincBuildTreeSettings.in.cmake
    "${PROJECT_BINARY_DIR}/ZincBuildTreeSettings.cmake" @ONLY )
# Create a ZincLibrarySetup.cmake file for defining the available libraries
SET( ZINC_LIBRARY_SETUP_FILE_NAME "${PROJECT_BINARY_DIR}/ZincLibrarySetup.cmake" )
IF( ZINC_SHARED_TARGET AND ZINC_STATIC_TARGET )
    CONFIGURE_FILE( cmake/ZincLibrarySetup.both.in.cmake
        "${ZINC_LIBRARY_SETUP_FILE_NAME}" @ONLY )
ELSE()
    IF( ZINC_SHARED_TARGET )
        CONFIGURE_FILE( cmake/ZincLibrarySetup.shared.in.cmake
            "${ZINC_LIBRARY_SETUP_FILE_NAME}" @ONLY )
    ENDIF()
    IF( ZINC_STATIC_TARGET )
        CONFIGURE_FILE( cmake/ZincLibrarySetup.static.in.cmake
            "${ZINC_LIBRARY_SETUP_FILE_NAME}" @ONLY )
    ENDIF()
ENDIF()

#SET( ZINC_LIBRARY_DEPENDS_FILE_NAME "${PROJECT_BINARY_DIR}/ZincLibraryDepends.cmake" )
# Create the build tree imported targets
EXPORT( TARGETS ${ZINC_SHARED_TARGET} ${ZINC_STATIC_TARGET} FILE "${PROJECT_BINARY_DIR}/ZincLibraryDepends.cmake" )

# Register the Zinc project with the CMake database
EXPORT( PACKAGE ${ZINC_BASE_NAME} )

# Install ZincLibraryDepends.cmake to the install destination
INSTALL( EXPORT ${ZINC_LIBRARY_DEPENDS_GROUP} DESTINATION "${ZINC_INSTALL_CMAKE_DIR}" )

# Install Zinc config files to the install location
INSTALL( FILES "${ZINC_CONFIG_FILE_NAME}" "${ZINC_CONFIG_VERSION_FILE_NAME}" "${ZINC_LIBRARY_SETUP_FILE_NAME}"
    DESTINATION "${ZINC_INSTALL_CMAKE_DIR}" )

# Install headers for Zinc
SET( ZINC_LIB_API_INSTALL_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/context.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/core.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/differentialoperator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/element.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/field.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldalias.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldarithmeticoperators.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldcomposite.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldconditional.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldconstant.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldcoordinatetransformation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldfiniteelement.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldgroup.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldimage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldimageprocessing.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldlogicaloperators.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldmatrixoperators.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldmodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldnodesetoperators.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldsceneviewerprojection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldsubobjectgroup.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtrigonometry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldvectoroperators.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/graphic.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/graphicsfont.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/graphicsfilter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/graphicsmaterial.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/graphicsmodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/node.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/optimisation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/region.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/rendition.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/scene.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/sceneviewer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/sceneviewerinput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/selection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/spectrum.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/status.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/stream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/tessellation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/timenotifier.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/timekeeper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/timesequence.h )
IF( USE_OPENCASCADE )
    SET( ZINC_LIB_API_INSTALL_HDRS ${ZINC_LIB_API_INSTALL_HDRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldcad.h )
ENDIF( USE_OPENCASCADE )

SET( ZINC_LIB_APIPP_INSTALL_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/differentialoperator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/element.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/field.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldcache.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldmodule.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypesarithmeticoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypescomposite.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypesconditional.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypesconstant.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypescoordinatetransformation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypesfiniteelement.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypesgroup.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypesimage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypeslogicaloperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypesmatrixoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypesnodesetoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypessubobjectgroup.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypestime.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypestrigonometry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/fieldtypesvectoroperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/graphic.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/graphicsfilter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/graphicsmaterial.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/graphicsmodule.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/node.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/optimisation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/region.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/rendition.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/scene.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/sceneviewer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/selection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/spectrum.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/stream.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/tessellation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/timenotifier.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/timekeeper.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/timesequence.hpp )

SET( ZINC_LIB_API_TYPES_INSTALL_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/contextid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/differentialoperatorid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/elementid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/fieldaliasid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/fieldfiniteelementid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/fieldgroupid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/fieldid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/fieldimageid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/fieldimageprocessingid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/fieldmoduleid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/fieldsubobjectgroupid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/graphicid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/graphicscoordinatesystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/graphicsfontid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/graphicsfilterid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/graphicsmaterialid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/graphicsmoduleid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/graphicsrendertype.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/interactivetoolid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/nodeid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/optimisationid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/regionid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/renditionid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/sceneid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/sceneviewerid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/selectionid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/spectrumid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/streamid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/tessellationid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/timeid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/timekeeperid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/timesequenceid.h )
IF( USE_OPENCASCADE )
    SET( ZINC_LIB_API_TYPES_INSTALL_HDRS ${ZINC_LIB_API_TYPES_INSTALL_HDRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/core/source/api/zinc/types/fieldcadid.h )
ENDIF( USE_OPENCASCADE )

# Api directory installs
INSTALL( FILES ${ZINC_LIB_API_TYPES_INSTALL_HDRS}
    DESTINATION "${ZINC_INSTALL_INC_DIR}/zinc/types" )

INSTALL( FILES "${PROJECT_BINARY_DIR}/core/source/api/zinc/zincconfigure.h"
    "${PROJECT_BINARY_DIR}/core/source/api/zinc/zincsharedobject.h"
    ${ZINC_LIB_API_INSTALL_HDRS}
    ${ZINC_LIB_APIPP_INSTALL_HDRS}
    DESTINATION "${ZINC_INSTALL_INC_DIR}/zinc" )

INCLUDE(cmake/ZincPackaging.cmake)

# Output configuration information if requested
IF( ZINC_PRINT_CONFIG_SUMMARY )
    MACRO( LIBRARY_SUMMARY_STRING LIBRARY_NAME OUTPUT_STRING )
        IF( USE_${LIBRARY_NAME} )
            SET( ${OUTPUT_STRING} "system" )
            FOREACH( LIB ${${LIBRARY_NAME}_LIBRARIES} )
                GET_TARGET_PROPERTY( TARGET_ILR ${LIB} IMPORTED_LOCATION_RELEASE )
                SET( SEARCH_LIB_STRING ${LIB} )
                # If we can get a target imported location use
                # that for the test string.
                IF( TARGET_ILR )
                    SET( SEARCH_LIB_STRING ${TARGET_ILR} )
                ENDIF( TARGET_ILR )
                STRING( REGEX MATCH "${ZINC_DEPENDENCIES_INSTALL_PREFIX}" REGEX_RESULT ${SEARCH_LIB_STRING} )
                IF( "x${REGEX_RESULT}" STREQUAL "x${ZINC_DEPENDENCIES_INSTALL_PREFIX}" )
                    SET( ${OUTPUT_STRING} "third_party" )
                ENDIF( "x${REGEX_RESULT}" STREQUAL "x${ZINC_DEPENDENCIES_INSTALL_PREFIX}" )
            ENDFOREACH( LIB ${${LIBRARY_NAME}_LIBRARIES} )
        ELSE( USE_${LIBRARY_NAME} )
            SET( ${OUTPUT_STRING} "n/a" )
        ENDIF( USE_${LIBRARY_NAME} )
    ENDMACRO( LIBRARY_SUMMARY_STRING LIBRARY_NAME OUTPUT_STRING )

    LIBRARY_SUMMARY_STRING( BZIP2 BZIP2_LIB_STRING )
    LIBRARY_SUMMARY_STRING( FIELDML FIELDML_LIB_STRING )
    LIBRARY_SUMMARY_STRING( FTGL FTGL_LIB_STRING )
    LIBRARY_SUMMARY_STRING( GLEW GLEW_LIB_STRING )
    LIBRARY_SUMMARY_STRING( IMAGEMAGICK IMAGEMAGICK_LIB_STRING )
    LIBRARY_SUMMARY_STRING( ITK INSIGHTTOOLKIT_LIB_STRING )
    LIBRARY_SUMMARY_STRING( JPEG JPEG_LIB_STRING )
    LIBRARY_SUMMARY_STRING( NETGEN NETGEN_LIB_STRING )
    LIBRARY_SUMMARY_STRING( OPENCASCADE OPENCASCADE_LIB_STRING )
    LIBRARY_SUMMARY_STRING( OPTPP OPT++_LIB_STRING )
    LIBRARY_SUMMARY_STRING( PNG PNG_LIB_STRING )
    LIBRARY_SUMMARY_STRING( TIFF TIFF_LIB_STRING )
    LIBRARY_SUMMARY_STRING( ZLIB ZLIB_LIB_STRING )
    IF( ZINC_BUILD_SHARED_LIBRARY AND ZINC_BUILD_STATIC_LIBRARY )
        SET( ZINC_BUILD_STRING "shared and static libraries" )
    ELSEIF( ZINC_BUILD_SHARED_LIBRARY )
        SET( ZINC_BUILD_STRING "shared library" )
    ELSEIF( ZINC_BUILD_STATIC_LIBRARY )
        SET( ZINC_BUILD_STRING "static library" )
    ELSE()
        SET( ZINC_BUILD_STRING "<something else>" )
    ENDIF()
    MESSAGE( STATUS "")
    MESSAGE( STATUS "=====================================================")
    MESSAGE( STATUS "Zinc has been configured with the following settings:")
    MESSAGE( STATUS "")
    MESSAGE( STATUS "     Source revision: '${ZINC_REVISION}'")
    MESSAGE( STATUS "         Target type: '${ZINC_BUILD_STRING}'")
    MESSAGE( STATUS "          Build type: '${CMAKE_BUILD_TYPE}'")
    MESSAGE( STATUS "    Architecture dir: '${ARCHITECTURE_DIR}'")
    MESSAGE( STATUS "")
    MESSAGE( STATUS " Dependent libraries:")
    MESSAGE( STATUS "               BZIP2: '${BZIP2_LIB_STRING}'")
    MESSAGE( STATUS "             FIELDML: '${FIELDML_LIB_STRING}'")
    MESSAGE( STATUS "                FTGL: '${FTGL_LIB_STRING}'")
    MESSAGE( STATUS "                GLEW: '${GLEW_LIB_STRING}'")
    MESSAGE( STATUS "         IMAGEMAGICK: '${IMAGEMAGICK_LIB_STRING}'")
    MESSAGE( STATUS "      INSIGHTTOOLKIT: '${INSIGHTTOOLKIT_LIB_STRING}'")
    MESSAGE( STATUS "                JPEG: '${JPEG_LIB_STRING}'")
    MESSAGE( STATUS "              NETGEN: '${NETGEN_LIB_STRING}'")
    MESSAGE( STATUS "         OPENCASCADE: '${OPENCASCADE_LIB_STRING}'")
    MESSAGE( STATUS "               OPT++: '${OPT++_LIB_STRING}'")
    MESSAGE( STATUS "                 PNG: '${PNG_LIB_STRING}'")
    MESSAGE( STATUS "                TIFF: '${TIFF_LIB_STRING}'")
    MESSAGE( STATUS "                ZLIB: '${ZLIB_LIB_STRING}'")
    MESSAGE( STATUS "=====================================================")
    MESSAGE( STATUS "")
ENDIF( ZINC_PRINT_CONFIG_SUMMARY )

