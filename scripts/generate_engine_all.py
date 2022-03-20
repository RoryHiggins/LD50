"""
Script to generate an entrypoint for loading all engine code and running
engine tests
"""
import pathlib

strs = [
    'require("engine/core/debugging").set_debugger_enabled(true)',
    "local function main()",
]
strs += [
    "\trequire('{lua_path}')".format(lua_path=path.with_suffix('').as_posix())
    for path in pathlib.Path('engine').glob('**/*.lua')
]
strs += [
    '\trequire("engine/core/testing").run_all()'
]

strs += [
    "end",
    # "main()",
    'require("engine/core/debugging").pcall(main)'
]

print('\n'.join(strs))
