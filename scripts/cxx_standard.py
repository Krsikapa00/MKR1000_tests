# PlatformIO pre-build script: apply language/link flags that platformio.ini
# can't scope on its own.
Import("env")

env.Append(
    # build_flags reach C files too; keep the C++ standard on C++ sources only
    # (the FreeRTOS kernel is C and keeps the platform's -std=gnu11).
    CXXFLAGS=["-std=c++11"],
    # src/startup.cpp provides Reset_Handler; don't link newlib's crt0.
    LINKFLAGS=["-nostartfiles"],
)
