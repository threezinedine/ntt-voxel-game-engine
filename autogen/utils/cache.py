import os
import shutil
from .system_info import SYSTEM


def GetStampFilePath(filePath: str) -> str:
    """
    Get the path to the stamp file for a given file.

    Args:
        filePath (str): The path to the original file (relative to the base directory).
    Returns:
        str: The path to the corresponding stamp file.
    """

    return os.path.join(SYSTEM.BaseDir, "autogen", "temp", f"{filePath}.stamp")


def IsFileModified(filePath: str) -> bool:
    """
    Check if a file has been modified based on its cache.

    Args:
        filePath (str): The path to the file to check (relative to the base directory).
    Returns:
        bool: True if the file has been modified, False otherwise.
    """

    fullFilePath = os.path.join(SYSTEM.BaseDir, filePath)
    stampFilePath = GetStampFilePath(filePath)

    if not os.path.exists(fullFilePath):
        raise FileNotFoundError(f"File '{fullFilePath}' does not exist.")

    if not os.path.exists(stampFilePath):
        return True

    if os.path.getmtime(fullFilePath) > os.path.getmtime(stampFilePath):
        return True

    return False


def UpdateFileStamp(filePath: str) -> None:
    """
    Update the stamp file for a given file to reflect its current modification time.

    Args:
        filePath (str): The path to the original file (relative to the base directory).
    """

    stampFilePath = GetStampFilePath(filePath)

    os.makedirs(os.path.dirname(stampFilePath), exist_ok=True)
    with open(stampFilePath, "w") as file:
        file.write("")


def ClearCache() -> None:
    """
    Clear all cached stamp files.
    """

    tempDir = os.path.join(SYSTEM.BaseDir, "autogen", "temp")
    shutil.rmtree(tempDir, ignore_errors=True)
