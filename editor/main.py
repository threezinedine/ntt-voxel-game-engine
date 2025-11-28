import sys
from PyQt6.QtWidgets import QApplication
from views import EditorMainWindow


def main():
    app = QApplication(sys.argv)
    mainWindow = EditorMainWindow()
    mainWindow.showMaximized()
    sys.exit(app.exec())


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        pass
