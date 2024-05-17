import torch
import torch
import pathlib
import sys # to access the system
import cv2
import urllib.request
import numpy as np
import concurrent.futures
import urllib.request
from PIL import Image
from io import BytesIO
temp = pathlib.PosixPath
pathlib.PosixPath = pathlib.WindowsPath
from pathlib import Path
# Define model path using Path object
model_path = Path(r'D:\do_an_khoa_hoc\yolov5_test\yolov5\runs\train\exp16\weights\best.pt')
# Load the model
model = torch.hub.load(str(Path(r'D:\do_an_khoa_hoc\yolov5_test\yolov5')), 'custom', path=model_path, source='local',force_reload=True)
url='http://192.168.93.50/cam-mid.jpg'
im=None

def run2():
    cv2.namedWindow("detection", cv2.WINDOW_AUTOSIZE)
    while True:
        with urllib.request.urlopen(url) as response:
            image_data = response.read()
        # Tạo đối tượng hình ảnh từ dữ liệu
        image = Image.open(BytesIO(image_data))
        # Chỉnh sửa kích thước hình ảnh thành 640x640
        image_resized = image.resize((640, 640))
        # Hiển thị hình ảnh đã chỉnh sửa kích thước
        image_resized.show()
        # Inference
        results = model(image_resized)
        print(results.pandas())
        results.print()
        results.show()
        cv2.imshow('detect', image_resized)
        key = cv2.waitKey(5)
        if key == ord('q'):
            break

    cv2.destroyAllWindows()

if __name__ == '__main__':
    print("started")
    with concurrent.futures.ProcessPoolExecutor() as executer:
        f2= executer.submit(run2)
