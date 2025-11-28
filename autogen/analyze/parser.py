import clang.cindex as cindex  # type: ignore
from .py_struct import PyStruct
from .py_enum import PyEnum
from .py_typedef import PyTypedef
from .py_function import PyFunction


class Parser:
    """
    Extracts the C header files' contents and convert them into self-defined python structures.

    Arguments
    ---------
    content : str | None
        The content of the C header file.
    filePath : str | None
        The path to the C header file.

    Notes
    -----
    If both content and filePath are provided, content will be used.
    If neither is provided, raise an exception.
    """

    def __init__(
        self,
        content: str | None = None,
        filePath: str | None = None,
    ) -> None:
        index = cindex.Index.create()

        if content is not None:
            translationUnit: cindex.TranslationUnit = index.parse(  # type: ignore
                "tmp.h",
                args=["-x", "c", "-std=c17"],
                unsaved_files=[("tmp.h", content)],
                options=0,
            )
        elif filePath is not None:
            translationUnit = index.parse(
                filePath,
                args=["-x", "c", "-std=c17"],
            )
        else:
            raise ValueError("Either content or filePath must be provided.")
        self._cursor: cindex.Cursor = translationUnit.cursor

        self._pyStructs: list[PyStruct] = []
        self._pyEnums: list[PyEnum] = []
        self._pyTypedefs: list[PyTypedef] = []
        self._pyFunctions: list[PyFunction] = []

    def Parse(self) -> None:
        """
        The main method of of the parser whereas it extracts the structures from the C header file content.
        """
        for child in self._cursor.get_children():
            if child.kind == cindex.CursorKind.STRUCT_DECL:
                pyStruct = PyStruct(child)
                self._pyStructs.append(pyStruct)
            elif child.kind == cindex.CursorKind.ENUM_DECL:
                pyEnum = PyEnum(child)
                self._pyEnums.append(pyEnum)
            elif child.kind == cindex.CursorKind.TYPEDEF_DECL:
                pyTypedef = PyTypedef(child)
                self._pyTypedefs.append(pyTypedef)
            elif child.kind == cindex.CursorKind.FUNCTION_DECL:
                pyFunction = PyFunction(child)
                self._pyFunctions.append(pyFunction)

    @property
    def Structs(self) -> list[PyStruct]:
        """
        Returns the parsed structures.

        Returns
        -------
        list[PyStruct]
            The list of parsed structures.
        """
        return self._pyStructs

    @property
    def Enums(self) -> list[PyEnum]:
        """
        Returns the parsed enumerations.

        Returns
        -------
        list
            The list of parsed enumerations.
        """
        return self._pyEnums

    @property
    def Typedefs(self) -> list[PyTypedef]:
        """
        Returns the parsed typedefs.

        Returns
        -------
        list
            The list of parsed typedefs.
        """
        return self._pyTypedefs

    @property
    def Functions(self) -> list[PyFunction]:
        """
        Returns the parsed functions.

        Returns
        -------
        list
            The list of parsed functions.
        """
        return self._pyFunctions

    @property
    def AllCustomTypes(self) -> list[str]:
        """
        Returns all custom types defined in the parsed C header file.

        Returns
        -------
        list[str]
            The list of all custom types.
        """
        customTypes: list[str] = []
        for struct in self._pyStructs:
            if "binding" in struct.annotations:
                customTypes.append(struct.name)
        for enum in self._pyEnums:
            if "binding" in enum.annotations:
                customTypes.append(enum.name)
        for typedef in self._pyTypedefs:
            if "binding" in typedef.annotations:
                customTypes.append(typedef.name)
        return customTypes
