import os
import shutil

CURRENT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def GetFileCachePath(filename: str) -> str:
    """
    Get the path to a file in the file cache directory.

    Arguments
    ---------
    cacheFileName : str
        The name of the cache file.

    Returns
    -------
    str
        The path to the cache file.
    """
    cacheDir = os.path.join(CURRENT_DIR, "temp")

    if not os.path.exists(cacheDir):
        os.makedirs(cacheDir)

    return os.path.join(cacheDir, filename)


def ClearCache() -> None:
    """
    Clear the file cache directory.
    """
    cacheDir = os.path.join(CURRENT_DIR, "temp")

    shutil.rmtree(cacheDir, ignore_errors=True)


def IsFileModified(
    file: str,
) -> bool:
    """
    Check if the file has been modified since last cached.

    Arguments
    ---------
    file : str
        The file path (relative with current directory).

    Returns
    -------
    bool
        True if the file has been modified, False otherwise.
    """

    cacheFilePath = GetFileCachePath(file)

    if not os.path.exists(cacheFilePath):
        return True

    return os.path.getmtime(file) > os.path.getmtime(cacheFilePath)


def UpdateFileCache(
    file: str,
) -> None:
    """
    Update the file cache with the current file.

    Arguments
    ---------
    file : str
        The file path (relative with current directory).
    """

    cacheFilePath = GetFileCachePath(file)

    os.makedirs(os.path.dirname(cacheFilePath), exist_ok=True)

    with open(cacheFilePath, "w") as f:
        f.write("")
