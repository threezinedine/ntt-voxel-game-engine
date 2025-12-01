import os
import re
import json
import shutil
from typing import Any

from ..conf import VARIABLES
from ..log import logger
from .utils import RunCommand
from ..system_info import SYSTEM
from dataclasses import dataclass


@dataclass
class Depedency:
    name: str
    url: str


def InstallCDependencies(
    **kwargs: Any,
) -> None:
    """
    Installs necessary dependencies for the C++ project which is loaded from the `dependecies.json` file. The
        code will be placed at `external/` folder.
    """
    dependencies: list[Depedency] = []

    with open("dependencies.json", "r") as depFile:
        dependenciesData = json.load(depFile)

        for depData in dependenciesData:
            dependency = Depedency(**depData)
            dependencies.append(dependency)

    externalDir = os.path.join(os.getcwd(), "externals")
    if not os.path.exists(externalDir):
        logger.debug(f'Creating external directory at "{externalDir}"')
        os.makedirs(externalDir)

    for dependency in dependencies:
        dependencyDir = os.path.join(externalDir, dependency.name)
        if os.path.exists(dependencyDir):
            logger.debug(
                f"Dependency {dependency.name} already exists at {dependencyDir}, skipping..."
            )
            continue

        logger.info(
            f'Cloning dependency "{dependency.name}" from "{dependency.url}"...'
        )
        RunCommand(f"git clone {dependency.url} {dependencyDir}")


def BuildCProject(
    project: str = "",
    type: str = "debug",
    force: bool = False,
    reload: bool = False,
    **kwargs: Any,
) -> None:
    """
    Builds a C++ project using CMake. Supports different build types and platforms.

    Arguments:
        project (str): The path to the C++ project.
        web (bool): Whether to build for web platform using Emscripten. Defaults to False.
        type (str): The build type, either 'debug' or 'release'. Defaults to 'debug'.

    """
    if force or reload:
        buildDir = os.path.join(SYSTEM.BaseDir, project, "build", type)
        if os.path.exists(buildDir):
            logger.info(
                f'Removing existing build directory at "{buildDir}" for reload...'
            )

            shutil.rmtree(buildDir)

    configDir = os.path.join(SYSTEM.BaseDir, project, "config")
    additionalOptions = ReadConfigFile(f"{type}.cfg", configDir)

    if SYSTEM.IsWindowsPlatform:
        additionalOptions = "-G Visual Studio 17 2022"

    if type.lower() == "release" or type.lower() == "web":
        additionalOptions += " -DCMAKE_BUILD_TYPE=Release"
    elif type.lower() == "debug":
        additionalOptions += " -DCMAKE_BUILD_TYPE=Debug"

    prefix = ""

    if type == "web":
        prefix = f"{VARIABLES.EMCMAKE}"

    logger.info(f'Building project "{project}" with build type "{type}"...')
    RunCommand(f"{prefix} cmake -S . -B build/{type} {additionalOptions}", cwd=project)
    RunCommand(f"cmake --build build/{type} --config {type.capitalize()}", cwd=project)


def ReadConfigFile(name: str, folder: str) -> str:
    """
    Reads a configuration file and returns its content as a string.

    Arguments:
        name (str): The name of the configuration file.
        folder (str): The folder where the configuration file is located.

    Returns:
        str: The content of the configuration file.
    """
    configPath = os.path.join(folder, name)
    if not os.path.exists(configPath):
        logger.warning(f'Configuration file "{configPath}" does not exist.')
        return ""

    additionalOptions = ""

    with open(configPath, "r") as cfgFile:
        content = cfgFile.read()
        for index, line in enumerate(content.splitlines()):
            line = line.strip()

            if line == "" or line.startswith("//") or line.startswith("#"):
                continue

            if re.match(r"^[A-Za-z0-9_\-]+=(.*)$", line):
                additionalOptions += f" -D{line}"
                continue

            includeMatch = re.match(r"^<[A-Za-z0-9_\-]+.cfg>$", line)

            if includeMatch:
                includeFileName = line[1:-1]
                logger.debug(
                    f'Including configuration file "{includeFileName}" from line {index + 1} in "{name}".'
                )
                includedOptions = ReadConfigFile(includeFileName, folder)
                additionalOptions += f" {includedOptions}"
                continue

            logger.warning(
                f'Unrecognized line "{line}" in configuration file "{name}" at line {index + 1}.'
            )

    return additionalOptions


def RunTestEngine(
    filter: str = "",
    **kwargs: Any,
) -> None:
    """
    Runs the test suite for the engine project.

    Arguments:
        filter (str): Optional filter to run specific tests by name.
    """

    logger.info("Running engine tests...")

    if SYSTEM.IsWindowsPlatform:
        engineTestDir = os.path.join(
            SYSTEM.BaseDir, "engine", "build", "debug", "tests", "Debug"
        )
        testCommand = "MEEDEngineTests.exe"
    else:
        engineTestDir = os.path.join(
            SYSTEM.BaseDir, "engine", "build", "debug", "tests"
        )
        testCommand = "./MEEDEngineTests"

    if filter:
        testCommand += f' --gtest_filter="{filter}"'

    RunCommand(testCommand, cwd=engineTestDir)


def RunExample(
    examples: list[str],
    type: str = "debug",
    **kwargs: Any,
) -> None:
    """
    Runs example projects.

    Arguments:
        examples (list[str]): List of example project names to run.
    """
    if SYSTEM.IsWindowsPlatform:
        exampleDir = os.path.join(
            SYSTEM.BaseDir,
            "engine",
            "build",
            type,
            type.capitalize(),
            "examples",
        )
    elif SYSTEM.IsLinuxPlatform:
        exampleDir = os.path.join(
            SYSTEM.BaseDir,
            "engine",
            "build",
            type,
            "examples",
        )
    else:
        raise NotImplementedError(
            "Current platform is not supported for running examples."
        )

    BuildCProject("engine", type=type, **kwargs)

    logger.debug(f"Example directory resolved to: {exampleDir}")

    for example in examples:
        if not os.path.exists(os.path.join(exampleDir, example)):
            logger.error(
                f'Example project "{example}" does not exist at path "{os.path.join(exampleDir, example)}".'
            )
            continue

        logger.info(f'Running example project "{example}"...')

        if SYSTEM.IsWindowsPlatform:
            RunCommand(f"{example}.exe", cwd=exampleDir)
        elif SYSTEM.IsLinuxPlatform:
            RunCommand(f"./{example}", cwd=exampleDir)


def RunApplication(
    type: str = "debug",
    **kwargs: Any,
) -> None:
    """
    Runs the main application.

    Arguments:
        type (str): The build type, either 'debug' or 'release'. Defaults to 'debug'.
    """
    BuildCProject(**(kwargs | dict(project="app", type=type)))

    if type == "web":
        RunWebApplication(**kwargs)
        return

    if SYSTEM.IsWindowsPlatform:
        appDir = os.path.join(
            SYSTEM.BaseDir,
            "app",
            "build",
            type,
            type.capitalize(),
        )
    elif SYSTEM.IsLinuxPlatform:
        appDir = os.path.join(
            SYSTEM.BaseDir,
            "app",
            "build",
            type,
        )
    else:
        raise NotImplementedError(
            "Current platform is not supported for running the application."
        )

    logger.debug(f"Application directory resolved to: {appDir}")

    logger.info(f"Running application...")

    if SYSTEM.IsWindowsPlatform:
        RunCommand(f"MEEDApp.exe", cwd=appDir)
    elif SYSTEM.IsLinuxPlatform:
        RunCommand(f"./MEEDApp", cwd=appDir)


def RunWebApplication(
    port: int = 8080,
    **kwargs: Any,
) -> None:
    """
    Runs the web application using Emscripten.

    Arguments:
        None
    """

    appDir = os.path.join(
        SYSTEM.BaseDir,
        "app",
        "public",
    )

    logger.info(f"Running web application on http://localhost:{port}...")

    RunCommand(f"{SYSTEM.PythonCommand} server.py", cwd=appDir)
