# ============================================================================
# Copyright Jean-Charles LAMBERT - 2009-2012
# e-mail:   Jean-Charles.Lambert@oamp.fr
# address:  Dynamique des galaxies
#           Centre de donneeS Astrophysique de Marseille (CeSAM)
#           Laboratoire d'Astrophysique de Marseille
#           Pole de l'Etoile, site de Chateau-Gombert
#           38, rue Frederic Joliot-Curie
#           13388 Marseille cedex 13 France
#           CNRS U.M.R 6110
# ============================================================================
if ( NOT SETUP_FLAGS_INCLUDED )
  set( SETUP_FLAGS_INCLUDED 1 )

macro( typed_cache_set type doc var )
    set ( ${var} ${ARGN} CACHE ${type} ${doc} FORCE )
    set ( ${var} ${ARGN} CACHE ${type} ${doc} FORCE )
endmacro()

typed_cache_set ( STRING "setup flags"  SETUP_FLAGS_INCLUDED 1  )

# Set a default build type if none is given
if ( NOT CMAKE_BUILD_TYPE ) # Release default
    typed_cache_set ( STRING "Build type: Release or Debug" CMAKE_BUILD_TYPE Release   )
 # else ()                   # else Debug 
 #   typed_cache_set ( STRING "Build type: Release or Debug" CMAKE_BUILD_TYPE Debug )
endif()

# set optimizer flag
if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
  typed_cache_set (STRING "Optimizer" OPT "-g -ggdb")
else ()
  typed_cache_set (STRING "Optimizer" OPT "-O3")
endif ()

# detect MacOS
if    (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   typed_cache_set ( BOOL "OSX detection" OSX TRUE  )
else()
   typed_cache_set ( BOOL "OSX detection" OSX FALSE )
endif()

# SHARED lib ?
option(BUILD_SHARED_LIBS "Shared lib " ON)  # SHARED lib default
if ( NOT BUILD_SHARED_LIBS ) # Debug TRUE
    typed_cache_set ( BOOL "Build SHARED" BUILD_SHARED_LIBS FALSE )
  else ()                   # else  FALSE
    typed_cache_set ( BOOL "Build SHARED" BUILD_SHARED_LIBS TRUE)
endif()



# set library type , SHARED or STATIC + LIB extension
if ( BUILD_SHARED_LIBS )
   typed_cache_set ( STRING "LIB TYPE" LIBTYPE SHARED )
   if (OSX)
      typed_cache_set ( STRING "LIB TYPE" LIBEXT  "dylib" )
   else()
      typed_cache_set ( STRING "LIB TYPE" LIBEXT  "so" )
   endif()

else ()
   typed_cache_set ( STRING "LIB TYPE" LIBTYPE STATIC )
   typed_cache_set ( STRING "LIB TYPE" LIBEXT  "a" )
endif()

if (UNSIO_INSTALLPATH)
   typed_cache_set ( STRING "UNSIO loation" UNSIOPATH  ${UNSIO_INSTALLPATH} )
endif()
#

endif( NOT SETUP_FLAGS_INCLUDED )
# ============================================================================
