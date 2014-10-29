flags = [
    "-std=c++11",
    "-Wall",
    "-Wextra",
    "-pedantic",
    "-O2",
    "-g",
    "-lm",
    "-I..",
]

def FlagsForFile( filename, **kwargs ):
    return {
        "flags": flags,
        "do_cache": True,
    }
