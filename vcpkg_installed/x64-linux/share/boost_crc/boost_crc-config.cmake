# Generated by BoostInstall.cmake for boost_crc-1.85.0

include(CMakeFindDependencyMacro)

if(NOT boost_array_FOUND)
  find_dependency(boost_array 1.85.0 EXACT)
endif()
if(NOT boost_config_FOUND)
  find_dependency(boost_config 1.85.0 EXACT)
endif()
if(NOT boost_integer_FOUND)
  find_dependency(boost_integer 1.85.0 EXACT)
endif()
if(NOT boost_type_traits_FOUND)
  find_dependency(boost_type_traits 1.85.0 EXACT)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/boost_crc-targets.cmake")