package("tiltedcore")
    set_homepage("https://github.com/tiltedphoques/TiltedCore")
    set_description("Core library from Tilted Phoques")

    add_urls("https://github.com/tiltedphoques/TiltedCore/archive/$(version).zip")
    add_urls("https://github.com/tiltedphoques/TiltedCore.git")

    add_versions("v0.2.9", "aa8dd5aee40c53fd57da91ef5193458b62f3dec420e78fac9dc8293a987b9758")
    add_versions("v0.2.7", "dbc9fcee3706e91a9fbe00648c3593c8b8f0ae9208fb510e756ae6bee8931b93")

    add_configs("shared", {description = "Build shared library.", default = false, type = "boolean", readonly = true})

    -- Keep mimalloc on 2.x; TiltedCore Tests also omit mimalloc from the link line.
    add_deps("mimalloc 2.2.4", {configs = {rltgenrandom = true}})

    on_check("windows", function (package)
        local msvc = package:toolchain("msvc")
        local vs = msvc:config("vs")
        if vs and tonumber(vs) < 2019 then
            raise("package(tiltedcore): VS2019 or newer is required.")
        end
    end)

    on_install("windows", "mingw", "linux", "cross", "android", "macosx|!arm*", function (package)
        -- Only build/install the library. The Tests binary fails to link mi_malloc_size
        -- because TiltedCore's xmake.lua does not add mimalloc to the Tests target.
        import("package.tools.xmake").install(package, {}, {targets = "TiltedCore"})
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            void test(int args, char** argv) {
                TiltedPhoques::Outcome<int, float> outcome;
            }
        ]]}, {includes = {"TiltedCore/Outcome.hpp"}}))
    end)
