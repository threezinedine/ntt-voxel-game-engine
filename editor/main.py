import sys
import resources_rc  # type: ignore
from views import EditorMainWindow
from PySide6.QtWidgets import QApplication


def main():
    app = QApplication(sys.argv)
    mainWindow = EditorMainWindow()
    mainWindow.showMaximized()
    sys.exit(app.exec())


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"An unhandled exception occurred: {e}")
