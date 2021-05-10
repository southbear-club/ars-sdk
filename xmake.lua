-- 工程名
set_project("ars")

set_version("$(aruversion)", {build = "%Y%m%d%H%M%S"})

-- 自定义配置
set_configvar("ARS_ATHOR", "wotsen(astralrovers@outlook.com)")

-- 生成配置文件
add_configfiles("src/configure.h.in")

-- 设置版本
option("aruversion")
    set_default("1.0.0")

-- debug版本设置
if is_mode("debug") then
    add_defines("DEBUG")
    add_cflags("-g")
    set_symbols("debug")
    set_optimize("none")
else
    set_optimize("fastest")
    add_defines("NDEBUG")
end

-- 编译选项
set_warnings("all", "error")
set_languages("c99", "c++17")

-- 库目标
target("ars")
    set_kind("shared")
    set_options("debug")
    set_default(true)
    set_toolset("cxx", "clang++")

    -- 头文件
    add_includedirs("src")
    add_includedirs("include")
    add_includedirs("/usr/local/include")

    add_files("src/log/*.cpp")
    add_files("src/sdk/crypto/*.cpp")

    set_targetdir("dist/lib/")
    on_package(function (target)
        os.mkdir("dist/include/ars", "dist/docs", "dist/lib", "dist/demo")
        os.cp("samples/*", "dist/demo/")
        os.cp("docs/usr/*", "dist/docs/")
        os.cp("README.md", "dist/docs/")
        os.cp("src/ars.hpp", "dist/include/ars/")
        os.cd("dist")
        os.execv("tar -zcf ars-$(aruversion)-$(mode).tar.gz include lib demo docs")
        os.cd("..")
    end)

-- 示例代码
rule("demo")
    add_imports("core.tool.compiler")

    -- 将单个文件编译为可执行程序
    on_build(function (target, sourcefile)
        local objectfile = os.tmpfile()
        compiler.compile(sourcefile, objectfile)
    end)

target("simples")
    set_kind("binary")
    set_toolset("cxx", "clang++")

    -- 库路径
    add_linkdirs("/usr/lib")
    add_linkdirs("/usr/local/lib")
    -- 头文件
    add_includedirs("src")
    add_includedirs("include")
    add_includedirs("/usr/local/include")
    -- 库文件
    add_syslinks("gtest")
    add_syslinks("glog")
    add_syslinks("pthread")
    add_syslinks("c")

    add_files("samples/*.cpp", {rule = "demo"})

-- 单元测试
target("ut")
    set_kind("binary")
    set_toolset("cxx", "clang++")
    add_deps("ars")

    -- 库路径
    add_linkdirs("/usr/lib")
    add_linkdirs("/usr/local/lib")
    -- 头文件
    add_includedirs("src")
    add_includedirs("include")
    add_includedirs("/usr/local/include")

    -- 库文件
    add_syslinks("gtest")
    add_syslinks("glog")
    add_syslinks("pthread")
    add_syslinks("c")

    add_files("unittest/*.cpp")
