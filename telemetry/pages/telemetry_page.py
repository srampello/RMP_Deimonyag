from collections import deque

import pyqtgraph as pg
from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QGridLayout,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from logger import TelemetryLogger


class TelemetryPage(QWidget):
    HISTORY_POINTS = 600  # 12 s a 50 Hz

    def __init__(self, backend, parent=None):
        super().__init__(parent)
        self.backend = backend
        self.logger = TelemetryLogger()

        self.times = deque(maxlen=self.HISTORY_POINTS)
        self.errors = deque(maxlen=self.HISTORY_POINTS)
        self.p_values = deque(maxlen=self.HISTORY_POINTS)
        self.d_values = deque(maxlen=self.HISTORY_POINTS)
        self.pid_values = deque(maxlen=self.HISTORY_POINTS)

        self.connection_label = QLabel("Desconectado")
        self.connect_button = QPushButton("Conectar")
        self.record_button = QPushButton("Iniciar grabación")
        self.stop_record_button = QPushButton("Detener grabación")
        self.stop_record_button.setEnabled(False)
        self.recording_label = QLabel("Grabación detenida")
        self.file_label = QLabel("Sin archivo")

        self.value_labels = {
            "position": QLabel("0"),
            "error": QLabel("0.0"),
            "p": QLabel("0.0"),
            "d": QLabel("0.0"),
            "pid": QLabel("0.0"),
            "motor_left": QLabel("0"),
            "motor_right": QLabel("0"),
            "edf": QLabel("0 %"),
            "loop_us": QLabel("0 us"),
            "running": QLabel("NO"),
            "line_lost": QLabel("NO"),
        }

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

        self.pid_plot = pg.PlotWidget()
        self.pid_plot.setBackground("w")
        self.pid_plot.setLabel("left", "Valor")
        self.pid_plot.setLabel("bottom", "Tiempo", units="s")
        self.pid_plot.showGrid(x=True, y=True, alpha=0.25)
        self.pid_plot.addLegend()

        self.error_curve = self.pid_plot.plot(name="Error", pen=pg.mkPen("#1f77b4", width=2))
        self.p_curve = self.pid_plot.plot(name="P", pen=pg.mkPen("#2ca02c", width=2))
        self.d_curve = self.pid_plot.plot(name="D", pen=pg.mkPen("#ff7f0e", width=2))
        self.pid_curve = self.pid_plot.plot(name="PID", pen=pg.mkPen("#d62728", width=2))

        self._build_ui()
        self._connect_signals()

    def _build_ui(self) -> None:
        root = QVBoxLayout(self)

        header = QHBoxLayout()
        title = QLabel("TELEMETRÍA DE CARRERA")
        title.setObjectName("pageTitle")
        header.addWidget(title)
        header.addStretch()
        header.addWidget(QLabel("ESP32:"))
        header.addWidget(self.connection_label)
        header.addWidget(self.connect_button)
        root.addLayout(header)

        recording = QHBoxLayout()
        recording.addWidget(self.record_button)
        recording.addWidget(self.stop_record_button)
        recording.addWidget(self.recording_label)
        recording.addStretch()
        recording.addWidget(QLabel("Archivo:"))
        recording.addWidget(self.file_label)
        root.addLayout(recording)

        values_group = QGroupBox("Estado instantáneo")
        values_layout = QGridLayout(values_group)
        items = [
            ("Posición", "position"),
            ("Error", "error"),
            ("P", "p"),
            ("D", "d"),
            ("PID", "pid"),
            ("Motor L", "motor_left"),
            ("Motor R", "motor_right"),
            ("EDF", "edf"),
            ("Loop", "loop_us"),
            ("En carrera", "running"),
            ("Línea perdida", "line_lost"),
        ]
        for index, (title, key) in enumerate(items):
            row = index // 6
            column = (index % 6) * 2
            values_layout.addWidget(QLabel(f"{title}:"), row, column)
            value_label = self.value_labels[key]
            value_label.setAlignment(Qt.AlignLeft | Qt.AlignVCenter)
            value_label.setObjectName("telemetryValue")
            values_layout.addWidget(value_label, row, column + 1)
        root.addWidget(values_group)

        graphs = QHBoxLayout()

        sensor_group = QGroupBox("12 sensores")
        sensor_layout = QVBoxLayout(sensor_group)
        sensor_layout.addWidget(self.sensor_plot)

        pid_group = QGroupBox("PID en el tiempo")
        pid_layout = QVBoxLayout(pid_group)
        pid_layout.addWidget(self.pid_plot)

        graphs.addWidget(sensor_group, 1)
        graphs.addWidget(pid_group, 2)
        root.addLayout(graphs, stretch=1)

        note = QLabel(
            "Modo solo lectura: durante una carrera la PC recibe y registra datos. "
            "No se envían cambios de motores, EDF ni parámetros PID."
        )
        note.setWordWrap(True)
        note.setObjectName("noteLabel")
        root.addWidget(note)

    def _connect_signals(self) -> None:
        self.connect_button.clicked.connect(self._toggle_connection)
        self.record_button.clicked.connect(self.start_recording)
        self.stop_record_button.clicked.connect(self.stop_recording)

        self.backend.connection_changed.connect(self._connection_changed)
        self.backend.telemetry_updated.connect(self._telemetry_updated)

    def _toggle_connection(self) -> None:
        if self.backend.connected:
            self.backend.disconnect_robot()
        else:
            self.backend.connect_robot()

    def _connection_changed(self, connected: bool) -> None:
        self.connection_label.setText("Conectado" if connected else "Desconectado")
        self.connect_button.setText("Desconectar" if connected else "Conectar")
        self.record_button.setEnabled(connected and not self.logger.is_recording)

        if not connected:
            self.stop_recording()

    def set_telemetry_mode_active(self, active: bool) -> None:
        self.record_button.setEnabled(active and self.backend.connected and not self.logger.is_recording)
        if not active:
            self.stop_recording()

    def start_recording(self) -> None:
        if not self.backend.connected or self.logger.is_recording:
            return

        path = self.logger.start()
        self.recording_label.setText("● REC")
        self.file_label.setText(path.name)
        self.record_button.setEnabled(False)
        self.stop_record_button.setEnabled(True)

    def stop_recording(self) -> None:
        if not self.logger.is_recording:
            self.stop_record_button.setEnabled(False)
            return

        path = self.logger.stop()
        self.recording_label.setText("Grabación detenida")
        if path is not None:
            self.file_label.setText(path.name)
        self.record_button.setEnabled(self.backend.connected and self.backend.mode == self.backend.MODE_TELEMETRY)
        self.stop_record_button.setEnabled(False)

    def _telemetry_updated(self, sample: dict) -> None:
        sensors = list(sample.get("sensors", []))
        if len(sensors) >= 12:
            self.sensor_bars.setOpts(height=sensors[:12])

        time_s = float(sample.get("time_ms", 0)) / 1000.0
        self.times.append(time_s)
        self.errors.append(float(sample.get("error", 0.0)))
        self.p_values.append(float(sample.get("p", 0.0)))
        self.d_values.append(float(sample.get("d", 0.0)))
        self.pid_values.append(float(sample.get("pid", 0.0)))

        x = list(self.times)
        self.error_curve.setData(x, list(self.errors))
        self.p_curve.setData(x, list(self.p_values))
        self.d_curve.setData(x, list(self.d_values))
        self.pid_curve.setData(x, list(self.pid_values))

        self.value_labels["position"].setText(str(sample.get("position", 0)))
        self.value_labels["error"].setText(f"{float(sample.get('error', 0.0)):.1f}")
        self.value_labels["p"].setText(f"{float(sample.get('p', 0.0)):.2f}")
        self.value_labels["d"].setText(f"{float(sample.get('d', 0.0)):.2f}")
        self.value_labels["pid"].setText(f"{float(sample.get('pid', 0.0)):.2f}")
        self.value_labels["motor_left"].setText(str(sample.get("motor_left", 0)))
        self.value_labels["motor_right"].setText(str(sample.get("motor_right", 0)))
        self.value_labels["edf"].setText(f"{sample.get('edf', 0)} %")
        self.value_labels["loop_us"].setText(f"{sample.get('loop_us', 0)} us")
        self.value_labels["running"].setText("SÍ" if sample.get("running", False) else "NO")
        self.value_labels["line_lost"].setText("SÍ" if sample.get("line_lost", False) else "NO")

        if self.logger.is_recording:
            self.logger.write(sample)
