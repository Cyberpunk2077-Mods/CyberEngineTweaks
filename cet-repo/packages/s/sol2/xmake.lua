package("sol2")
    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/ThePhD/sol2")
    set_description("A C++ library binding to Lua.")

    set_urls("https://github.com/ThePhD/sol2/archive/refs/tags/$(version).tar.gz",
             "https://github.com/ThePhD/sol2.git")

    add_versions("v3.5.0", "86c0f6d2836b184a250fc2907091c076bf53c9603dd291eaebade36cc342e13c")

    add_configs("includes_lua", {description = "Should this package includes the Lua package (set to false if you're shipping a custom Lua)", default = true, type = "boolean"})

    on_load(function (package)
        if package:config("includes_lua") then
            package:add("deps", "lua 5.4")
        end
    end)

    on_install(function (package)
        -- sol2's CMakeLists treats the source tree as a top-level project and will
        -- find/build Lua even when we only need headers (CET ships LuaJIT separately).
        -- Install as a pure header package to avoid that configure path.
        os.cp("include/sol", package:installdir("include"))
        os.trycp("include/sol.hpp", package:installdir("include"))
    end)

    on_test(function (package)
        if package:config("includes_lua") then
            assert(package:check_cxxsnippets({test = [[
                #include <sol/sol.hpp>
                #include <cassert>
                void test() {
                    sol::state lua;
                    int x = 0;
                    lua.set_function("beep", [&x]{ ++x; });
                    lua.script("beep()");
                    assert(x == 1);
                }
            ]]}, {configs = {languages = "c++17"}}))
        end
    end)
