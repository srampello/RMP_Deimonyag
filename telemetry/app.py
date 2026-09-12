import os
import sys

from PySide6.QtWidgets import QApplication, QMainWindow, QTabWidget

from communication import SimulatorBackend
from communication_real import Esp32Backend
from pages.telemetry_page import TelemetryPage
from pages.test_page import TestPage


APP_STYLE = """
QMainWindow, QWidget {
    background-color: #151515;
    color: #f2f2f2;
    font-size: 13px;
}
QTabWidget::pane {
    border: 1px solid #333333;
}
QTabBar::tab {
    background: #242424;
    color: #dcdcdc;
    padding: 10px 22px;
    margin-right: 2px;
}
QTabBar::tab:selected {
    background: #3b0f0f;
    color: white;
}
QGroupBox {
    border: 1px solid #3a3a3a;
    border-radius: 6px;
    margin-top: 10px;
    padding-top: 12px;
    font-weight: 600;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 5px;
}
QPushButton {
    background-color: #2b2b2b;
    border: 1px solid #555555;
    border-radius: 5px;
    padding: 7px 14px;
}
QPushButton:hover {
    background-color: #383838;
}
QPushButton:disabled {
    color: #777777;
    background-color: #222222;
}
#stopButton {
    background-color: #8b1515;
    color: white;
    font-size: 20px;
    font-weight: 700;
}
#stopButton:hover {
    background-color: #a91b1b;
}
#pageTitle {
    font-size: 20px;
    font-weight: 700;
}
#telemetryValue {
    font-weight: 700;
}
#noteLabel {
    color: #bcbcbc;
}
QSlider::groove:horizontal {
    height: 6px;
    background: #333333;
    border-radius: 3px;
}
QSlider::handle:horizontal {
    background: #b92626;
    width: 16px;
    margin: -5px 0;
    border-radius: 8px;
}
"""


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Deimonyag Control & Telemetry")
        self.resize(1400, 820)

        backend_kind = os.getenv("DEIMONYAG_BACKEND", "esp32").strip().lower()
        ws_url = os.getenv("DEIMONYAG_WS", "ws://192.168.4.1:81")

        if backend_kind == "simulator":
            self.backend = SimulatorBackend(self)
            self.setWindowTitle("Deimonyag Control & Telemetry - SIMULADOR")
        else:
            self.backend = Esp32Backend(ws_url, self)
            self.setWindowTitle("Deimonyag Control & Telemetry - ESP32")

        self.tabs = QTabWidget()
        self.test_page = TestPage(self.backend)
        self.telemetry_page = TelemetryPage(self.backend)

        self.tabs.addTab(self.test_page, "Pruebas")
        self.tabs.addTab(self.telemetry_page, "Telemetría")
        self.tabs.currentChanged.connect(self._tab_changed)

        self.setCentralWidget(self.tabs)
        self._tab_changed(0)

    def _tab_changed(self, index: int) -> None:
        if index == 0:
            self.backend.set_mode(self.backend.MODE_TEST)
            self.test_page.set_test_mode_active(True)
            self.telemetry_page.set_telemetry_mode_active(False)
        else:
            # Antes de TELEMETRY el backend real ordena STOP solo si estaba en TEST.
            # Una vez en TELEMETRY no se envían cambios de actuadores durante carrera.
            self.backend.set_mode(self.backend.MODE_TELEMETRY)
            self.test_page.set_test_mode_active(False)
            self.telemetry_page.set_telemetry_mode_active(True)

    def closeEvent(self, event) -> None:
        self.telemetry_page.stop_recording()
        self.backend.disconnect_robot()
        super().closeEvent(event)


def main() -> int:
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    app.setStyleSheet(APP_STYLE)

    window = MainWindow()
    window.show()

    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
