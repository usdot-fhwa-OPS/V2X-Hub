#!/usr/bin/python3
import datetime
import sys
import json
import threading
import socket

import j2735_202409
from PyQt6.QtCore import Qt, QTimer
from PyQt6.QtWidgets import (
    QApplication,
    QComboBox,
    QDialog,
    QDialogButtonBox,
    QDoubleSpinBox,
    QFormLayout,
    QGroupBox,
    QHBoxLayout,
    QHeaderView,
    QLabel,
    QLineEdit,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QSpinBox,
    QTableWidget,
    QTableWidgetItem,
    QVBoxLayout,
    QWidget,
)

# Global send state
msgCnt = 0
msgCnt_lock = threading.Lock()
_send_sock = None
_send_sock_lock = threading.Lock()
_send_dest = None

# Column indices for the SRM entry table
COL_INTERSECTION = 0
COL_LANE = 1
COL_DISTANCE = 2
COL_CLEARANCE = 3
COL_SPEED = 4
COL_ENTITY = 5
COL_ROLE = 6
COL_REQUEST_TYPE = 7
COLUMN_LABELS = [
    "Intersection ID",
    "Inbound Lane",
    "Distance to Intersection (m)",
    "Clearance Distance (m)",
    "Speed (m/s)",
    "Entity ID",
    "Role",
    "Request Type",
]

# J2735 Velocity_t units: 0.02 m/s
J2735_SPEED = 0.02

# Guard speed to avoid division by zero
MIN_SPEED = 0.01

ROLE_CHOICES = [
    "basicVehicle",
    "publicTransport",
    "specialTransport",
    "dangerousGoods",
    "roadWork",
    "roadRescue",
    "emergency",
    "safetyCar",
    "none-unknown",
    "truck",
    "motorcycle",
    "roadSideSource",
    "police",
    "fire",
    "ambulance",
    "dot",
    "transit",
    "slowMoving",
    "stopNgo",
    "cyclist",
    "pedestrian",
    "nonMotorized",
    "military",
]

REQUEST_TYPE_CHOICES = [
    "priorityRequest",
    "priorityRequestUpdate",
    "priorityCancellation"
]


def getMOY() -> int:
    """Minute-of-year for the current UTC time."""
    now = datetime.datetime.now(tz=datetime.timezone.utc)
    start_of_year = datetime.datetime(now.year, 1, 1, tzinfo=datetime.timezone.utc)
    delta = now - start_of_year
    return delta.days * 1440 + now.hour * 60 + now.minute


def getDSecond() -> int:
    """Millisecond within the current UTC minute."""
    now = datetime.datetime.now(tz=datetime.timezone.utc)
    return now.second * 1000 + now.microsecond // 1000


def offsetToMOYAndDSecond(offset_seconds: int) -> tuple[int, int]:
    """Convert a seconds-from-now offset into (MOY, DSecond) pair."""
    now = datetime.datetime.now(tz=datetime.timezone.utc)
    target = now + datetime.timedelta(seconds=offset_seconds)
    start_of_year = datetime.datetime(target.year, 1, 1, tzinfo=datetime.timezone.utc)
    delta = target - start_of_year
    moy = delta.days * 1440 + target.hour * 60 + target.minute
    dsecond = target.second * 1000 + target.microsecond // 1000
    return moy, dsecond

def buildSRM(entries: list[dict]) -> str:
    """Build an SRM JSON string from a list of entry dicts.

    Each entry dict has keys:
        intersection_id, inbound_lane, distance_m, clearance_m, speed_mps,
        entity_id, role, request_type

    ETA is derived from distance_m / speed_mps and EDT (duration in ms) from
    clearance_m / speed_mps. A zero distance means the vehicle is at the stop
    bar (ETA = now); a zero clearance means the vehicle has cleared.
    """
    global msgCnt
    with msgCnt_lock:
        cnt = msgCnt
        msgCnt = (msgCnt + 1) & 0x7F

    requests = []
    for entry in entries:
        speed = max(float(entry["speed_mps"]), MIN_SPEED)
        eta_s = max(round(float(entry["distance_m"]) / speed), 0)
        edt_s = max(round(float(entry["clearance_m"]) / speed), 0)

        eta_moy, eta_ds = offsetToMOYAndDSecond(eta_s)

        requests.append(
            {
                "request": {
                    "id": {"id": entry["intersection_id"]},
                    "requestID": 1,
                    "requestType": entry["request_type"],
                    "inBoundLane": {"lane": entry["inbound_lane"]},
                },
                "minute": eta_moy,
                "second": eta_ds,
                "duration": edt_s * 1000,
            }
        )

    # All entries in this call share the same entity; use the first for requestor fields
    first = entries[0]
    entity_id = first["entity_id"]
    role = first["role"]
    speed_field = max(round(float(first["speed_mps"]) / J2735_SPEED), 0)

    srm = {
        "messageId": 29,
        "value": {
            "timeStamp": getMOY(),
            "second": getDSecond(),
            "sequenceNumber": cnt,
            "requests": requests,
            "requestor": {
                "id": {"entityID": entity_id},
                "type": {"role": role},
                "position": {
                    "position": {
                        "lat": 389562674,
                        "long": -771505027,
                        "elevation": 40,
                    },
                    "heading": 0,
                    "speed": {"transmisson": "unavailable", "speed": speed_field},
                },
            },
        },
    }

    return json.dumps(srm)


def sendMessage(msg: bytes, ip_send: str, port_send: int) -> None:
    """Send a UDP datagram, reusing a cached socket."""
    global _send_sock, _send_dest
    try:
        with _send_sock_lock:
            if _send_dest != (ip_send, port_send) or _send_sock is None:
                if _send_sock:
                    _send_sock.close()
                _send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                _send_sock.connect((ip_send, port_send))
                _send_dest = (ip_send, port_send)
            sock = _send_sock
        sock.send(msg)
    except Exception as e:
        raise RuntimeError(f"UDP send failed: {e}") from e


class AddEntryDialog(QDialog):
    """Dialog for adding a new SRM entry with editable fields."""

    def __init__(self, parent=None, initial: dict | None = None):
        super().__init__(parent)
        defaults = initial or {}
        self.setWindowTitle("Edit SRM Entry" if initial else "Add SRM Entry")
        self.setMinimumWidth(380)

        layout = QFormLayout(self)

        self.intersection_id = QSpinBox()
        self.intersection_id.setRange(0, 65535)
        self.intersection_id.setValue(defaults.get("intersection_id", 9709))

        self.inbound_lane = QSpinBox()
        self.inbound_lane.setRange(0, 255)
        self.inbound_lane.setValue(defaults.get("inbound_lane", 1))

        self.distance = QDoubleSpinBox()
        self.distance.setRange(0.0, 100000.0)
        self.distance.setDecimals(1)
        self.distance.setSingleStep(1.0)
        self.distance.setValue(float(defaults.get("distance_m", 100)))
        self.distance.setSuffix(" m")

        self.clearance = QDoubleSpinBox()
        self.clearance.setRange(0.0, 10000.0)
        self.clearance.setDecimals(1)
        self.clearance.setSingleStep(1.0)
        self.clearance.setValue(float(defaults.get("clearance_m", 30)))
        self.clearance.setSuffix(" m")

        self.speed = QDoubleSpinBox()
        self.speed.setRange(0.1, 120.0)
        self.speed.setDecimals(2)
        self.speed.setSingleStep(0.5)
        self.speed.setValue(defaults.get("speed_mps", 10.0))
        self.speed.setSuffix(" m/s")

        self.entity_id = QLineEdit(defaults.get("entity_id", "12345678"))

        self.role = QComboBox()
        self.role.addItems(ROLE_CHOICES)
        self.role.setCurrentText(defaults.get("role", "transit"))

        self.request_type = QComboBox()
        self.request_type.addItems(REQUEST_TYPE_CHOICES)
        self.request_type.setCurrentText(defaults.get("request_type", "Request"))

        layout.addRow("Intersection ID:", self.intersection_id)
        layout.addRow("Inbound Lane:", self.inbound_lane)
        layout.addRow("Distance to Intersection:", self.distance)
        layout.addRow("Clearance Distance:", self.clearance)
        layout.addRow("Speed:", self.speed)
        layout.addRow("Entity ID:", self.entity_id)
        layout.addRow("Role:", self.role)
        layout.addRow("Request Type:", self.request_type)
        buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
        )
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addRow(buttons)

    def values(self) -> dict:
        return {
            "intersection_id": self.intersection_id.value(),
            "inbound_lane": self.inbound_lane.value(),
            "distance_m": self.distance.value(),
            "clearance_m": self.clearance.value(),
            "speed_mps": self.speed.value(),
            "entity_id": self.entity_id.text(),
            "role": self.role.currentText(),
            "request_type": self.request_type.currentText(),
        }


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Mock SRM Sender")
        self.resize(900, 600)

        central = QWidget()
        self.setCentralWidget(central)
        root_layout = QVBoxLayout(central)

        # Connection settings
        conn_group = QGroupBox("Connection")
        conn_layout = QHBoxLayout(conn_group)

        conn_layout.addWidget(QLabel("IP Address:"))
        self.ip_input = QLineEdit("127.0.0.1")
        self.ip_input.setFixedWidth(160)
        conn_layout.addWidget(self.ip_input)

        conn_layout.addWidget(QLabel("Port:"))
        self.port_input = QSpinBox()
        self.port_input.setRange(1, 65535)
        self.port_input.setValue(26789)
        conn_layout.addWidget(self.port_input)

        conn_layout.addStretch()
        root_layout.addWidget(conn_group)

        # SRM entries table
        entries_group = QGroupBox("SRM Entries")
        entries_layout = QVBoxLayout(entries_group)

        self.table = QTableWidget(0, len(COLUMN_LABELS))
        self.table.setHorizontalHeaderLabels(COLUMN_LABELS)
        self.table.setSelectionBehavior(
            QTableWidget.SelectionBehavior.SelectRows
        )
        self.table.horizontalHeader().setSectionResizeMode(
            QHeaderView.ResizeMode.Stretch
        )
        self.table.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers)
        entries_layout.addWidget(self.table)

        btn_layout = QHBoxLayout()
        self.add_btn = QPushButton("Add Entry")
        self.edit_btn = QPushButton("Edit Selected")
        self.remove_btn = QPushButton("Remove Selected")
        btn_layout.addWidget(self.add_btn)
        btn_layout.addWidget(self.edit_btn)
        btn_layout.addWidget(self.remove_btn)
        btn_layout.addStretch()
        entries_layout.addLayout(btn_layout)

        root_layout.addWidget(entries_group)

        # Send buttons
        send_btn_layout = QHBoxLayout()
        self.send_btn = QPushButton("Send SRM")
        self.send_btn.setFixedHeight(36)
        send_btn_layout.addWidget(self.send_btn)
        self.simulate_btn = QPushButton("Simulate Approach")
        self.simulate_btn.setFixedHeight(36)
        self.simulate_btn.setToolTip(
            "Send an initial priorityRequest, then priorityRequestUpdate messages "
            "at 1 Hz. Each tick advances the vehicle by speed_mps * 1 s, "
            "consuming distance-to-intersection first and then clearance. "
            "Stops when the intersection is cleared."
        )
        send_btn_layout.addWidget(self.simulate_btn)
        root_layout.addLayout(send_btn_layout)

        self.status_label = QLabel("")
        self.status_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        root_layout.addWidget(self.status_label)

        # Approach-simulation state
        self._sim_timer = QTimer(self)
        self._sim_timer.setInterval(1000)
        self._sim_timer.timeout.connect(self._on_simulate_tick)
        self._sim_entries: list[dict] = []
        self._sim_target: tuple[str, int] = ("", 0)

        # Signals
        self.add_btn.clicked.connect(self._on_add)
        self.edit_btn.clicked.connect(self._on_edit)
        self.remove_btn.clicked.connect(self._on_remove)
        self.send_btn.clicked.connect(self._on_send)
        self.simulate_btn.clicked.connect(self._on_simulate)
        self.table.doubleClicked.connect(lambda idx: self._edit_row(idx.row()))

    # Slots
    def _set_row(self, row: int, vals: dict) -> None:
        """Write an entry dict into the given table row."""
        self.table.setItem(row, COL_INTERSECTION, QTableWidgetItem(str(vals["intersection_id"])))
        self.table.setItem(row, COL_LANE, QTableWidgetItem(str(vals["inbound_lane"])))
        self.table.setItem(row, COL_DISTANCE, QTableWidgetItem(f"{vals['distance_m']:.1f}"))
        self.table.setItem(row, COL_CLEARANCE, QTableWidgetItem(f"{vals['clearance_m']:.1f}"))
        self.table.setItem(row, COL_SPEED, QTableWidgetItem(f"{vals['speed_mps']:.2f}"))
        self.table.setItem(row, COL_ENTITY, QTableWidgetItem(vals["entity_id"]))
        self.table.setItem(row, COL_ROLE, QTableWidgetItem(vals["role"]))
        self.table.setItem(row, COL_REQUEST_TYPE, QTableWidgetItem(vals["request_type"]))

    def _read_row(self, row: int) -> dict:
        """Read a single table row back into an entry dict."""
        return {
            "intersection_id": int(self.table.item(row, COL_INTERSECTION).text()),
            "inbound_lane": int(self.table.item(row, COL_LANE).text()),
            "distance_m": float(self.table.item(row, COL_DISTANCE).text()),
            "clearance_m": float(self.table.item(row, COL_CLEARANCE).text()),
            "speed_mps": float(self.table.item(row, COL_SPEED).text()),
            "entity_id": self.table.item(row, COL_ENTITY).text(),
            "role": self.table.item(row, COL_ROLE).text(),
            "request_type": self.table.item(row, COL_REQUEST_TYPE).text(),
        }

    def _on_add(self):
        dlg = AddEntryDialog(self)
        if dlg.exec() != QDialog.DialogCode.Accepted:
            return
        row = self.table.rowCount()
        self.table.insertRow(row)
        self._set_row(row, dlg.values())

    def _edit_row(self, row: int) -> None:
        """Open the edit dialog for a specific table row."""
        if row < 0 or row >= self.table.rowCount():
            return
        current = self._read_row(row)
        dlg = AddEntryDialog(self, initial=current)
        if dlg.exec() != QDialog.DialogCode.Accepted:
            return
        self._set_row(row, dlg.values())

    def _on_edit(self):
        rows = sorted({idx.row() for idx in self.table.selectedIndexes()})
        if not rows:
            QMessageBox.information(self, "No selection", "Select a row to edit.")
            return
        self._edit_row(rows[0])

    def _on_remove(self):
        rows = sorted(
            {idx.row() for idx in self.table.selectedIndexes()}, reverse=True
        )
        for row in rows:
            self.table.removeRow(row)

    def _read_entries(self) -> list[dict]:
        """Read every table row back into a list of entry dicts."""
        return [self._read_row(row) for row in range(self.table.rowCount())]

    def _send_entries(self, entries: list[dict]) -> tuple[int, int]:
        """Encode and UDP-send the given entries, grouped by entity.

        Returns (srm_count, total_bytes). Raises on send/encode failure.
        """
        ip = self.ip_input.text().strip()
        port = self.port_input.value()

        # Group entries by Entity ID so each entity gets its own SRM
        groups: dict[str, list[dict]] = {}
        for entry in entries:
            groups.setdefault(entry["entity_id"], []).append(entry)

        frame = j2735_202409.MessageFrame.MessageFrame
        total_bytes = 0
        for _, group_entries in groups.items():
            srm_str = buildSRM(group_entries)
            frame.from_jer(srm_str)
            uper = frame.to_uper()
            sendMessage(uper, ip, port)
            total_bytes += len(uper)
        return len(groups), total_bytes

    def _on_send(self):
        if self.table.rowCount() == 0:
            QMessageBox.warning(self, "No entries", "Add at least one SRM entry before sending.")
            return

        entries = self._read_entries()
        ip = self.ip_input.text().strip()
        port = self.port_input.value()
        try:
            count, total_bytes = self._send_entries(entries)
            self.status_label.setText(
                f"Sent {count} SRM(s) ({total_bytes} bytes total) to {ip}:{port}"
            )
        except Exception as exc:
            QMessageBox.critical(self, "Send Error", str(exc))
            self.status_label.setText("Send failed.")

    def _on_simulate(self):
        """Start (or cancel) a 1 Hz approach simulation.

        Sends a single priorityRequest with the table's entries, then fires
        priorityRequestUpdate messages every second. On each tick the vehicle
        advances by (speed_mps * 1 s), consuming distance_m first, then
        clearance_m. The simulation stops once every entry has cleared the
        intersection (distance_m == 0 and clearance_m == 0).
        """
        if self._sim_timer.isActive():
            self._stop_simulation("Simulation cancelled.")
            return

        if self.table.rowCount() == 0:
            QMessageBox.warning(self, "No entries", "Add at least one SRM entry before simulating.")
            return

        entries = self._read_entries()
        if all(entry["distance_m"] + entry["clearance_m"] <= 0 for entry in entries):
            QMessageBox.warning(
                self, "Already cleared",
                "At least one entry needs distance + clearance > 0 m for the approach to run."
            )
            return

        # Force the first message to be a priorityRequest; subsequent ticks use updates.
        for entry in entries:
            entry["request_type"] = "priorityRequest"

        ip = self.ip_input.text().strip()
        port = self.port_input.value()
        try:
            count, total_bytes = self._send_entries(entries)
        except Exception as exc:
            QMessageBox.critical(self, "Send Error", str(exc))
            self.status_label.setText("Send failed.")
            return

        self._sim_entries = entries
        self._sim_target = (ip, port)
        self._set_controls_enabled(False)
        self.simulate_btn.setText("Cancel Simulation")
        self.simulate_btn.setEnabled(True)
        self.status_label.setText(
            f"Simulating approach to {ip}:{port} — initial priorityRequest sent "
            f"({count} SRM(s), {total_bytes} bytes)."
        )
        self._sim_timer.start()

    def _advance_entry(self, entry: dict, dt_s: float) -> None:
        """Advance one entry by dt_s seconds at its configured speed."""
        dv = max(float(entry["speed_mps"]), MIN_SPEED) * dt_s
        distance = float(entry["distance_m"])
        clearance = float(entry["clearance_m"])
        if distance > 0:
            if dv >= distance:
                clearance = max(clearance - (dv - distance), 0.0)
                distance = 0.0
            else:
                distance -= dv
        else:
            clearance = max(clearance - dv, 0.0)
        entry["distance_m"] = distance
        entry["clearance_m"] = clearance

    def _on_simulate_tick(self):
        """Advance each simulated entry by one tick and broadcast updates."""
        dt_s = self._sim_timer.interval() / 1000.0
        for entry in self._sim_entries:
            self._advance_entry(entry, dt_s)
            entry["request_type"] = "priorityRequestUpdate"

        ip, port = self._sim_target
        try:
            count, total_bytes = self._send_entries(self._sim_entries)
        except Exception as exc:
            QMessageBox.critical(self, "Send Error", str(exc))
            self._stop_simulation("Send failed; simulation stopped.")
            return

        max_distance = max((entry["distance_m"] for entry in self._sim_entries), default=0.0)
        max_clearance = max((entry["clearance_m"] for entry in self._sim_entries), default=0.0)
        self.status_label.setText(
            f"Simulating {ip}:{port} — priorityRequestUpdate sent "
            f"({count} SRM(s), {total_bytes} bytes, dist={max_distance:.1f} m, "
            f"clr={max_clearance:.1f} m)."
        )

        if all(e["distance_m"] <= 0 and e["clearance_m"] <= 0 for e in self._sim_entries):
            self._stop_simulation("Intersection cleared — simulation complete.")

    def _stop_simulation(self, status_text: str) -> None:
        self._sim_timer.stop()
        self._sim_entries = []
        self._sim_target = ("", 0)
        self.simulate_btn.setText("Simulate Approach")
        self._set_controls_enabled(True)
        self.status_label.setText(status_text)

    def _set_controls_enabled(self, enabled: bool) -> None:
        """Enable or disable controls that should not be touched during simulation."""
        for widget in (
            self.add_btn, self.edit_btn, self.remove_btn,
            self.send_btn, self.ip_input, self.port_input,
        ):
            widget.setEnabled(enabled)
        self.table.setEnabled(enabled)


# Entry point
def main() -> None:
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
