from dataclasses import dataclass, field


@dataclass
class Binding:
    file: str = field(default="")
    template: str = field(default="")
    output: str = field(default="")
    dependencies: list[str] | None = field(default=None)
    extensions: list[str] | None = field(default=None)
