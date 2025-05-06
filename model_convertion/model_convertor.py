import os

tflite_file_path = "/home/takudzwa/Documents/PlatformIO/Projects/model_convertion/broiler_disease_esp32.tflite"
output_header_path = "/home/takudzwa/Documents/Projects/broiler_disease/hardware/esp32_camera/broiler_disease_esp32_model.h"
variable_name = "model_data"

try:
    print("Attempting to open the .tflite file...")
    with open(tflite_file_path, "rb") as f:
        print(".tflite file opened successfully.")
        tflite_model_content = f.read()
        print("Finished reading .tflite file.")
        total_bytes = len(tflite_model_content)
        print(f"Processing {total_bytes} bytes...")
        header_content = f'const unsigned char {variable_name}[] = {{\n  '
        for i, byte in enumerate(tflite_model_content):
            header_content += f'0x{byte:02x}'
            if i < total_bytes - 1:
                header_content += ','
            if (i + 1) % 12 == 0:
                header_content += '\n  '
            else:
                header_content += ' '
            if i % 10000 == 0:
                percentage = (i / total_bytes) * 100
                print(f"Processed {i} bytes ({percentage:.2f}%)")
        header_content += '\n};\n\n'
        header_content += f'const int {variable_name}_len = sizeof({variable_name});\n'

        with open(output_header_path, "w") as f:
            f.write(header_content)

        print(f"Successfully converted {tflite_file_path} to {output_header_path}")


except FileNotFoundError:
    print(f"Error: File not found at {tflite_file_path}")
except Exception as e:
    print(f"An error occurred: {e}")