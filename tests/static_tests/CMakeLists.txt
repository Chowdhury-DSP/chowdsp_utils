message(STATUS "Configuring static tests...")
include(SetupStaticTests)

create_static_test(StaticLanczosResamplerTest)
add_static_test_check(StaticLanczosResamplerTest LanczosKernel4_pass TRUE)
add_static_test_check(StaticLanczosResamplerTest LanczosKernel5_fail FALSE)
