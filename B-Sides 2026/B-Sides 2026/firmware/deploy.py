import subprocess
import time
import sys
import os
import serial.tools.list_ports

# --- CONFIGURATION ---
DEBOUNCE_DELAY = 0.8  
COOLDOWN = 1.0        
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
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
    res = subprocess.run(cmd, capture_output=True, text=True)
    return res.returncode == 0

def flash_all():
    """Flashes Fuses and Firmware. Cross-platform compatible."""
    print("\n[!] TARGET DETECTED. DO NOT REMOVE...")

    # 1. Define the Targets
    # We use 'fuses' as the target name for megaTinyCore
    targets = ["fuses", "upload"]
    
    for target in targets:
        print(f"  -> Executing: {target.upper()}...")
        
        # Build the command parts
        cmd_args = [PIO_PATH, "run", "-e", "tiny816", "--target", target]
        
        if sys.platform == "win32":
            # Windows needs the command as a string and shell=True 
            # to correctly resolve the .exe in the virtual env
            cmd = " ".join(f'"{arg}"' if " " in arg else arg for arg in cmd_args)
            res = subprocess.run(cmd, cwd=BASE_DIR, shell=True)
        else:
            # Linux/macOS works best with a list and no shell
            res = subprocess.run(cmd_args, cwd=BASE_DIR)

        if res.returncode != 0:
            print(f"  ❌ {target.upper()} Failed.")
            return False

    return True
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
