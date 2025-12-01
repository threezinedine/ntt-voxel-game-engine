import argparse
from typing import Dict, Any
from .system_info import SYSTEM


class Args:
    def __init__(self):
        parser = argparse.ArgumentParser(
            description="meed-voxel-game-engine configuration"
        )

        parser.add_argument(
            "--verbose",
            "-v",
            action="store_true",
            help="Enable verbose logging",
        )

        parser.add_argument(
            "--force",
            "-f",
            action="store_true",
            help="Force to re-create environments or rebuild projects",
        )

        parser.add_argument(
            "--type",
            "-t",
            choices=["debug", "release", "web"],
            default="debug",
            help="Type of run (debug or release)",
        )

        subparser = parser.add_subparsers(dest="command", required=True)

        autogenParser = subparser.add_parser("autogen", help="Run the autogen process")

        autogenParser.add_argument(
            "--reload",
            "-r",
            action="store_true",
            help="Reload all templates, ignoring cache",
        )

        buildParser = subparser.add_parser("build", help="Build the project")
        buildParser.add_argument(
            "project",
            choices=["app", "engine", "editor"],
            help="Project to build (engine or editor)",
        )

        packageSubParser = subparser.add_parser(
            "package",
            help="Install the dependencies required for building the project",
        )

        packageSubParser.add_argument(
            "--dependencies",
            "-d",
            nargs="+",
            type=str,
            default=[],
            help="Additional dependencies to install",
        )

        packageSubParser.add_argument(
            "--save-dev",
            "-s",
            action="store_true",
            help="Save the installed dependencies to dev-requirements.txt (Node projects only)",
        )

        packageSubParser.add_argument(
            "project",
            type=str,
            choices=["editor", "autogen"],
            help="Project to install dependencies for (editor or autogen)",
        )

        runSubParser = subparser.add_parser("run")

        runSubParser.add_argument(
            "project",
            type=str,
            choices=["app", "editor"],
            default="editor",
        )

        exampleSubParser = subparser.add_parser(
            "example",
            help="Run an example project",
        )

        exampleSubParser.add_argument(
            "--examples",
            "-e",
            type=str,
            nargs="+",
            help="Name of the example project to run",
        )

        subparser.add_parser(
            "designer",
            help="Run the GUI designer tool",
        )

        testSubParser = subparser.add_parser(
            "test",
            help="Run the test suite",
        )

        testSubParser.add_argument(
            "project",
            type=str,
            choices=["autogen", "editor", "engine"],
            help="Project to test",
        )

        testSubParser.add_argument(
            "--filter",
            "-f",
            type=str,
            default=None,
            help="Filter tests to run by name",
        )

        self.args = parser.parse_args()

    @property
    def Args(self) -> Dict[str, Any]:
        """
        Always be passed to the python util methods as a dictionary.

        Example
        -------
        >>> from config.args import Args
        >>> args = Args()
        >>> runProject(**args.Args) # equivalent to runProject(type=args.asrgs.type, command=args.asrgs.command)
        """
        return vars(self.args)

    @property
    def IsBuild(self) -> str:
        return self.args.command == "build"

    @property
    def IsPackage(self) -> str:
        return self.args.command == "package"

    @property
    def IsRun(self) -> str:
        return self.args.command == "run"

    @property
    def IsRunExample(self) -> str:
        return self.args.command == "example"

    @property
    def IsRunAutogen(self) -> str:
        return self.args.command == "autogen"

    @property
    def IsRunDesigner(self) -> str:
        return self.args.command == "designer"

    @property
    def IsRunTest(self) -> str:
        return self.args.command == "test"

    @property
    def IsPythonProject(self) -> bool:
        assert self.args is not None, "Args is not initialized."
        assert self.args.project is not None, "Project name is not specified."
        return self.args.project in SYSTEM.PythonProjects

    @property
    def IsCProject(self) -> bool:
        assert self.args is not None, "Args is not initialized."
        assert self.args.project is not None, "Project name is not specified."
        return self.args.project in SYSTEM.CProjects
