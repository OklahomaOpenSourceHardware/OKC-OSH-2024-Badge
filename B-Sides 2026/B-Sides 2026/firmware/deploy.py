import subprocess
import time
import sys
import os
import serial.tools.list_ports

# --- CONFIGURATION ---
DEBOUNCE_DELAY = 0.8  
COOLDOWN = 1.0        

# Determine PIO path based on OS
if sys.platform == "win32":
    PIO_PATH = os.path.join("prod_env", "Scripts", "pio.exe")
else:
    PIO_PATH = os.path.join("prod_env", "bin", "pio")

def get_serial_port():
    """Finds the first available USB serial port."""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        # Common USB-Serial chips: CH340, CP210x, FTDI
        if "USB" in port.description or "UART" in port.description or "SER" in port.description:
            return port.device
    return None

def check_chip(port):
    """Pings the chip via avrdude."""
    cmd = [PIO_PATH, "pkg", "exec", "-p", "tool-avrdude", "--", 
           "avrdude", "-c", "serialupdi", "-P", port, "-p", "t816", "-n"]
    # We suppress output to keep the terminal clean during polling
    res = subprocess.run(cmd, capture_output=True)
    return res.returncode == 0

def flash_all():
    """Flashes Fuses + Firmware."""
    print("\n[!] TARGET DETECTED. DO NOT REMOVE...")
    # -t setfuses: Ensures BOD/Clock are locked in
    # -t upload: Flashes the code
    cmd = [PIO_PATH, "run", "-t", "setfuses", "-t", "upload"]
    res = subprocess.run(cmd)
    return res.returncode == 0

def main():
    print(f"--- ATtiny816 Production Station (OS: {sys.platform}) ---")
    
    port = get_serial_port()
    if not port:
        print("❌ ERROR: No USB-Serial adapter found. Plug it in and restart.")
        sys.exit(1)
        
    print(f"Found adapter on: {port}")
    print("Ready for board insertion...")

    while True:
        if check_chip(port):
            time.sleep(DEBOUNCE_DELAY) 
            
            if check_chip(port): 
                if flash_all():
                    print("\n✅ PASS: Success. PLEASE REMOVE BOARD.")
                    
                    # Wait for removal
                    while check_chip(port):
                        time.sleep(0.5)
                    
                    print("--- Ready for next unit ---\n")
                    time.sleep(COOLDOWN)
                else:
                    print("\n❌ FAIL: Flash error. Check connection.")
                    time.sleep(2)
        
        time.sleep(0.2)

if __name__ == "__main__":
    main()