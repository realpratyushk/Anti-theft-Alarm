🔐 Smart Anti-Theft Alarm System

An Arduino-based smart security system designed to protect lockers from unauthorized access using PIR motion detection, PIN authentication, buzzer, LEDs, and LCD display.

✨ Features
🚨 Motion detection using PIR sensor
🔢 4×4 keypad PIN authentication
🔊 Buzzer alarm for unauthorized access
🔴 Red LED warning/alarm indicator
🟢 Green LED armed/silent indicator
🔒 Lockout after 3 incorrect PIN attempts
⏱️ Warning, silent, and lockout timers
📟 16×2 LCD status display
🛠️ Components
Arduino Uno
PIR Sensor
4×4 Keypad
16×2 LCD
Buzzer
Red & Green LEDs
Resistors & jumper wires
⚙️ System Flow

Motion Detected → Warning → PIN Verification → Access Granted / Alarm → Lockout after 3 wrong attempts

🚀 Getting Started
Open anti_theft_alarm1.ino in Arduino IDE.
Connect the components according to the circuit.
Install the LiquidCrystal and Keypad libraries.
Upload the code to Arduino Uno.
Default PIN: 2580

Note: The current Arduino code implements the local alarm and security logic. Mobile/IoT notification functionality is part of the proposed system concept but is not implemented in this .ino file.

📁 Files
anti_theft_alarm1.ino — Arduino source code
Circuit.png — Circuit diagram
Presentation.pdf — Project presentation

Built with ❤️ using Arduino Uno.
