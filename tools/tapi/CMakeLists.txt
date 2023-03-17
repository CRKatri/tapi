add_custom_target(tapi-clang-resource-headers ALL DEPENDS clang-resource-headers)

add_tapi_executable(tapi
  tapi.cpp
  )

target_link_libraries(tapi
  PRIVATE
  tapiDriver
  tapiObjCMetadata
  )

add_dependencies(tapi tapi-clang-resource-headers)

install(TARGETS tapi
  RUNTIME DESTINATION bin
  COMPONENT tapi
  )

add_llvm_install_targets(install-tapi
                         DEPENDS tapi
                         COMPONENT tapi)
