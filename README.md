
# EdgeVision-High-Resolution-Edge-Detection-with-Reconfigurable-Computing
**EdgeVision** is a project that applies a Sobel filter to BMP images for edge detection. This filter is commonly used in image processing to highlight the edges by calculating the gradient of the image intensity at each pixel. The project is designed to work on an FPGA, making it suitable for high-speed, hardware-based image processing applications.

## Features
- **Edge Detection**: Applies the Sobel filter to an image for edge detection.
- **Boundary Handling**: Edge pixels are replicated to handle boundary conditions.
- **Flexible Input/Output**: Supports processing multiple BMP images with options for logging.

## Requirements

- **ARM Cross-Compiler**: Required to compile the code for the ARM processor on the DE1-SoC.
- **Quartus Prime**: To configure and program the FPGA on the DE1-SoC.
- **BMP Images**: The program supports 24-bit RGB or 8-bit grayscale BMP images.

## Usage

To process images with the Sobel filter, use the following command structure:

```bash
./main -o/-w input1.bmp [input2.bmp input3.bmp]
```
## Options
- **-o**: Write processed output to a log file (`HPS_output.txt`).
- **-w**: Process images without writing to a log file.

### Examples:
- To process a single image and write the output to a log file:
  ```bash
  ./main -o image.bmp
  ```
- To process multiple images without logging:
  ```bash
  ./main -w image1.bmp image2.bmp
  ```
## Compilation

To compile the program, navigate to the project directory and run:
```bash
make
```
## Upload the .sof File to the DE1-SoC

To upload the compiled `.sof` file (FPGA configuration bitstream) to the DE1-SoC, follow these steps:

1. **Connect the DE1-SoC to your computer** via USB-Blaster or another compatible JTAG interface.

2. **Open Quartus Programmer**:
   - Open the Quartus software (you need to have Quartus installed on your system).
   - Launch the **Quartus Programmer**.

3. **Load the .sof File**:
   - In Quartus Programmer, click **Add File** and browse to the location of your `.sof` file (this file is generated after compilation in Quartus).
   - Select the `.sof` file and load it.

4. **Select the DE1-SoC Device**:
   - In the Quartus Programmer, ensure that the **DE1-SoC** is selected as the target device.
   - If necessary, select the USB-Blaster or another programming cable.

5. **Program the FPGA**:
   - Click **Start** to upload the `.sof` file to the FPGA.
   - Wait for the process to finish.

6. **Verify the Upload**:
   - Once the programming is complete, the FPGA on the DE1-SoC should be configured and ready to run your design.


## Notes
- The program expects BMP files in 24-bit RGB or 8-bit grayscale format.
- Boundary pixels are handled by replicating the edge pixels to avoid artifacts.
- The program can process multiple images in a single execution. If multiple input files are specified, all images will be processed sequentially.
