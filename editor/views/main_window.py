from typing import Any
from PySide6.QtWidgets import QMainWindow
from pyui import Ui_MainWindow


class EditorMainWindow(QMainWindow):
    """
    The entry application main window.
    """

    def __init__(
        self,
        **kwargs: Any,
    ) -> None:
        super().__init__(**kwargs)

        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)  # type: ignore
