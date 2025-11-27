import os


class SystemInfo:
    def __init__(self) -> None:
        self._compilerPath = ""
        self._intelliSenseMode = ""

        if self.IsWindowsPlatform:
            self._compilerPath = "C:/Program Files/LLVM/bin/clang.exe"
            self._intelliSenseMode = "windows-clang-x64"

        elif self.IsLinuxPlatform:
            self._compilerPath = "/usr/bin/clang"
            self._intelliSenseMode = "linux-clang-x64"

    @property
    def BASE_DIR(self) -> str:
        return os.path.dirname(
            os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        )

    @property
    def IsWindowsPlatform(self) -> bool:
        return os.name == "nt"

    @property
    def IsLinuxPlatform(self) -> bool:
        return os.name == "posix"

    @property
    def CCompiler(self) -> str:
        return self._compilerPath

    @property
    def IntelliSenseMode(self) -> str:
        return self._intelliSenseMode


SYSTEM = SystemInfo()
TEMPLATE_DATA = dict(
    BASE_DIR=SYSTEM.BASE_DIR,
    PLATFORM_DEFINE=(
        "PLATFORM_IS_WINDOWS" if SYSTEM.IsWindowsPlatform else "PLATFORM_IS_LINUX"
    ),
    COMPILER_PATH=SYSTEM.CCompiler,
    INTELLISENSE_MODE=SYSTEM.IntelliSenseMode,
)
