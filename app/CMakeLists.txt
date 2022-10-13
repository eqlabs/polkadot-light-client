add_executable(polkadot_light_client
    src/main.cpp)

target_link_libraries(polkadot_light_client plc_core boost_program_options)
