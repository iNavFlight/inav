# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Defines utilities for Azure SDK to integrate with VCPKG automatically

macro(az_vcpkg_integrate)
  message("Vcpkg integrate step.")
  # AUTO CMAKE_TOOLCHAIN_FILE:
  #   User can call `cmake -DCMAKE_TOOLCHAIN_FILE="path_to_the_toolchain"` as the most specific scenario.
  #   As the last alternative (default case), Azure SDK will automatically clone VCPKG folder and set toolchain from there.
  if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message("CMAKE_TOOLCHAIN_FILE is not defined. Define it for the user.")
    # Set AZURE_SDK_DISABLE_AUTO_VCPKG env var to avoid Azure SDK from cloning and setting VCPKG automatically
    # This option delegate package's dependencies installation to user.
    if(NOT DEFINED ENV{AZURE_SDK_DISABLE_AUTO_VCPKG})
      message("AZURE_SDK_DISABLE_AUTO_VCPKG is not defined. Fetch a local copy of vcpkg.")
      # GET VCPKG FROM SOURCE
      #  User can set env var AZURE_SDK_VCPKG_COMMIT to pick the VCPKG commit to fetch
      set(VCPKG_COMMIT_STRING 94ce0dab56f4d8ba6bd631ba59ed682b02d45c46) # default SDK tested commit
      if(DEFINED ENV{AZURE_SDK_VCPKG_COMMIT})
        message("AZURE_SDK_VCPKG_COMMIT is defined. Using that instead of the default.")
        set(VCPKG_COMMIT_STRING "$ENV{AZURE_SDK_VCPKG_COMMIT}") # default SDK tested commit
      endif()
      message("Vcpkg commit string used: ${VCPKG_COMMIT_STRING}")
      include(FetchContent)
      FetchContent_Declare(
          vcpkg
          GIT_REPOSITORY      https://github.com/microsoft/vcpkg.git
          GIT_TAG             ${VCPKG_COMMIT_STRING}
          )
      FetchContent_GetProperties(vcpkg)
      # make sure to pull vcpkg only once.
      if(NOT vcpkg_POPULATED) 
          FetchContent_Populate(vcpkg)
      endif()
      # use the vcpkg source path 
      set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()
  endif()

  # enable triplet customization
  if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
  endif()
endmacro()
