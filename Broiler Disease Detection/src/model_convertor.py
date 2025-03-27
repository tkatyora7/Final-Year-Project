import os

tflite_file_path = "/home/takudzwa/Documents/Projects/broiler_disease/final_year_broiler/fomo_model.tflite"
output_header_path = "fomo_model.h"
variable_name = "model_data"

try:
    with open(tflite_file_path, "rb") as f:
        tflite_model_content = f.read()

    header_content = f'const unsigned char {variable_name}[] = {{\n  '
    for i, byte in enumerate(tflite_model_content):
        header_content += f'0x{byte:02x}'
        if i < len(tflite_model_content) - 1:
            header_content += ','
        if (i + 1) % 12 == 0:  # Format for readability
            header_content += '\n  '
        else:
            header_content += ' '
    header_content += '\n};\n\n'
    header_content += f'const int {variable_name}_len = sizeof({variable_name});\n' # Correct way to get size

    with open(output_header_path, "w") as f:
        f.write(header_content)

    print(f"Successfully converted {tflite_file_path} to {output_header_path}")
    

except FileNotFoundError:
    print(f"Error: File not found at {tflite_file_path}")
except Exception as e:
    print(f"An error occurred: {e}")