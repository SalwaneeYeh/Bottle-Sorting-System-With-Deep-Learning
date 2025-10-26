
# Bottle-Sorting-System-With-Deep-Learning
Bottle Sorting Machine Use Deep Learning and Weight to Sorting Bottle (Plastic, Glass bottle and Aluminnium)
```
# Set up Raspberry Pi 
1.ลง OS ใช้โปรแกรม Raspberry Pi imager --> Select Raspberry Pi Device:Raspberry Pi 5 , Operating System:Raspberry Pi OS (64 bit) , Storage:SD Card
2.boot raspberry pi ขึ้นจอ --> Configuration --> set Wifi Hostname password 
3.ติดตั้ง Python 3.9 และไลบรารี tflite_runtime, pillow, numpy, opencv, serial ใช้คำสั่งดังนี้
  เปิด terminal 
  sudo apt update
  sudo apt install -y build-essential libssl-dev zlib1g-dev libncurses5-dev libncursesw5-dev libreadline-dev libsqlite3-dev libgdbm-dev libdb5.3-dev libbz2-dev libexpat1-dev liblzma-dev tk-dev libffi-dev wget
  wget https://www.python.org/ftp/python/3.9.18/Python-3.9.18.tgz
  tar -xf Python-3.9.18.tgz
  cd Python-3.9.18
  ./configure --enable-optimizations
  make -j $(nproc)
  sudo make altinstall
  deactivate
  python3.9 -m venv bottle_env
  source bottle_env/bin/activate
  pip install "numpy<2"
  #ติดตั้ง tflite-runtime โดยนำเข้าจาก https://google-coral.github.io/py-repo/tflite-runtime/?utm_source=chatgpt.com เลือกดาวน์โหลดไฟล์ "tflite_runtime-2.5.0.post1-cp39-cp39-linux_aarch64.whl"
  pip install --no-deps /home/project/tflite_runtime-2.5.0.post1-cp39-cp39-linux_aarch64.whl #ตาม path ไฟล์ tflite_runtime
  pip install pillow
  pip install opencv-python
  pip install pyserial
4.ลงโค้ด run.py บนโปรแกรม Thonny 
5.ลงโฟล์เดอร์ Model --> model-v1.1 โดยประกอบด้วยไฟล์ model-v1.1.tflite, predict.py, labels.txt

# ขั้นตอนการทำ Raspberry Pi autostart code
1.เปิด terminal
2.ใช้คำสั่ง  sudo nano /etc/systemd/system/sorter.service
3.วางโค้ด
  [Unit]
  Description=Bottle Sort Python Script
  After=multi-user.target dev-ttyAMC0.device
  Wants=dev-ttyAMC0.device
  [Service]
  ExecStartPre=/bin/sleep 5
  ExecStart=/home/project/bottle_env/bin/python3 -u /home/project/run.py
  WorkingDirectory=/home/project
  StandardOutput=inherit
  StandardError=inherit
  Restart=always
  User=project
  Environment=PATH=/home/project/bottle_env/bin:/usr/bin:/bin
  Environment=DISPLAY=:0
  [Install]
  WantedBy=multi-user.target
4.บันทึกไฟล์ (Ctrl + O → Enter → Ctrl + X)
5.ใช้คำสั่ง nano /home/project/bottle_env/run_bottlesort.sh
6.วางโค้ด
  #!/bin/bash
  # Activate virtual environment
  source /home/project/bottle_env/bin/activate
  # Run Python script
  python /home/project/run.py
7.บันทึกไฟล์ (Ctrl + O → Enter → Ctrl + X)
8.ใช้คำสั่ง  sudo systemctl daemon-reload
          sudo systemctl restart sorter.service
          sudo systemctl status sorter.service
เมื่อสำเร็จ จะขึ้นสถานะเป็น Active: active (running)

# กรณีต้องการเปลี่ยน Model 
1.จัดโครงสร้างโฟลเดอร์ "ชื่อโฟลเดอร์โมเดล" ประกอบด้วย 1)ไฟล์ Model สกุล tflite 2)predict.py และ 3)labels.txt
2.นำโฟลเดอร์โมเดลเข้า Raspberry pi 
3.แก้โค้ดไฟล์ run.py และไฟล์ predict.py

# Arduino Atmega 328
ใช้บอร์ด Arduino 2 ตัวในการควบคุมระบบ 
บอร์ด (1) สำหรับควบคุมการส่ง-รับค่า,Load Cell,servo,เซ็นเซอร์ก้ามปู --> base_board.ino
  วิธีการ Calibrate Load Cell --> https://youtu.be/sxzoAGf1kOo?si=XqDgxRvnZsf30kAV
บอร์ด (2) สำหรับควบคุมการรับค่า,แสดงผลจอ TFT LCD --> tft.ino
