target_include_directories(lua PUBLIC .)
target_sources(lua PUBLIC lapi.h lauxlib.h lcode.h ldebug.h ldo.h lfunc.h lgc.h llex.h llimits.h lmem.h lobject.h lopcodes.h lparser.h lstate.h lstring.h ltable.h ltm.h lua.h luaconf.h lualib.h lundump.h lvm.h lzio.h)

target_sources(lua PRIVATE lapi.c lbaselib.c ldblib.c ldo.c lfunc.c linit.c llex.c lmem.c lobject.c loslib.c lstate.c lstrlib.c ltablib.c lundump.c lzio.c lauxlib.c lcode.c ldebug.c ldump.c lgc.c liolib.c lmathlib.c loadlib.c lopcodes.c lparser.c lstring.c ltable.c ltm.c luac.c lvm.c print.c)
