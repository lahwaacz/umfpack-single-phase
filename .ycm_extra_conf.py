flags = [
    "-std=c++11",
    "-Wall",
    "-Wextra",
    "-pedantic",
    "-Og",
    "-g",
    "-lm",
]

def FlagsForFile( filename, **kwargs ):
    return {
        "flags": flags,
        "do_cache": True,
    }
