
SET(CPACK_PACKAGE_VERSION_MAJOR "${ZINC_MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${ZINC_MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${ZINC_PATCH_VERSION}")

SET( CPACK_GENERATOR "TGZ;DEB" )
SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "Hugh Sorby" ) #required
SET( CPACK_PACKAGE_FILE_NAME "zinc-${ZINC_MAJOR_VERSION}.${ZINC_MINOR_VERSION}.${ZINC_PATCH_VERSION}" )

INCLUDE(CPack)
