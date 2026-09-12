import json
import math
import random
import threading
import time
from typing import Dict, List

import websocket
from PySide6.QtCore import QObject, QTimer, Signal


class SimulatorBackend(QObject):
    """Backend simulado para desarrollar la interfaz sin conectar el ESP32."""

    connection_changed = Signal(bool)
    mode_changed = Signal(str)
    sensors_updated = Signal(list)
    telemetry_updated = Signal(dict)

    MODE_TEST = "TEST"
    MODE_TELEMETRY = "TELEMETRY"

    def __init__(self, parent=None):
        super().__init__(parent)
        self.connected = False
        self.mode = self.MODE_TEST

        self.motor_left = 0
        self.motor_right = 0
        self.edf = 0

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

        if mode == self.MODE_TELEMETRY:
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


class Esp32Backend(QObject):
    """Backend WebSocket real para Deimonyag.

    El ESP32 crea el AP DEIMONYAG y escucha por defecto en:
    ws://192.168.4.1:81

    TEST permite comandos de banco. TELEMETRY es solo lectura.
    """

    connection_changed = Signal(bool)
    mode_changed = Signal(str)
    sensors_updated = Signal(list)
    telemetry_updated = Signal(dict)

    MODE_TEST = "TEST"
    MODE_TELEMETRY = "TELEMETRY"

    def __init__(self, url: str = "ws://192.168.4.1:81", parent=None):
        super().__init__(parent)
        self.url = url
        self.connected = False
        self.mode = self.MODE_TEST

        self.motor_left = 0
        self.motor_right = 0
        self.edf = 0

        self._ws = None
        self._thread = None
        self._send_lock = threading.Lock()
        self._closing = False

        # Keepalive de aplicación. Además de mantener la conexión, alimenta
        # el watchdog de 500 ms del modo TEST en el ESP32.
        self._keepalive = QTimer(self)
        self._keepalive.setInterval(200)
        self._keepalive.timeout.connect(self._send_ping)

    def connect_robot(self) -> None:
        if self.connected or (self._thread and self._thread.is_alive()):
            return

        self._closing = False
        self._thread = threading.Thread(target=self._run_socket, daemon=True)
        self._thread.start()

    def disconnect_robot(self) -> None:
        self._closing = True

        # Nunca detener actuadores desde la PC al cerrar durante TELEMETRY.
        if self.connected and self.mode == self.MODE_TEST:
            self.emergency_stop()

        self._keepalive.stop()

        ws = self._ws
        if ws is not None:
            try:
                ws.close()
            except Exception:
                pass

    def set_mode(self, mode: str) -> None:
        if mode not in (self.MODE_TEST, self.MODE_TELEMETRY):
            raise ValueError(f"Modo no válido: {mode}")

        if mode == self.MODE_TELEMETRY and self.mode == self.MODE_TEST:
            self.emergency_stop()

        self.mode = mode
        self.mode_changed.emit(mode)

        if self.connected:
            self._send({"type": "mode", "mode": mode})

    def set_motor_left(self, value: int) -> None:
        if not (self.connected and self.mode == self.MODE_TEST):
            return

        self.motor_left = max(-255, min(255, int(value)))
        self._send({"type": "control", "motor_left": self.motor_left})

    def set_motor_right(self, value: int) -> None:
        if not (self.connected and self.mode == self.MODE_TEST):
            return

        self.motor_right = max(-255, min(255, int(value)))
        self._send({"type": "control", "motor_right": self.motor_right})

    def set_edf(self, value: int) -> None:
        if not (self.connected and self.mode == self.MODE_TEST):
            return

        self.edf = max(0, min(100, int(value)))
        self._send({"type": "control", "edf": self.edf})

    def emergency_stop(self) -> None:
        if self.mode != self.MODE_TEST:
            return

        self.motor_left = 0
        self.motor_right = 0
        self.edf = 0

        if self.connected:
            self._send({"type": "stop"})

    def _run_socket(self) -> None:
        self._ws = websocket.WebSocketApp(
            self.url,
            on_open=self._on_open,
            on_message=self._on_message,
            on_error=self._on_error,
            on_close=self._on_close,
        )

        try:
            self._ws.run_forever(ping_interval=5, ping_timeout=2)
        finally:
            self._ws = None

    def _on_open(self, ws) -> None:
        self.connected = True
        self.connection_changed.emit(True)
        self._keepalive.start()

        self._send({"type": "hello", "protocol": 1})
        self._send({"type": "mode", "mode": self.mode})

    def _on_close(self, ws, close_status_code, close_msg) -> None:
        was_connected = self.connected
        self.connected = False
        self._keepalive.stop()

        if was_connected:
            self.connection_changed.emit(False)

    def _on_error(self, ws, error) -> None:
        # Si falla antes de abrir, igual refrescamos el estado de la UI.
        if self.connected:
            return
        self.connection_changed.emit(False)

    def _on_message(self, ws, message: str) -> None:
        try:
            data = json.loads(message)
        except (TypeError, json.JSONDecodeError):
            return

        message_type = data.get("type")

        if message_type == "hello":
            remote_mode = data.get("mode")
            if remote_mode in (self.MODE_TEST, self.MODE_TELEMETRY):
                self.mode = remote_mode
                self.mode_changed.emit(remote_mode)
            return

        if message_type == "mode":
            remote_mode = data.get("mode")
            if remote_mode in (self.MODE_TEST, self.MODE_TELEMETRY):
                self.mode = remote_mode
                self.mode_changed.emit(remote_mode)
            return

        if message_type == "sensors":
            sensors = data.get("sensors", [])
            if isinstance(sensors, list) and len(sensors) >= 12:
                self.sensors_updated.emit(sensors[:12])
            return

        if message_type == "telemetry":
            sensors = data.get("sensors", [])
            if isinstance(sensors, list) and len(sensors) >= 12:
                self.sensors_updated.emit(sensors[:12])

            sample = {
                "time_ms": int(data.get("time_ms", 0)),
                "sensors": sensors[:12] if isinstance(sensors, list) else [],
                "position": int(data.get("position", 0)),
                "error": float(data.get("error", 0.0)),
                "p": float(data.get("p", 0.0)),
                "d": float(data.get("d", 0.0)),
                "pid": float(data.get("pid", 0.0)),
                "motor_left": int(data.get("motor_left", 0)),
                "motor_right": int(data.get("motor_right", 0)),
                "edf": int(data.get("edf", 0)),
                "loop_us": int(data.get("loop_us", 0)),
                "line_lost": bool(data.get("line_lost", False)),
                "running": bool(data.get("running", False)),
            }
            self.telemetry_updated.emit(sample)
            return

        # pong y error no modifican actuadores ni la telemetría visual.

    def _send_ping(self) -> None:
        if self.connected:
            self._send({"type": "ping"})

    def _send(self, payload: dict) -> None:
        ws = self._ws
        if ws is None or not self.connected:
            return

        message = json.dumps(payload, separators=(",", ":"))

        try:
            with self._send_lock:
                ws.send(message)
        except Exception:
            pass
