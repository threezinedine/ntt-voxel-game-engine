from dataclasses import dataclass, field


@dataclass
class Template:
    file: str
    outputs: list[str] | None = field(default=None)
    dependencies: list[str] | None = field(default=None)
    extensions: list[str] | None = field(default=None)
