import requests
import sys
from SCons.Script import Import

Import("env")

def on_upload(source, target, env):
    firmware_path = str(source[0])
    upload_url = env.GetProjectOption("upload_port")

    print(f"\nUploading {firmware_path} to {upload_url} ...")

    try:
        with open(firmware_path, "rb") as f:
            # The 'update' key matches the <input name="update"> in your ESP32 HTML
            response = requests.post(upload_url, files={"update": f})

        if response.status_code == 200:
            print("\n[SUCCESS] Firmware uploaded successfully!")
            print(f"Server Response: {response.text}")
        else:
            print(f"\n[ERROR] Upload failed. Status Code: {response.status_code}")
            print(f"Response: {response.text}")
            sys.exit(1)

    except Exception as e:
        print(f"\n[ERROR] Connection failed: {e}")
        sys.exit(1)

# Register the upload handler
env.Replace(UPLOADCMD=on_upload)
