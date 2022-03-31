"""
Script to generate an entrypoint for loading all engine code and running
engine tests
"""
import pathlib
import argparse


def to_lua_path(path: pathlib.Path) -> str:
    """
    Converts abs or relative filesystem path to lua relative import path
    """
    if path.is_absolute():
        working_dir = pathlib.Path(".").resolve()
        path = path.relative_to(working_dir)

    return path.with_suffix('').as_posix()


def main():
    """
    Generate entrypoint for running engine tests and app from a debugger
    """
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument(
        "--entrypoint", type=str, default=None)
    arg_parser.add_argument(
        "--debugger", action=argparse.BooleanOptionalAction, default=True)

    args = arg_parser.parse_args()

    run_main = ('require("engine/core/debugging").pcall(main)'
                if args.debugger else "main()")
    entrypoint = (f'require("{to_lua_path(pathlib.Path(args.entrypoint))}")'
                  if args.entrypoint else "-- entrypoint")

    paths = pathlib.Path('engine').glob('**/*.lua')
    requires = '\n\t'.join(f"require('{to_lua_path(path)}')" for path in paths)

    lua_script = (
        f"local function main()\n"
        f'\t{requires}\n'
        f'\trequire("engine/core/testing").run_all()\n'
        f"\t{entrypoint}\n"
        f"end\n"
        f'local Debugging = require("engine/core/debugging")\n'
        f'Debugging.debug_checks_enabled = true\n'
        f'Debugging.expensive_debug_checks_enabled = true\n'
        f'Debugging.set_debugger_enabled(true)\n'
        f"{run_main}"
    )

    print(lua_script)


if __name__ == '__main__':
    main()
