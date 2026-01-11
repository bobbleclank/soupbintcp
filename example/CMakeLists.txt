add_subdirectory(utility)
add_subdirectory(client)
add_subdirectory(server)

if(BCSOUP_BUILD_TESTS)
  add_subdirectory(test)
endif()

install(
  TARGETS
    bc_soup_client
    bc_soup_server
  EXPORT BcSoupTargets
)
