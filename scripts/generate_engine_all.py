"""
Script to generate an entrypoint for loading all engine code and running
engine tests
"""
import pathlib

file_expressions = [
    "require('{lua_path}')".format(
        lua_path=path.with_suffix('').as_posix()
    )
    for path in pathlib.Path('engine').glob('**/*.lua')
]
expressions = file_expressions + [
    'require("engine/core/testing").run_all()'
]

print('\n'.join(expressions))
