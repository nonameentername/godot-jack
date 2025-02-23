#!/usr/bin/env python
import os
import sys


def normalize_path(val, env):
    return val if os.path.isabs(val) else os.path.join(env.Dir("#").abspath, val)


def validate_parent_dir(key, val, env):
    if not os.path.isdir(normalize_path(os.path.dirname(val), env)):
        raise UserError("'%s' is not a directory: %s" % (key, os.path.dirname(val)))

def set_build_info(source, target):
    with open(source, 'r') as f:
        content = f.read()

    content = content.replace("${version}", os.environ.get('TAG_VERSION', '0.0.0'))
    content = content.replace("${build}", os.environ.get('BUILD_SHA', 'none'))

    with open(target, 'w') as f:
        f.write(content) 


build_info_source_files = ["templates/jackgodot.gdextension.in",
                           "templates/version_generated.gen.h.in"]

build_info_target_files = ["addons/jack/bin/jackgodot.gdextension",
                           "src/version_generated.gen.h"]


def create_build_info_files(target, source, env):
    for i in range(len(build_info_source_files)):
        set_build_info(build_info_source_files[i], build_info_target_files[i])


def create_build_info_strfunction(target, source, env):
    return f"Creating build info files: {', '.join(str(t) for t in target)}"


libname = "libjackgodot"
projectdir = "."

if sys.platform == "windows":
    localEnv = Environment(tools=["mingw"], PLATFORM="")
else:
    localEnv = Environment(tools=["default"], PLATFORM="")

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
opts.Add(
    BoolVariable(
        key="compiledb",
        help="Generate compilation DB (`compile_commands.json`) for external tools",
        default=localEnv.get("compiledb", False),
    )
)
opts.Add(
    PathVariable(
        key="compiledb_file",
        help="Path to a custom `compile_commands.json` file",
        default=localEnv.get("compiledb_file", "compile_commands.json"),
        validator=validate_parent_dir,
    )
)
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()
env["compiledb"] = False

env.Tool("compilation_db")
compilation_db = env.CompilationDatabase(
    normalize_path(localEnv["compiledb_file"], localEnv)
)
env.Alias("compiledb", compilation_db)

env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

jack_library = "jack"
env.Append(LIBS=[jack_library])

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

file = "{}{}{}".format(libname, env["suffix"], env["SHLIBSUFFIX"])

if env["platform"] == "macos":
    platlibname = "{}.{}.{}".format(libname, env["platform"], env["target"])
    file = "{}.framework/{}".format(env["platform"], platlibname, platlibname)

libraryfile = "addons/jack/bin/{}/{}".format(env["platform"], file)
library = env.SharedLibrary(
    libraryfile,
    source=sources,
)

create_build_info_action = Action(create_build_info_files,
                                  strfunction=create_build_info_strfunction)

env.Command(
    build_info_target_files,
    build_info_source_files,
    create_build_info_action
)

env.AddPreAction(library, create_build_info_action)
env.Depends(library, build_info_target_files)

#copy = env.InstallAs("{}/bin/{}/lib{}".format(projectdir, env["platform"], file), library)

default_args = [library]
if localEnv.get("compiledb", False):
    default_args += [compilation_db]
Default(*default_args)
