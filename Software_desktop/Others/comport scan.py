import serial.tools.list_ports

ports = serial.tools.list_ports.comports()

print([x[0] for x in sorted(ports)])