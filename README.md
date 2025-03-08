# Sea Drone Project

## Introduction
The Sea Drone project is an autonomous marine vehicle designed for navigation and monitoring. It integrates embedded systems, sensors, and communication modules to enable real-time data collection and remote control. This project is built using ESP8266, ESP32 CAM, Arduino Mega 2560, GPS, IMU, and T200 thrusters for propulsion.

## System Architecture
The system architecture is designed to efficiently collect and transmit data while maintaining precise navigation. Below is a high-level diagram illustrating the system components:

![System Architecture](./439914853_284398504762663_6579418254427418034_n.jpg)

### Components Used:
- **ESP8266**: Handles MQTT communication, sensor data processing, and interfacing with the Arduino Mega.
- **ESP32-CAM**: Provides real-time video streaming over a WebSocket connection.
- **Arduino Mega 2560**: Controls the propulsion system (ESC and T200 thrusters) using PWM signals.
- **GPS NEO7m**: Provides real-time position data.
- **MPU9250 IMU**: Measures orientation and heading.
- **ESC Controller**: Controls the dual T200 thrusters for movement.
- **Web Page Interface**: Displays real-time video feed and telemetry data.

## Web Interface
The sea drone can be monitored and controlled remotely via a web interface that displays telemetry data, including:
- Speed (in knots)
- Heading (in degrees)
- GPS Position (Longitude & Latitude)

Below is a screenshot of the web interface displaying the drone’s real-time position and status:

![Web Interface](./439544365_968023964764443_7626106271228992843_n.jpg)

## Codebase
The software running on the Sea Drone is divided into two main parts:
1. **ESP32 Web Page Interface**: Manages WebSocket communication and video streaming.
   - [Final ESP32 Code](./final.code.for.esp32.page.ino)

2. **Arduino Mega Control System**: Handles propulsion and sensor data processing.
   - [Arduino Mega Code](./arduino.fini.ino)

## Features
- **Remote Monitoring**: Live telemetry and video streaming via a web interface.
- **Autonomous Navigation**: GPS-based position tracking and heading correction.
- **IMU Stabilization**: Ensures smooth navigation in varying water conditions.
- **Dual Thruster Propulsion**: Provides efficient and precise movement control.

## Future Enhancements
- Integrate obstacle detection using sonar or LiDAR.
- Implement AI-based navigation for autonomous missions.
- Enhance the web interface with advanced controls and logging features.

## Conclusion
The Sea Drone project is an innovative solution for remote marine exploration and data collection. Its combination of embedded systems, real-time communication, and web-based control makes it a valuable tool for various applications, including environmental monitoring, search and rescue, and marine research.
"""

# Write the content to a README.md file
with open("README.md", "w") as file:
    file.write(readme_content)

print("README.md file has been created successfully.")
