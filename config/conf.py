from typing import Dict, Any
from dataclasses import dataclass, field


_VARIABLES: Dict[str, Any] = dict(
    # EMCC="/absolute/path/to/emcc",
    EMCMAKE="/home/threezinedine/Projects/emsdk/upstream/emscripten/emcmake",
)


@dataclass
class Variables:
    EMCC: str | None = field(default=None)
    EMCMAKE: str | None = field(default=None)


VARIABLES = Variables(**_VARIABLES)
