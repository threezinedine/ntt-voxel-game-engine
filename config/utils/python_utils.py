import os
import shutil
from typing import Any
from ..log import logger
from .utils import RunCommand
from ..system_info import SYSTEM


def GetPythonExecutable(
    project: str,
) -> str:
    """
    Get the path to the Python executable for a given project.
    Arguments
    ---------
    project : str
        The project to get the Python executable for.
    Returns
    -------
    str
        The path to the Python executable.
    """
    if SYSTEM.IsWindowsPlatform:
        return os.path.join(
            SYSTEM.BaseDir,
            project,
            "venv",
            "Scripts",
            "python.exe",
        )
    else:
        return os.path.join(
            SYSTEM.BaseDir,
            project,
            "venv",
            "bin",
            "python",
        )


def GetPipExecutable(
    project: str,
) -> str:
    """
    Get the path to the pip executable for a given project.
    Arguments
    ---------
    project : str
        The project to get the pip executable for.
    Returns
    -------
    str
        The path to the pip executable.
    """
    if SYSTEM.IsWindowsPlatform:
        return os.path.join(
            SYSTEM.BaseDir,
            project,
            "venv",
            "Scripts",
            "pip.exe",
        )
    else:
        return os.path.join(
            SYSTEM.BaseDir,
            project,
            "venv",
            "bin",
            "pip",
        )


def GetPytestExecutable(
    project: str,
) -> str:
    """
    Get the path to the pytest executable for a given project.
    Arguments
    ---------
    project : str
        The project to get the pytest executable for.
    Returns
    -------
    str
        The path to the pytest executable.
    """
    if SYSTEM.IsWindowsPlatform:
        return os.path.join(
            SYSTEM.BaseDir,
            project,
            "venv",
            "Scripts",
            "pytest.exe",
        )
    else:
        return os.path.join(
            SYSTEM.BaseDir,
            project,
            "venv",
            "bin",
            "pytest",
        )


def CreateEnvironment(
    dir: str,
    force: bool = False,
    **kwargs: Any,
) -> None:
    """
    Create the development environment for a given project.
    Arguments
    ---------
    dir : str
        The project to create the environment for.
    """
    if force and os.path.exists(os.path.join(SYSTEM.BaseDir, dir, "venv")):
        logger.info(f"Removing existing virtual environment for project: {dir}")
        shutil.rmtree(os.path.join(SYSTEM.BaseDir, dir, "venv"))

    if os.path.exists(os.path.join(SYSTEM.BaseDir, dir, "venv")):
        logger.debug(f"Virtual environment already exists for project: {dir}")
        return

    RunCommand(
        f"{SYSTEM.PythonCommand} -m venv venv",
        cwd=os.path.join(SYSTEM.BaseDir, dir),
    )

    pipExecutable = GetPipExecutable(dir)
    RunCommand(
        f'"{pipExecutable}" install --upgrade pip',
        cwd=os.path.join(SYSTEM.BaseDir, dir),
    )

    RunCommand(
        f'"{pipExecutable}" install -r requirements.txt',
        cwd=os.path.join(SYSTEM.BaseDir, dir),
    )


def InstallPythonDependencies(
    project: str,
    dependencies: list[str],
    **kwargs: Any,
) -> None:
    """
    Install additional dependencies for a given project.
    Arguments
    ---------
    project : str
        The project to install dependencies for.
    dependencies : list[str]
        The list of dependencies to install.
    """
    if not dependencies:
        logger.debug(f"No additional dependencies to install for project: {project}")
        return

    pipExecutable = GetPipExecutable(project)
    depsString = " ".join(dependencies)
    RunCommand(
        f'"{pipExecutable}" install {depsString}',
        cwd=os.path.join(SYSTEM.BaseDir, project),
    )

    RunCommand(
        f'"{pipExecutable}" freeze > requirements.txt',
        cwd=os.path.join(SYSTEM.BaseDir, project),
    )


def RunAutogen(
    reload: bool = False,
    **kwargs: Any,
) -> None:
    """
    Run the autogen script to generate necessary files.
    """
    logger.info("Generating needed files...")
    RunCommand(
        f'"{GetPythonExecutable("autogen")}" main.py {"--reload" if reload else ""}',
        cwd="autogen",
    )
