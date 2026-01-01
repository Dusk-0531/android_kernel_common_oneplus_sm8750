# GitHub Actions Workflows

## Build Android Kernel (AK3)

This workflow automatically builds the Android kernel and packages it with AnyKernel3 (AK3) for easy flashing.

### Triggers

The workflow runs on:
- Push to `main`, `master`, or any `android*` branches
- Pull requests to `main`, `master`, or any `android*` branches
- Manual trigger via GitHub Actions UI (workflow_dispatch)

### Build Process

1. **Setup Environment**: Installs all necessary build dependencies including build-essential, flex, bison, libelf-dev, etc.

2. **Setup Clang Toolchain**: Downloads and extracts the Clang toolchain (clang-r522817) required for building Android kernels.

3. **Setup Build Tools**: Clones AOSP kernel build tools for proper kernel compilation.

4. **Build Kernel**: 
   - Uses `gki_defconfig` for arm64 architecture
   - Compiles with LLVM/Clang
   - Outputs to `out/` directory

5. **Prepare AnyKernel3**: 
   - Clones the AnyKernel3 template
   - Copies the compiled kernel image (Image.gz)
   - Copies DTB/DTBO files if available
   - Configures for sm8750/OnePlus devices

6. **Package**: Creates a flashable AK3 ZIP file with SHA256 checksum

### Artifacts

The workflow uploads three artifacts:

1. **kernel-image**: Raw kernel images (Image.gz and Image)
2. **AnyKernel3-flashable**: Flashable ZIP file and its checksum
3. **build-info**: Build configuration and kernel version info

### Usage

#### Automatic Builds
The workflow runs automatically when you push code to the repository or create a pull request.

#### Manual Builds
1. Go to the "Actions" tab in your GitHub repository
2. Select "Build Android Kernel (AK3)" workflow
3. Click "Run workflow"
4. Select the branch you want to build
5. (Optional) Customize the build:
   - **device_name1**: First device identifier (default: sm8750)
   - **device_name2**: Second device identifier (default: oneplus)
   - **defconfig**: Kernel configuration to use (default: gki_defconfig)
6. Click "Run workflow" button

### Flashing the Kernel

1. Download the `AnyKernel3-flashable` artifact from the workflow run
2. Extract the ZIP file from the artifact
3. Boot your device into recovery mode (TWRP recommended)
4. Flash the AK3 ZIP file
5. Reboot

### Customization

The workflow supports customization through workflow inputs when running manually:

1. **device_name1** & **device_name2**: Set the target device names for AnyKernel3
2. **defconfig**: Choose which kernel configuration to use
   - `gki_defconfig` (default): Generic Kernel Image configuration
   - `defconfig`: Standard arm64 configuration
   - Any other defconfig available in `arch/arm64/configs/`

To customize for automated builds:

1. Edit the default values in the workflow file under `workflow_dispatch.inputs`
2. Modify the defconfig in the "Build Kernel" step
3. Update the device names in the "Prepare AnyKernel3" step
4. Adjust compiler flags if needed

### Troubleshooting

- **Build fails at configuration**: Check if the defconfig name is correct
- **Build fails at compilation**: Review the build logs for missing dependencies or compilation errors
- **AK3 ZIP doesn't flash**: Verify that the kernel image is compatible with your device

### Build Time

Typical build time on GitHub Actions: 30-60 minutes (depending on GitHub runner load)
