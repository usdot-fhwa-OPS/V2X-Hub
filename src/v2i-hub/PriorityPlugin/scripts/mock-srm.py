#!/usr/bin/python3
import datetime
import sys
import json
import threading
import socket

import j2735_202409
from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import (
    QApplication,
    QComboBox,
    QDialog,
    QDialogButtonBox,
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
COL_ETA = 2
COL_EDT = 3
COL_ENTITY = 4
COL_ROLE = 5
COL_REQUEST_TYPE = 6
COLUMN_LABELS = [
    "Intersection ID",
    "Inbound Lane",
    "ETA (s from now)",
    "EDT (s from ETA)",
    "Entity ID",
    "Role",
    "Request Type",
]

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
        intersection_id, inbound_lane, eta_s, edt_s, entity_id, role, request_type
    """
    global msgCnt
    with msgCnt_lock:
        cnt = msgCnt
        msgCnt = (msgCnt + 1) & 0x7F

    requests = []
    for entry in entries:
        eta_moy, eta_ds = offsetToMOYAndDSecond(entry["eta_s"])

        # edt_s is seconds from ETA, convert to duration in milliseconds
        duration = max(entry["edt_s"] * 1000, 0)

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
                "duration": duration,
            }
        )

    # All entries in this call share the same entity; use the first for requestor fields
    first = entries[0]
    entity_id = first["entity_id"]
    role = first["role"]

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
                    "speed": {"transmisson": "unavailable", "speed": 415},
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

        self.eta = QSpinBox()
        self.eta.setRange(0, 3600)
        self.eta.setValue(defaults.get("eta_s", 5))
        self.eta.setSuffix(" s")

        self.edt = QSpinBox()
        self.edt.setRange(0, 3600)
        self.edt.setValue(defaults.get("edt_s", 3))
        self.edt.setSuffix(" s")

        self.entity_id = QLineEdit(defaults.get("entity_id", "12345678"))

        self.role = QComboBox()
        self.role.addItems(ROLE_CHOICES)
        self.role.setCurrentText(defaults.get("role", "transit"))

        self.request_type = QComboBox()
        self.request_type.addItems(REQUEST_TYPE_CHOICES)
        self.request_type.setCurrentText(defaults.get("request_type", "Request"))

        layout.addRow("Intersection ID:", self.intersection_id)
        layout.addRow("Inbound Lane:", self.inbound_lane)
        layout.addRow("Est. Time of Arrival:", self.eta)
        layout.addRow("Est. Departure Time:", self.edt)
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
            "eta_s": self.eta.value(),
            "edt_s": self.edt.value(),
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

        # Send button
        self.send_btn = QPushButton("Send SRM")
        self.send_btn.setFixedHeight(36)
        root_layout.addWidget(self.send_btn)

        self.status_label = QLabel("")
        self.status_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        root_layout.addWidget(self.status_label)

        # Signals
        self.add_btn.clicked.connect(self._on_add)
        self.edit_btn.clicked.connect(self._on_edit)
        self.remove_btn.clicked.connect(self._on_remove)
        self.send_btn.clicked.connect(self._on_send)
        self.table.doubleClicked.connect(lambda idx: self._edit_row(idx.row()))

    # Slots
    def _set_row(self, row: int, vals: dict) -> None:
        """Write an entry dict into the given table row."""
        self.table.setItem(row, COL_INTERSECTION, QTableWidgetItem(str(vals["intersection_id"])))
        self.table.setItem(row, COL_LANE, QTableWidgetItem(str(vals["inbound_lane"])))
        self.table.setItem(row, COL_ETA, QTableWidgetItem(str(vals["eta_s"])))
        self.table.setItem(row, COL_EDT, QTableWidgetItem(str(vals["edt_s"])))
        self.table.setItem(row, COL_ENTITY, QTableWidgetItem(vals["entity_id"]))
        self.table.setItem(row, COL_ROLE, QTableWidgetItem(vals["role"]))
        self.table.setItem(row, COL_REQUEST_TYPE, QTableWidgetItem(vals["request_type"]))

    def _read_row(self, row: int) -> dict:
        """Read a single table row back into an entry dict."""
        return {
            "intersection_id": int(self.table.item(row, COL_INTERSECTION).text()),
            "inbound_lane": int(self.table.item(row, COL_LANE).text()),
            "eta_s": int(self.table.item(row, COL_ETA).text()),
            "edt_s": int(self.table.item(row, COL_EDT).text()),
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

    def _on_send(self):
        if self.table.rowCount() == 0:
            QMessageBox.warning(self, "No entries", "Add at least one SRM entry before sending.")
            return

        ip = self.ip_input.text().strip()
        port = self.port_input.value()
        entries = self._read_entries()

        # Group entries by Entity ID so each entity gets its own SRM
        groups: dict[str, list[dict]] = {}
        for entry in entries:
            groups.setdefault(entry["entity_id"], []).append(entry)

        try:
            frame = j2735_202409.MessageFrame.MessageFrame
            total_bytes = 0
            for entity_id, group_entries in groups.items():
                srm_str = buildSRM(group_entries)
                frame.from_jer(srm_str)
                uper = frame.to_uper()
                sendMessage(uper, ip, port)
                total_bytes += len(uper)
            self.status_label.setText(
                f"Sent {len(groups)} SRM(s) ({total_bytes} bytes total) to {ip}:{port}"
            )
        except Exception as exc:
            QMessageBox.critical(self, "Send Error", str(exc))
            self.status_label.setText("Send failed.")


# Entry point
def main() -> None:
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
