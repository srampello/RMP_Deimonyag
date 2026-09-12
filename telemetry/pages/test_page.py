import pyqtgraph as pg
from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QGridLayout,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QSlider,
    QVBoxLayout,
    QWidget,
)


class TestPage(QWidget):
    def __init__(self, backend, parent=None):
        super().__init__(parent)
        self.backend = backend

        self.connection_label = QLabel("Desconectado")
        self.connect_button = QPushButton("Conectar")
        self.stop_button = QPushButton("STOP")

        self.motor_left_value = QLabel("0")
        self.motor_right_value = QLabel("0")
        self.edf_value = QLabel("0 %")

        self.motor_left_slider = self._make_slider(-255, 255, 0)
        self.motor_right_slider = self._make_slider(-255, 255, 0)
        self.edf_slider = self._make_slider(0, 100, 0)

        self.sensor_plot = pg.PlotWidget()
        self.sensor_plot.setBackground("w")
        self.sensor_plot.setYRange(0, 4095)
        self.sensor_plot.setXRange(0.25, 12.75)
        self.sensor_plot.setLabel("left", "ADC")
        self.sensor_plot.setLabel("bottom", "Sensor")
        self.sensor_plot.showGrid(x=False, y=True, alpha=0.25)
        self.sensor_bars = pg.BarGraphItem(
            x=list(range(1, 13)),
            height=[0] * 12,
            width=0.72,
        )
        self.sensor_plot.addItem(self.sensor_bars)

        self.sensor_labels = [QLabel(f"S{i}: 0") for i in range(1, 13)]
        for label in self.sensor_labels:
            label.setAlignment(Qt.AlignCenter)

        self._build_ui()
        self._connect_signals()
        self._set_controls_enabled(False)

    @staticmethod
    def _make_slider(minimum: int, maximum: int, value: int) -> QSlider:
        slider = QSlider(Qt.Horizontal)
        slider.setRange(minimum, maximum)
        slider.setValue(value)
        slider.setTracking(True)
        return slider

    def _build_ui(self) -> None:
        root = QVBoxLayout(self)

        header = QHBoxLayout()
        title = QLabel("MODO PRUEBAS")
        title.setObjectName("pageTitle")
        header.addWidget(title)
        header.addStretch()
        header.addWidget(QLabel("ESP32:"))
        header.addWidget(self.connection_label)
        header.addWidget(self.connect_button)
        root.addLayout(header)

        actuators = QGridLayout()

        motor_left_group = QGroupBox("Motor izquierdo")
        motor_left_layout = QVBoxLayout(motor_left_group)
        motor_left_layout.addWidget(self.motor_left_slider)
        motor_left_layout.addWidget(self.motor_left_value)

        motor_right_group = QGroupBox("Motor derecho")
        motor_right_layout = QVBoxLayout(motor_right_group)
        motor_right_layout.addWidget(self.motor_right_slider)
        motor_right_layout.addWidget(self.motor_right_value)

        edf_group = QGroupBox("EDF27")
        edf_layout = QVBoxLayout(edf_group)
        edf_layout.addWidget(self.edf_slider)
        edf_layout.addWidget(self.edf_value)

        self.stop_button.setObjectName("stopButton")
        self.stop_button.setMinimumHeight(70)

        actuators.addWidget(motor_left_group, 0, 0)
        actuators.addWidget(motor_right_group, 0, 1)
        actuators.addWidget(edf_group, 1, 0)
        actuators.addWidget(self.stop_button, 1, 1)
        root.addLayout(actuators)

        sensors_group = QGroupBox("Lectura de sensores en tiempo real")
        sensors_layout = QVBoxLayout(sensors_group)
        sensors_layout.addWidget(self.sensor_plot, stretch=1)

        values_layout = QGridLayout()
        for index, label in enumerate(self.sensor_labels):
            values_layout.addWidget(label, index // 6, index % 6)
        sensors_layout.addLayout(values_layout)

        root.addWidget(sensors_group, stretch=1)

        note = QLabel(
            "Esta página es únicamente para pruebas de banco. "
            "No se utiliza para modificar parámetros durante una carrera."
        )
        note.setWordWrap(True)
        note.setObjectName("noteLabel")
        root.addWidget(note)

    def _connect_signals(self) -> None:
        self.connect_button.clicked.connect(self._toggle_connection)
        self.stop_button.clicked.connect(self._stop_all)

        self.motor_left_slider.valueChanged.connect(self._motor_left_changed)
        self.motor_right_slider.valueChanged.connect(self._motor_right_changed)
        self.edf_slider.valueChanged.connect(self._edf_changed)

        self.backend.connection_changed.connect(self._connection_changed)
        self.backend.sensors_updated.connect(self._sensors_updated)

    def _toggle_connection(self) -> None:
        if self.backend.connected:
            self.backend.disconnect_robot()
        else:
            self.backend.connect_robot()

    def _connection_changed(self, connected: bool) -> None:
        self.connection_label.setText("Conectado" if connected else "Desconectado")
        self.connect_button.setText("Desconectar" if connected else "Conectar")
        self._set_controls_enabled(connected and self.backend.mode == self.backend.MODE_TEST)

        if not connected:
            self._reset_controls()

    def set_test_mode_active(self, active: bool) -> None:
        self._set_controls_enabled(active and self.backend.connected)
        if not active:
            self._reset_controls()

    def _set_controls_enabled(self, enabled: bool) -> None:
        self.motor_left_slider.setEnabled(enabled)
        self.motor_right_slider.setEnabled(enabled)
        self.edf_slider.setEnabled(enabled)
        self.stop_button.setEnabled(enabled)

    def _motor_left_changed(self, value: int) -> None:
        self.motor_left_value.setText(str(value))
        self.backend.set_motor_left(value)

    def _motor_right_changed(self, value: int) -> None:
        self.motor_right_value.setText(str(value))
        self.backend.set_motor_right(value)

    def _edf_changed(self, value: int) -> None:
        self.edf_value.setText(f"{value} %")
        self.backend.set_edf(value)

    def _stop_all(self) -> None:
        self.backend.emergency_stop()
        self._reset_controls()

    def _reset_controls(self) -> None:
        self.motor_left_slider.blockSignals(True)
        self.motor_right_slider.blockSignals(True)
        self.edf_slider.blockSignals(True)

        self.motor_left_slider.setValue(0)
        self.motor_right_slider.setValue(0)
        self.edf_slider.setValue(0)

        self.motor_left_slider.blockSignals(False)
        self.motor_right_slider.blockSignals(False)
        self.edf_slider.blockSignals(False)

        self.motor_left_value.setText("0")
        self.motor_right_value.setText("0")
        self.edf_value.setText("0 %")

    def _sensors_updated(self, values: list) -> None:
        if len(values) < 12:
            return

        self.sensor_bars.setOpts(height=values[:12])
        for index, value in enumerate(values[:12]):
            self.sensor_labels[index].setText(f"S{index + 1}: {value}")
