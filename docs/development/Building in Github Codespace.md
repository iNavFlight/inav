# Building in GitHub Codespaces
> **A codespace is a cloud-hosted development environment.**
> <br/>This guide provides a simple method to modify and build a version of INAV in the cloud.

## Setup
1. Navigate to the version of INAV you want to modify.
2. Create a new codespace for that version. `Code> Codespaces> Create codespace on 8.x.x.`
<img width="1567" height="769" alt="image" src="https://github.com/user-attachments/assets/4427ea1e-19ff-4c2c-8d99-3870c13f767b" />

> A new codespace will launch in your browser.


## Modify
3. Modify the code as required.
   You can freeley modify your copy of the INAV code from within the cloud environment.

## Build
Use the terminal inside the codespace environment to run the following commands:

### Prepare
#### 4. Prepare the build:
```bash
cmake -B build -S .
```
<img width="1566" height="989" alt="image" src="https://github.com/user-attachments/assets/5747e49b-256b-4afb-9a16-3c592ba6773c" />

> Note: You may need to use ctrl+C to exit the process and return to the shell once it completes.

### Target
#### 5. (Optional) If you don't know your target name, list the availble targests with:
```bash
cmake --build build --target help
```
<img width="1520" height="377" alt="image" src="https://github.com/user-attachments/assets/c3fc5099-ed92-4006-94ca-d7b777d31f2f" />

### Build
#### 6. Build the binary (replace the target name with your specific one):
``` bash
cmake --build build --target NEUTRONRCF435MINI
```
<img width="1566" height="373" alt="image" src="https://github.com/user-attachments/assets/bb56ea92-0b2a-423c-b6dd-edec39f6e358" />

## Download
7. Once the build process has complete, download the .hex binary from the build folder on the right.
   Example: `inav_8.0.1_NEUTRONRCF435MINI.hex` (Right-click > Download.
<img width="1566" height="978" alt="image" src="https://github.com/user-attachments/assets/fd3bdeb4-459f-433b-ab70-74a49b26712f" />

> Note: Codespaces are automatically deleted after a period of inactivity, or you can manually deleted them at https://github.com/codespaces

