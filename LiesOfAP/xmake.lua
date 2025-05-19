local projectName = "LiesOfAP"

target(projectName)
    add_rules("ue4ss.mod")
    add_includedirs(".")
    add_includedirs("dependencies/apclientpp")
    add_includedirs("dependencies/json/include")
    add_includedirs("dependencies/valijson/include")
    add_includedirs("dependencies/websocketpp")
    add_includedirs("dependencies/wswrap/include")
    add_includedirs("dependencies/asio/include")
    add_includedirs("dependencies/openssl/include")
    add_includedirs("dependencies/zlib/include")

    add_linkdirs("dependencies/zlib")
    add_links("zlibstaticd")

    add_linkdirs("dependencies/openssl/lib")
    add_links("libcrypto")
    add_links("libssl")

    add_files("dllmain.cpp")

    add_defines("_WIN32_WINNT=0x0600")

    add_cxxflags("/Zc:__cplusplus")