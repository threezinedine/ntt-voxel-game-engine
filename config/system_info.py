import os
import platform
import sysconfig


class SystemInfo:
    """
    This class is used to tracking the whole application constants which are
    used throughout the application.
    """

    def __init__(self) -> None:
        self._pythonCommand = "python3" if self.IsLinuxPlatform else "python"
        self._baseDir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        self._compilerPath = ""
        self._intelliSenseMode = ""

        if self.IsWindowsPlatform:
            self._compilerPath = "C:/Program Files/LLVM/bin/clang.exe"
            self._intelliSenseMode = "windows-clang-x64"
        elif self.IsLinuxPlatform:
            self._compilerPath = "/usr/bin/g++"
            self._intelliSenseMode = "linux-gcc-arm64"

        self._pythonIncludeDir = sysconfig.get_path("include")

    @property
    def IsWindowsPlatform(self) -> bool:
        return platform.system() == "Windows"

    @property
    def IsLinuxPlatform(self) -> bool:
        return platform.system() == "Linux"

    @property
    def PythonCommand(self) -> str:
        return self._pythonCommand

    @property
    def BaseDir(self) -> str:
        return self._baseDir

    @property
    def PythonProjects(self) -> list[str]:
        return ["autogen", "editor"]

    @property
    def CProjects(self) -> list[str]:
        return ["engine", "app"]

    @property
    def CCompiler(self) -> str:
        return self._compilerPath

    @property
    def IntelliSenseMode(self) -> str:
        return self._intelliSenseMode

    @property
    def PythonInterpreter(self) -> str:
        if self.IsWindowsPlatform:
            return "venv/Scripts/python.exe"
        elif self.IsLinuxPlatform:
            return "venv/bin/python3"
        else:
            raise NotImplementedError("Unsupported platform")

    @property
    def PythonIncludeDir(self) -> str:
        return self._pythonIncludeDir

    def GetAllEditorResources(self) -> list[str]:
        iconDir = os.path.join(self.BaseDir, "editor", "assets", "icons")
        imagesDir = os.path.join(self.BaseDir, "editor", "assets", "images")
        resources = [os.path.join("assets/icons", file) for file in os.listdir(iconDir)]
        resources += [
            os.path.join("assets/images", file) for file in os.listdir(imagesDir)
        ]
        return resources

    def AllEngineExamples(self) -> list[str]:
        examplesDir = os.path.join(self.BaseDir, "engine", "examples")
        return [file[:-2] for file in os.listdir(examplesDir) if file.endswith(".c")]


SYSTEM = SystemInfo()  # Global constant instance
