import math
import random
import time
from typing import Dict, List

from PySide6.QtCore import QObject, QTimer, Signal


class SimulatorBackend(QObject):
    """Backend simulado para desarrollar la interfaz sin conectar el ESP32."""

    connection_changed = Signal(bool)
    mode_changed = Signal(str)
    sensors_updated = Signal(list)
    telemetry_updated = Signal(dict)
    io_updated = Signal(dict)

    MODE_TEST = "TEST"
    MODE_TELEMETRY = "TELEMETRY"

    def __init__(self, parent=None):
        super().__init__(parent)
        self.connected = False
        self.mode = self.MODE_TEST

        self.motor_left = 0
        self.motor_right = 0
        self.edf = 0
        self.led_on = False

        self._start_time = time.monotonic()
        self._last_error = 0.0

        self._timer = QTimer(self)
        self._timer.setInterval(20)  # 50 Hz
        self._timer.timeout.connect(self._tick)

    def connect_robot(self) -> None:
        if self.connected:
            return
        self.connected = True
        self._start_time = time.monotonic()
        self._last_error = 0.0
        self._timer.start()
        self.connection_changed.emit(True)
        self.io_updated.emit({"button_pressed": False, "led_on": self.led_on})

    def disconnect_robot(self) -> None:
        if not self.connected:
            return
        if self.mode == self.MODE_TEST:
            self.emergency_stop()
        self._timer.stop()
        self.connected = False
        self.connection_changed.emit(False)

    def set_mode(self, mode: str) -> None:
        if mode not in (self.MODE_TEST, self.MODE_TELEMETRY):
            raise ValueError(f"Modo no válido: {mode}")

        if mode == self.MODE_TELEMETRY and self.mode == self.MODE_TEST:
            self.emergency_stop()

        self.mode = mode
        self.mode_changed.emit(mode)

    def set_motor_left(self, value: int) -> None:
        if self.connected and self.mode == self.MODE_TEST:
            self.motor_left = max(-255, min(255, int(value)))

    def set_motor_right(self, value: int) -> None:
        if self.connected and self.mode == self.MODE_TEST:
            self.motor_right = max(-255, min(255, int(value)))

    def set_edf(self, value: int) -> None:
        if self.connected and self.mode == self.MODE_TEST:
            self.edf = max(0, min(100, int(value)))

    def set_led(self, on: bool) -> None:
        if self.connected and self.mode == self.MODE_TEST:
            self.led_on = bool(on)
            self.io_updated.emit({"button_pressed": False, "led_on": self.led_on})

    def emergency_stop(self) -> None:
        if self.mode != self.MODE_TEST:
            return
        self.motor_left = 0
        self.motor_right = 0
        self.edf = 0

    @staticmethod
    def _sensor_values(line_position: float) -> List[int]:
        values = []
        sensor_positions = [i * 1000 for i in range(12)]

        for position in sensor_positions:
            distance = position - line_position
            response = 3500.0 * math.exp(-0.5 * (distance / 700.0) ** 2)
            noise = random.uniform(-60, 60)
            value = int(max(0, min(4095, 250 + response + noise)))
            values.append(value)

        return values

    def _tick(self) -> None:
        elapsed = time.monotonic() - self._start_time

        if self.mode == self.MODE_TEST:
            line_position = 5500 + 3000 * math.sin(elapsed * 0.65)
            sensors = self._sensor_values(line_position)
            self.sensors_updated.emit(sensors)
            return

        line_position = (
            5500
            + 2300 * math.sin(elapsed * 1.15)
            + 650 * math.sin(elapsed * 3.8)
        )
        sensors = self._sensor_values(line_position)

        error = line_position - 5500
        proportional = 0.035 * error
        derivative = 0.16 * (error - self._last_error)
        pid = proportional + derivative
        self._last_error = error

        base_speed = 175
        motor_left = int(max(-255, min(255, base_speed + pid)))
        motor_right = int(max(-255, min(255, base_speed - pid)))

        sample: Dict[str, object] = {
            "time_ms": int(elapsed * 1000),
            "sensors": sensors,
            "position": int(line_position),
            "error": float(error),
            "p": float(proportional),
            "d": float(derivative),
            "pid": float(pid),
            "motor_left": motor_left,
            "motor_right": motor_right,
            "edf": 80,
            "loop_us": 1250 + random.randint(-80, 80),
            "line_lost": False,
            "running": True,
        }

        self.sensors_updated.emit(sensors)
        self.telemetry_updated.emit(sample)
