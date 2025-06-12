import os

folder_path = "/home/takudzwa/Documents/Projects/broiler_disease/detection/FOMO/ESP_32/train/healthy"

image_files = [f for f in os.listdir(folder_path) if f.lower().endswith(('.jpg', '.jpeg', '.png'))]


image_files.sort()


for i, filename in enumerate(image_files, start=1):
    new_name = f"health_{i}.jpg"
    src = os.path.join(folder_path, filename)
    dst = os.path.join(folder_path, new_name)
    os.rename(src, dst)

print(f"Renamed {len(image_files)} images.")
