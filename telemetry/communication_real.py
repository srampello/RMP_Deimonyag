import json
import threading

import websocket
from PySide6.QtCore import QObject, Signal


class Esp32Backend(QObject):
    connection_changed = Signal(bool)
    mode_changed = Signal(str)
    sensors_updated = Signal(list)
    telemetry_updated = Signal(dict)

    MODE_TEST = "TEST"
    MODE_TELEMETRY = "TELEMETRY"

    def __init__(self, url="ws://192.168.4.1:81", parent=None):
        super().__init__(parent)
        self.url = url
        self.connected = False
        self.mode = self.MODE_TEST
        self.motor_left = 0
        self.motor_right = 0
        self.edf = 0
        self._ws = None
        self._thread = None
        self._keepalive_thread = None
        self._keepalive_stop = threading.Event()
        self._send_lock = threading.Lock()

    def connect_robot(self):
        if self.connected or (self._thread and self._thread.is_alive()):
            return
        self._keepalive_stop.clear()
        self._thread = threading.Thread(target=self._run_socket, daemon=True)
        self._thread.start()

    def disconnect_robot(self):
        if self.connected and self.mode == self.MODE_TEST:
            self.emergency_stop()
        self._keepalive_stop.set()
        if self._ws is not None:
            try:
                self._ws.close()
            except Exception:
                pass

    def set_mode(self, mode):
        if mode not in (self.MODE_TEST, self.MODE_TELEMETRY):
            raise ValueError(f"Modo no válido: {mode}")
        if mode == self.MODE_TELEMETRY and self.mode == self.MODE_TEST:
            self.emergency_stop()
        self.mode = mode
        self.mode_changed.emit(mode)
        if self.connected:
            self._send({"type": "mode", "mode": mode})

    def set_motor_left(self, value):
        if self.connected and self.mode == self.MODE_TEST:
            self.motor_left = max(-255, min(255, int(value)))
            self._send({"type": "control", "motor_left": self.motor_left})

    def set_motor_right(self, value):
        if self.connected and self.mode == self.MODE_TEST:
            self.motor_right = max(-255, min(255, int(value)))
            self._send({"type": "control", "motor_right": self.motor_right})

    def set_edf(self, value):
        if self.connected and self.mode == self.MODE_TEST:
            self.edf = max(0, min(100, int(value)))
            self._send({"type": "control", "edf": self.edf})

    def emergency_stop(self):
        if self.mode != self.MODE_TEST:
            return
        self.motor_left = 0
        self.motor_right = 0
        self.edf = 0
        if self.connected:
            self._send({"type": "stop"})

    def _run_socket(self):
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

    def _on_open(self, ws):
        self.connected = True
        self.connection_changed.emit(True)
        self._start_keepalive()
        self._send({"type": "hello", "protocol": 1})
        self._send({"type": "mode", "mode": self.mode})

    def _on_close(self, ws, close_status_code, close_msg):
        was_connected = self.connected
        self.connected = False
        self._keepalive_stop.set()
        if was_connected:
            self.connection_changed.emit(False)

    def _on_error(self, ws, error):
        if not self.connected:
            self.connection_changed.emit(False)

    def _start_keepalive(self):
        if self._keepalive_thread and self._keepalive_thread.is_alive():
            return
        self._keepalive_stop.clear()
        self._keepalive_thread = threading.Thread(target=self._keepalive_loop, daemon=True)
        self._keepalive_thread.start()

    def _keepalive_loop(self):
        while not self._keepalive_stop.wait(0.2):
            if self.connected:
                self._send({"type": "ping"})

    def _on_message(self, ws, message):
        try:
            data = json.loads(message)
        except (TypeError, json.JSONDecodeError):
            return

        kind = data.get("type")

        if kind in ("hello", "mode"):
            remote_mode = data.get("mode")
            if remote_mode in (self.MODE_TEST, self.MODE_TELEMETRY):
                self.mode = remote_mode
                self.mode_changed.emit(remote_mode)
            return

        if kind == "sensors":
            sensors = data.get("sensors", [])
            if isinstance(sensors, list) and len(sensors) >= 12:
                self.sensors_updated.emit(sensors[:12])
            return

        if kind == "telemetry":
            sensors = data.get("sensors", [])
            if isinstance(sensors, list) and len(sensors) >= 12:
                self.sensors_updated.emit(sensors[:12])
            self.telemetry_updated.emit({
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
            })

    def _send(self, payload):
        if self._ws is None or not self.connected:
            return
        message = json.dumps(payload, separators=(",", ":"))
        try:
            with self._send_lock:
                self._ws.send(message)
        except Exception:
            pass
