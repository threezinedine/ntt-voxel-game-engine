import os
import sys

ROOT_DIR = os.path.normpath(
    os.path.join(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        "..",
    )
)

print(ROOT_DIR)

sys.path.insert(0, ROOT_DIR)

from config import SystemInfo


SYSTEM = SystemInfo()
TEMPLATE_DATA = dict(
    BASE_DIR=SYSTEM.BaseDir,
    PLATFORM_DEFINE=(
        "PLATFORM_IS_WINDOWS" if SYSTEM.IsWindowsPlatform else "PLATFORM_IS_LINUX"
    ),
    COMPILER_PATH=SYSTEM.CCompiler,
    INTELLISENSE_MODE=SYSTEM.IntelliSenseMode,
    PYTHON_INTERPRETER=SYSTEM.PythonInterpreter,
    PYTHON_INCLUDE_DIR=SYSTEM.PythonIncludeDir,
    RESOURCE_FILES=SYSTEM.GetAllEditorResources(),
)
