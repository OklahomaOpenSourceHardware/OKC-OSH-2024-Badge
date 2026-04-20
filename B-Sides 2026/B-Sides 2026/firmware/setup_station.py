import os
import subprocess
import sys
import venv
import shutil

def run_cmd(cmd, env=None):
    print(f"> Running: {' '.join(cmd)}")
    subprocess.run(cmd, check=True, env=env)

def main():
    # 1. Setup paths based on OS
    venv_dir = os.path.join(os.getcwd(), "prod_env")
    
    if sys.platform == "win32":
        python_bin = os.path.join(venv_dir, "Scripts", "python.exe")
        pio_bin = os.path.join(venv_dir, "Scripts", "pio.exe")
    else:
        python_bin = os.path.join(venv_dir, "bin", "python")
        pio_bin = os.path.join(venv_dir, "bin", "pio")

    # 2. Clean old environment if it exists
    if os.path.exists(venv_dir):
        print("--- Removing existing environment ---")
        shutil.rmtree(venv_dir)

    # 3. Create fresh Virtual Environment
    print(f"--- Creating Virtual Environment on {sys.platform} ---")
    venv.create(venv_dir, with_pip=True)

    # 4. Install/Upgrade PlatformIO
    print("--- Installing PlatformIO Core ---")
    # Add pyserial for port detection
    run_cmd([python_bin, "-m", "pip", "install", "--upgrade", "pip", "platformio", "pyserial"])
    # 5. Pre-fetch dependencies
    # This downloads the ATtiny816 toolchain and megaTinyCore libraries now
    # so the production line doesn't wait on the first chip.
    print("--- Pre-loading Toolchains and Libraries ---")
    run_cmd([pio_bin, "pkg", "install"])

    print("\n" + "="*50)
    print("SETUP COMPLETE")
    print(f"To start programming, use this command:")
    print(f"  {python_bin} deploy.py")
    print("="*50)

if __name__ == "__main__":
    main()