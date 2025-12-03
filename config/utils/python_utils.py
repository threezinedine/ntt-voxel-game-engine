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


def GetDesignerExecutable(
    project: str,
) -> str:
    """
    Get the path to the Qt Designer executable for a given project.
    Arguments
    ---------
    project : str
        The project to get the Qt Designer executable for.
    Returns
    -------
    str
        The path to the Qt Designer executable.
    """
    if SYSTEM.IsWindowsPlatform:
        return os.path.join(
            SYSTEM.BaseDir,
            project,
            "venv",
            "Scripts",
            "pyside6-designer.exe",
        )
    else:
        return os.path.join(
            SYSTEM.BaseDir,
            project,
            "venv",
            "bin",
            "pyside6-designer",
        )


def GetPyinstallerExecutable(
    project: str,
) -> str:
    """
    Get the path to the PyInstaller executable for a given project.
    Arguments
    ---------
    project : str
        The project to get the PyInstaller executable for.
    Returns
    -------
    str
        The path to the PyInstaller executable.
    """
    if SYSTEM.IsWindowsPlatform:
        return os.path.join(
            SYSTEM.BaseDir,
            project,
            "venv",
            "Scripts",
            "pyinstaller.exe",
        )
    else:
        return os.path.join(
            SYSTEM.BaseDir,
            project,
            "venv",
            "bin",
            "pyinstaller",
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
        f"{SYSTEM.PythonCommand} -m pip install --upgrade pip",
        cwd=os.path.join(SYSTEM.BaseDir, dir),
    )

    RunCommand(
        f"{pipExecutable} install -r requirements.txt",
        cwd=os.path.join(SYSTEM.BaseDir, dir),
    )


def RunEditorConvertUI(
    reload: bool = False,
    **kwargs: Any,
) -> None:
    """
    Run the UI conversion process for the editor project.
    """
    if reload:
        shutil.rmtree(
            os.path.join(SYSTEM.BaseDir, "editor", "pyui"), ignore_errors=True
        )

    logger.info("Converting .ui files to .py files...")
    RunCommand(
        f'{GetPythonExecutable("editor")} convert.py',
        cwd=os.path.join(SYSTEM.BaseDir, "editor"),
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
        f"{pipExecutable} install {depsString}",
        cwd=os.path.join(SYSTEM.BaseDir, project),
    )

    RunCommand(
        f"{pipExecutable} freeze > requirements.txt",
        cwd=os.path.join(SYSTEM.BaseDir, project),
    )


def RunAutogen(
    reload: bool = False,
    verbose: bool = False,
    **kwargs: Any,
) -> None:
    """
    Run the autogen script to generate necessary files.
    """
    logger.info("Generating needed files...")
    RunCommand(
        f'{GetPythonExecutable("autogen")} main.py {"--reload " if reload else ""}{"--verbose" if verbose else ""}',
        cwd="autogen",
    )


def RunEditor(
    **kwargs: Any,
) -> None:
    """
    Launch the code editor.
    """
    logger.info("Launching code editor...")
    RunCommand(
        f'{GetPythonExecutable("editor")} main.py',
        cwd="editor",
    )


def BuildEditor(
    **kwargs: Any,
) -> None:
    """
    Build the editor project.
    """
    logger.info("Building editor project...")

    assetDir = os.path.join(SYSTEM.BaseDir, "editor", "assets")
    iconPath = os.path.join(assetDir, "icons", "meed-log.ico")

    RunCommand(
        f'{GetPyinstallerExecutable("editor")} --onefile main.py --name Meed --icon {iconPath}',
        cwd="editor",
    )

    if SYSTEM.IsWindowsPlatform:
        distPath = os.path.join(SYSTEM.BaseDir, "editor", "dist", "Meed.exe")
        outputPath = os.path.join(SYSTEM.BaseDir, "bin", "Meed.exe")
    else:
        distPath = os.path.join(SYSTEM.BaseDir, "editor", "dist", "Meed")
        outputPath = os.path.join(SYSTEM.BaseDir, "bin", "Meed")

    os.makedirs(os.path.join(SYSTEM.BaseDir, "bin"), exist_ok=True)
    shutil.copyfile(distPath, outputPath)

    if SYSTEM.IsLinuxPlatform:
        os.chmod(outputPath, 0o755)


def OpenDesigner(
    **kwargs: Any,
) -> None:
    """
    Launch the GUI designer tool.
    """
    logger.info("Launching GUI designer...")
    RunCommand(
        f'{GetDesignerExecutable("editor")}',
        cwd="editor",
    )


def RunPythonTest(
    project: str,
    filter: str | None = None,
    **kwargs: Any,
) -> None:
    """
    Run the test suite for the specified project.

    Arguments:
        project (str): The project to run tests for.
    """
    logger.info("Running test suite...")

    RunCommand(
        f'{GetPytestExecutable(project)} {"-k " + filter if filter else ""}',
        cwd=os.path.join(SYSTEM.BaseDir, project),
    )
