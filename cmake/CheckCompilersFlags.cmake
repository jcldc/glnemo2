# ============================================================================
# Copyright Jean-Charles LAMBERT - 2007-2026
# e-mail:   Jean-Charles.Lambert@lam.fr
# address:  Dynamique des galaxies
#           Centre de donneeS Astrophysique de Marseille (CeSAM)
#           Laboratoire d'Astrophysique de Marseille
#           Pole de l'Etoile, site de Chateau-Gombert
#           38, rue Frederic Joliot-Curie
#           13388 Marseille cedex 13 France
#           CNRS U.M.R 6110
# ============================================================================


# Liste des options de compilation à vérifier
set(COMPILER_FLAGS_TO_CHECK
    -std=gnu17 
    -Wno-implicit-function-declaration 
    -Wno-int-conversion 
    -Wno-implicit-int 
    -Wno-write-strings
    -std=c++17
    -Wno-conversion-null
    -Wno-deprecated-declarations
    -Wno-format-overflow
)

# Check C/C++ flags
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
set(CMAKE_REQUIRED_QUIET TRUE)
foreach(flag IN LISTS COMPILER_FLAGS_TO_CHECK)
    check_cxx_compiler_flag(${flag} HAS_CXX_${flag})
    if(HAS_CXX_${flag})
        message(STATUS "C++: ${flag} added")
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${flag}>)
    endif()

    check_c_compiler_flag(${flag} HAS_C_${flag})
    if(HAS_C_${flag})
        message(STATUS "C: ${flag} added")
        add_compile_options($<$<COMPILE_LANGUAGE:C>:${flag}>)
    endif()
endforeach()
unset(CMAKE_REQUIRED_QUIET)