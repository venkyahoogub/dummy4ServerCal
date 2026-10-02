#!/usr/bin/env python3
"""
PyQt5 GUI application that streams all 3 cameras using the protobuf protocol.
Maintains separate socket connections for stereo and wide streams.
"""

import sys
import socket
import struct
import threading
import cv2
import numpy as np
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton
from PyQt5.QtGui import QImage, QPixmap, QFont
from PyQt5.QtCore import QTimer, Qt, pyqtSignal, QObject
import calibration_server_api_pb2 as api

class CameraSignals(QObject):
    """Signals for thread-safe frame updates."""
    frame_received = pyqtSignal(str, np.ndarray)
    status_changed = pyqtSignal(str, str)

class StreamConnection:
    """Manages a single stream connection to the server."""
    
    def __init__(self, host='127.0.0.1', port=50051, stream_type='stereo'):
        self.host = host
        self.port = port
        self.stream_type = stream_type
        self.socket = None
        self.connected = False
        self.signals = CameraSignals()
        self.running = False
        
    def connect(self):
        """Connect to the server."""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            self.connected = True
            self.signals.status_changed.emit(self.stream_type.capitalize(), "Connected")
            return True
        except Exception as e:
            self.signals.status_changed.emit(self.stream_type.capitalize(), f"Failed to connect: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from the server."""
        self.running = False
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
            self.connected = False
            self.signals.status_changed.emit(self.stream_type.capitalize(), "Disconnected")
    
    def send_request(self, request):
        """Send a protobuf request to the server."""
        if not self.connected:
            return False
        
        try:
            data = request.SerializeToString()
            length = struct.pack('>I', len(data))
            self.socket.sendall(length + data)
            return True
        except Exception as e:
            self.signals.status_changed.emit(self.stream_type.capitalize(), f"Send error: {e}")
            self.connected = False
            return False
    
    def receive_response(self, timeout=2):
        """Receive a protobuf response from the server."""
        if not self.connected:
            return None
        
        try:
            self.socket.settimeout(timeout)
            length_data = self.socket.recv(4)
            if not length_data:
                return None
            
            length = struct.unpack('>I', length_data)[0]
            
            data = b''
            while len(data) < length:
                chunk = self.socket.recv(length - len(data))
                if not chunk:
                    break
                data += chunk
            
            response = api.FromServer()
            response.ParseFromString(data)
            return response
        except socket.timeout:
            return None
        except Exception as e:
            self.connected = False
            return None

class StereoStreamConnection(StreamConnection):
    """Manages stereo camera stream."""
    
    def __init__(self, host='127.0.0.1', port=50051):
        super().__init__(host, port, 'stereo')
    
    def start_stream(self):
        """Start stereo stream."""
        request = api.ToServer()
        request.start_video_stream.CopyFrom(api.StartVideoStream())
        return self.send_request(request)
    
    def stop_stream(self):
        """Stop stereo stream."""
        request = api.ToServer()
        request.stop_video_stream.CopyFrom(api.StopVideoStream())
        return self.send_request(request)

class WideStreamConnection(StreamConnection):
    """Manages wide camera stream."""
    
    def __init__(self, host='127.0.0.1', port=50051):
        super().__init__(host, port, 'wide')
    
    def start_stream(self):
        """Start wide stream."""
        request = api.ToServer()
        request.get_wide_video_stream.CopyFrom(api.GetWideVideoStream())
        return self.send_request(request)
    
    def stop_stream(self):
        """Stop wide stream."""
        request = api.ToServer()
        request.stop_wide_video_stream.CopyFrom(api.StopWideVideoStream())
        return self.send_request(request)

class CameraStreamGUI(QWidget):
    def __init__(self):
        super().__init__()
        
        # Create separate connections for each stream
        self.stereo_conn = StereoStreamConnection()
        self.stereo_conn.signals.frame_received.connect(self.on_frame_received)
        self.stereo_conn.signals.status_changed.connect(self.on_status_changed)
        
        self.wide_conn = WideStreamConnection()
        self.wide_conn.signals.frame_received.connect(self.on_frame_received)
        self.wide_conn.signals.status_changed.connect(self.on_status_changed)
        
        self.stereo_thread = None
        self.wide_thread = None
        
        # Fixed display size
        self.display_width = 480
        self.display_height = 480
        
        self.init_ui()
    
    def init_ui(self):
        """Initialize the UI."""
        self.setWindowTitle("Calibration Server - 3 Camera Stream Viewer")
        self.setGeometry(100, 100, 1600, 700)
        self.setFixedSize(1600, 700)  # Fix window size
        
        main_layout = QVBoxLayout()
        
        # Control panel
        control_layout = QHBoxLayout()
        
        self.btn_stereo = QPushButton("Start Stereo Stream")
        self.btn_stereo.clicked.connect(self.on_toggle_stereo)
        control_layout.addWidget(self.btn_stereo)
        
        self.btn_wide = QPushButton("Start Wide Stream")
        self.btn_wide.clicked.connect(self.on_toggle_wide)
        control_layout.addWidget(self.btn_wide)
        
        self.status_label = QLabel("Ready")
        self.status_label.setFont(QFont("Arial", 10))
        control_layout.addWidget(self.status_label)
        
        main_layout.addLayout(control_layout)
        
        # Camera display panel
        camera_layout = QHBoxLayout()
        
        self.labels = {}
        for name in ["Left", "Right", "Wide"]:
            layout = QVBoxLayout()
            title = QLabel(name)
            title.setFont(QFont("Arial", 12, QFont.Bold))
            
            label = QLabel(f"{name} Camera")
            label.setStyleSheet("border: 2px solid #333; background-color: #222; color: white;")
            # Set fixed size for display
            label.setFixedSize(self.display_width, self.display_height)
            label.setAlignment(Qt.AlignCenter)
            label.setFont(QFont("Arial", 10))
            label.setScaledContents(True)  # Let the label scale the pixmap
            
            self.labels[name] = label
            
            layout.addWidget(title)
            layout.addWidget(label)
            layout.addStretch()  # Add stretch to push content to top
            camera_layout.addLayout(layout)
        
        main_layout.addLayout(camera_layout)
        main_layout.addStretch()
        self.setLayout(main_layout)
    
    def on_toggle_stereo(self):
        """Handle stereo stream toggle."""
        if not self.stereo_conn.running:
            # Connect and start
            if self.stereo_conn.connect():
                if self.stereo_conn.start_stream():
                    self.stereo_conn.running = True
                    self.btn_stereo.setText("Stop Stereo Stream")
                    
                    # Start receive thread
                    self.stereo_thread = threading.Thread(target=self.stereo_stream_worker, daemon=True)
                    self.stereo_thread.start()
                    self.on_status_changed("Stereo", "Stream started")
                else:
                    self.stereo_conn.disconnect()
        else:
            # Stop and disconnect
            self.stereo_conn.stop_stream()
            self.stereo_conn.running = False
            self.stereo_conn.disconnect()
            self.btn_stereo.setText("Start Stereo Stream")
            self.on_status_changed("Stereo", "Stream stopped")
    
    def on_toggle_wide(self):
        """Handle wide stream toggle."""
        if not self.wide_conn.running:
            # Connect and start
            if self.wide_conn.connect():
                if self.wide_conn.start_stream():
                    self.wide_conn.running = True
                    self.btn_wide.setText("Stop Wide Stream")
                    
                    # Start receive thread
                    self.wide_thread = threading.Thread(target=self.wide_stream_worker, daemon=True)
                    self.wide_thread.start()
                    self.on_status_changed("Wide", "Stream started")
                else:
                    self.wide_conn.disconnect()
        else:
            # Stop and disconnect
            self.wide_conn.stop_stream()
            self.wide_conn.running = False
            self.wide_conn.disconnect()
            self.btn_wide.setText("Start Wide Stream")
            self.on_status_changed("Wide", "Stream stopped")
    
    def stereo_stream_worker(self):
        """Worker thread for stereo stream reception."""
        while self.stereo_conn.running:
            response = self.stereo_conn.receive_response(timeout=1)
            if response and response.HasField("stream_frame"):
                frame = response.stream_frame.frame
                
                # Send left frame
                left_data = np.frombuffer(frame.left.image, dtype=np.uint8)
                left_image = left_data.reshape((frame.left.height, frame.left.width))
                self.stereo_conn.signals.frame_received.emit("Left", left_image)
                
                # Send right frame
                right_data = np.frombuffer(frame.right.image, dtype=np.uint8)
                right_image = right_data.reshape((frame.right.height, frame.right.width))
                self.stereo_conn.signals.frame_received.emit("Right", right_image)
    
    def wide_stream_worker(self):
        """Worker thread for wide stream reception."""
        while self.wide_conn.running:
            response = self.wide_conn.receive_response(timeout=1)
            if response and response.HasField("wide_stream_frame"):
                wide_frame = response.wide_stream_frame.wide
                wide_data = np.frombuffer(wide_frame.image, dtype=np.uint8)
                wide_image = wide_data.reshape((wide_frame.height, wide_frame.width))
                self.wide_conn.signals.frame_received.emit("Wide", wide_image)
    
    def on_frame_received(self, camera_name, image):
        """Handle frame received signal."""
        if image is None or image.size == 0:
            return
        
        # Convert grayscale to RGB for display
        if len(image.shape) == 2:
            image_rgb = cv2.cvtColor(image, cv2.COLOR_GRAY2RGB)
        else:
            image_rgb = image
        
        # Add camera info
        cv2.putText(image_rgb, f"{camera_name} {image.shape[1]}x{image.shape[0]}", 
                   (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
        
        # Convert to QImage without scaling
        h, w, ch = image_rgb.shape
        bytes_per_line = 3 * w
        qt_image = QImage(image_rgb.data, w, h, bytes_per_line, QImage.Format_RGB888)
        pixmap = QPixmap.fromImage(qt_image)
        
        # Get label and set pixmap (label will handle scaling via setScaledContents)
        label = self.labels[camera_name]
        label.setPixmap(pixmap)
    
    def on_status_changed(self, category, message):
        """Handle status change signal."""
        self.status_label.setText(f"{category}: {message}")
    
    def closeEvent(self, event):
        """Clean up on close."""
        if self.stereo_conn.running:
            self.stereo_conn.stop_stream()
            self.stereo_conn.running = False
            self.stereo_conn.disconnect()
        
        if self.wide_conn.running:
            self.wide_conn.stop_stream()
            self.wide_conn.running = False
            self.wide_conn.disconnect()
        
        event.accept()


if __name__ == "__main__":
    print("=== Calibration Server Camera Stream Viewer ===\n")
    app = QApplication(sys.argv)
    window = CameraStreamGUI()
    window.show()
    sys.exit(app.exec_())