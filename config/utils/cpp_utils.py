import os
import json
import shutil
from typing import Any
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
    web: bool = False,
    type: str = "debug",
    force: bool = False,
    **kwargs: Any,
) -> None:
    """
    Builds a C++ project using CMake. Supports different build types and platforms.

    Arguments:
        project (str): The path to the C++ project.
        web (bool): Whether to build for web platform using Emscripten. Defaults to False.
        type (str): The build type, either 'debug' or 'release'. Defaults to 'debug'.

    """
    if web:
        BuildEngineWebLib(project=project, **kwargs)
    else:
        if force:
            buildDir = os.path.join(SYSTEM.BaseDir, project, "build", type)
            if os.path.exists(buildDir):
                logger.info(
                    f'Removing existing build directory at "{buildDir}" for reload...'
                )

                shutil.rmtree(buildDir)

        additionalOptions = ""

        configFile = os.path.join(SYSTEM.BaseDir, project, f"{type}.cfg")

        if os.path.exists(configFile):
            with open(configFile, "r") as cfgFile:
                for line in cfgFile:
                    line = line.strip()
                    if line and not line.startswith("#"):
                        additionalOptions += f" -D{line}"

        if SYSTEM.IsWindowsPlatform:
            additionalOptions = "-G Visual Studio 17 2022"

        if type.lower() == "release":
            additionalOptions += " -DCMAKE_BUILD_TYPE=Release"
        else:
            additionalOptions += " -DCMAKE_BUILD_TYPE=Debug"

        logger.info(f'Building project "{project}" with build type "{type}"...')
        RunCommand(f"cmake -S . -B build/{type} {additionalOptions}", cwd=project)
        RunCommand(
            f"cmake --build build/{type} --config {type.capitalize()}", cwd=project
        )


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
    web: bool = False,
    **kwargs: Any,
) -> None:
    """
    Runs the main application.

    Arguments:
        type (str): The build type, either 'debug' or 'release'. Defaults to 'debug'.
    """
    if web:
        RunApplicationWeb(type=type, **kwargs)
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

    BuildCProject(**(kwargs | dict(project="app")))

    logger.debug(f"Application directory resolved to: {appDir}")

    logger.info(f"Running application...")

    if SYSTEM.IsWindowsPlatform:
        RunCommand(f"MEEDApp.exe", cwd=appDir)
    elif SYSTEM.IsLinuxPlatform:
        RunCommand(f"./MEEDApp", cwd=appDir)


def RunApplicationWeb(
    type: str = "debug",
    **kwargs: Any,
) -> None:
    pass


def BuildEngineWebLib(
    project: str = "engine",
    **kwargs: Any,
) -> None:
    from ..conf import VARIABLES

    assert (
        VARIABLES.EMCMAKE is not None
    ), '"EMCMAKE" variable is not set in the configuration.'

    projectDir = os.path.join(SYSTEM.BaseDir, project)

    RunCommand(
        f"{VARIABLES.EMCMAKE} cmake -S . -B build/webruntime -DCMAKE_BUILD_TYPE=Release -DEMCC_FORCE_STDLIBS=ON",
        cwd=projectDir,
    )

    RunCommand(
        f"cmake --build build/webruntime --config Release",
        cwd=projectDir,
    )
