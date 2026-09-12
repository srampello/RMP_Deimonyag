import csv
from datetime import datetime
from pathlib import Path
from typing import Dict, Optional, TextIO


class TelemetryLogger:
    FIELDNAMES = [
        "time_ms",
        *[f"sensor_{i}" for i in range(1, 13)],
        "position",
        "error",
        "p",
        "d",
        "pid",
        "motor_left",
        "motor_right",
        "edf",
        "loop_us",
        "line_lost",
        "running",
    ]

    def __init__(self) -> None:
        self._file: Optional[TextIO] = None
        self._writer: Optional[csv.DictWriter] = None
        self.current_path: Optional[Path] = None

    @property
    def is_recording(self) -> bool:
        return self._file is not None

    def start(self) -> Path:
        if self.is_recording:
            return self.current_path  # type: ignore[return-value]

        logs_dir = Path(__file__).resolve().parent / "logs"
        logs_dir.mkdir(parents=True, exist_ok=True)

        timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        self.current_path = logs_dir / f"run_{timestamp}.csv"

        self._file = self.current_path.open("w", newline="", encoding="utf-8")
        self._writer = csv.DictWriter(self._file, fieldnames=self.FIELDNAMES)
        self._writer.writeheader()
        self._file.flush()
        return self.current_path

    def write(self, sample: Dict[str, object]) -> None:
        if not self.is_recording or self._writer is None:
            return

        sensors = list(sample.get("sensors", []))
        row: Dict[str, object] = {
            "time_ms": sample.get("time_ms", ""),
            "position": sample.get("position", ""),
            "error": sample.get("error", ""),
            "p": sample.get("p", ""),
            "d": sample.get("d", ""),
            "pid": sample.get("pid", ""),
            "motor_left": sample.get("motor_left", ""),
            "motor_right": sample.get("motor_right", ""),
            "edf": sample.get("edf", ""),
            "loop_us": sample.get("loop_us", ""),
            "line_lost": sample.get("line_lost", ""),
            "running": sample.get("running", ""),
        }

        for index in range(12):
            row[f"sensor_{index + 1}"] = sensors[index] if index < len(sensors) else ""

        self._writer.writerow(row)
        self._file.flush()  # type: ignore[union-attr]

    def stop(self) -> Optional[Path]:
        if self._file is None:
            return self.current_path

        self._file.close()
        self._file = None
        self._writer = None
        return self.current_path
