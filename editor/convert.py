import os
import logging
import argparse
from utils import convertLogger, IsFileModified, UpdateFileCache


PYUI_DIR = "pyui"
ASSETS_DIR = os.path.join("assets")
UIS_DIR = os.path.join(ASSETS_DIR, "uis")
ICONS_DIR = os.path.join(ASSETS_DIR, "icons")
IMAGES_DIR = os.path.join(ASSETS_DIR, "images")

if os.name == "nt":
    PYUIC_PATH = os.path.join(
        "venv",
        "Scripts",
        "pyside6-uic.exe",
    )

    PYRCC_PATH = os.path.join(
        "venv",
        "Scripts",
        "pyside6-rcc.exe",
    )
else:
    PYUIC_PATH = os.path.join(
        "venv",
        "bin",
        "pyside6-uic",
    )

    PYRCC_PATH = os.path.join(
        "venv",
        "bin",
        "pyside6-rcc",
    )


def ConvertUIFileToPython(uiFilePath: str, outputPath: str) -> None:
    """
    Transfer .ui file into .py file using PySide6's pyside6-uic tool.

    Arguments
    ---------
    uiFilePath : str
        The path to the .ui file (relative path to the editor directory)
    outputPath : str
        The path to the output .py file (relative path to the editor directory)
    """


def main():
    parser = argparse.ArgumentParser(
        description="Convert .ui files to .py files using PySide6's pyside6-uic tool."
    )

    parser.add_argument(
        "--verbose",
        "-v",
        action="store_true",
        help="Enable verbose logging",
    )

    args = parser.parse_args()

    if args.verbose:
        convertLogger.setLevel(logging.DEBUG)
    else:
        convertLogger.setLevel(logging.INFO)

    allUIFiles = os.listdir(UIS_DIR)

    if not os.path.exists(PYUI_DIR):
        convertLogger.info(
            f'Creating directory "{PYUI_DIR}" for storing converted UI files...'
        )
        os.makedirs(PYUI_DIR)

    initFileContent = (
        "# This file is auto-generated to mark the pyui directory as a package.\n"
    )

    isAnyUIFileChanged = False

    for fileName in allUIFiles:
        if fileName.endswith(".ui"):
            uiFilePath = os.path.join(UIS_DIR, fileName)
            outputFileName = fileName.replace(".ui", "_ui.py")
            outputPath = os.path.join(PYUI_DIR, outputFileName)

            initFileContent += f'from .{outputFileName.replace(".py", "")} import *\n'

            if os.path.exists(outputPath) and not IsFileModified(uiFilePath):
                convertLogger.debug(
                    f'Skipping conversion for "{uiFilePath}" as the output file is up to date.'
                )
                continue

            isAnyUIFileChanged = True
            convertLogger.info(f'Converting "{uiFilePath}" to "{outputPath}"...')
            os.system(f'{PYUIC_PATH} "{uiFilePath}" -o "{outputPath}"')

            UpdateFileCache(uiFilePath)

    if isAnyUIFileChanged:
        with open(os.path.join(PYUI_DIR, "__init__.py"), "w") as initFile:
            initFile.write(initFileContent)

    qrcFile = "resources.qrc"
    if os.path.exists(qrcFile):
        outputPath = os.path.join("resources_rc.py")
        if not os.path.exists(outputPath) or IsFileModified(qrcFile):
            convertLogger.info(f'Converting "{qrcFile}" to "{outputPath}"...')
            os.system(f'{PYRCC_PATH} "{qrcFile}" -o "{outputPath}"')

            UpdateFileCache(qrcFile)


if __name__ == "__main__":
    main()
