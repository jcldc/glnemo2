# -*-cmake-*-
# ============================================================================
# Copyright Jean-Charles LAMBERT - 2007-2026
# e-mail:   Jean-Charles.Lambert@lam.fr
# address:  Dynamique des galaxies
#           Centre de donneeS Astrophysique de Marseille (CeSAM)
#           Laboratoire d'Astrophysique de Marseille
#           Pole de l'Etoile, site de Chateau-Gombert
#           38, rue Frederic Joliot-Curie
#           13388 Marseille cedex 13 France
#           CNRS U.M.R 7326
# ============================================================================
# Detect OS and architecture to select right CPACK Generator
# ============================================================================
#
#
if(UNIX AND NOT APPLE)
  # CMake read system info from /etc/os-release
  cmake_host_system_information(RESULT OS_ID QUERY DISTRIB_ID)
  cmake_host_system_information(RESULT OS_LIKE QUERY DISTRIB_ID_LIKE)

  string(TOLOWER "${OS_ID}" OS_ID)
  string(TOLOWER "${OS_LIKE}" OS_LIKE)

  # Debian / Ubuntu / Mint
  if(OS_ID MATCHES "ubuntu|debian|mint" OR OS_LIKE MATCHES "debian|ubuntu")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON) # generate apt dependencies
  # Fedora / RedHat / SuSE / Mageia
  elseif(OS_ID MATCHES "fedora|rhel|centos|suse|mageia"
          OR OS_LIKE MATCHES "rhel|fedora|suse")
    set(CPACK_GENERATOR "RPM")
  # if distrib not detected, then tarball .tar.gz
  else()
    set(CPACK_GENERATOR "TGZ")
  endif()
endif()

