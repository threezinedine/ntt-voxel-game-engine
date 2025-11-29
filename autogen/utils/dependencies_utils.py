import os
from .system_info import SYSTEM
from .log import logger


def _IsFileValid(
    file: str,
    extensions: list[str],
) -> bool:
    """
    Check if the file has a valid extension.

    Arguments
    ---------
    file : str
        The file path.

    extensions : list[str]
        The list of valid extensions.

    Returns
    -------
    bool
        True if the file has a valid extension, False otherwise.
    """

    _, ext = os.path.splitext(file)
    return ext in extensions


def _LoadFilesRecursively(
    finalFiles: set[str],
    folder: str,
    extensions: list[str],
) -> set[str]:
    """
    Load all files in the folder recursively.

    Arguments
    ---------
    finalFiles : list[str]
        The list of files to append to.

    folder : str
        The folder path.

    extensions : list[str]
        The list of valid extensions.

    Returns
    -------
    list[str]
        The list of files.
    """

    if not os.path.exists(folder):
        logger.warning(f'Folder path "{folder}" does not exist, skipping...')
        return finalFiles

    for root, _folders, files in os.walk(folder):
        for file in files:
            fullPath = os.path.join(root, file)

            if _IsFileValid(fullPath, extensions):
                relPath = os.path.relpath(fullPath, SYSTEM.BaseDir)
                finalFiles.add(relPath)

        for _folder in _folders:
            _LoadFilesRecursively(finalFiles, os.path.join(root, _folder), extensions)

    return finalFiles


def AllDependenciesFiles(
    dependencies: list[str],
    extensions: list[str],
) -> list[str]:
    """
    List all files in the dependencies (folders) recursively.

    Arguments
    ---------
    dependencies : list[str]
        The list of dependency folders.

    extensions : list[str]
        The list of extensions to consider.

    Returns
    -------
    list[str]
        The list of all files in the dependencies.
    """

    allFiles: set[str] = set()
    for dep in dependencies:
        allFiles.update(
            _LoadFilesRecursively(
                allFiles,
                os.path.join(SYSTEM.BaseDir, dep),
                extensions,
            )
        )

    return list(allFiles)
