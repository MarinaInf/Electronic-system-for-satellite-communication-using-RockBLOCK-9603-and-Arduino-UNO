
## Introduction
This project is my Bachelor Thesis of the degree in Physics in the University of Seville. The main goal of this project is getting to communicate with an Iridium satellite. The message sent contains the latitude and 
longitude, obtained by a GPS. In this repository, you will be able to find the main code used for this project, as well as schematics and images showing the message sent, both in RockBLOCK Admin and in the Serial 
Monitor of the Arduino UNO. Besides, some videos showing the whole system, which includes an OLED display, are uploaded. The  display flicker in the videos is due to camera and display refresh rate mismatch.
Furthermore, a code to use the RockBLOCK 9603 on its own is provided. This is useful to check if the device functions correctly.

## Hardware
The devices used for this project are:
- Arduino UNO 
- OLED display SSD1306
- Adafruit Ultimate GPS Breakout
- RockBLOCK 9603

Besides, a 100uF capacitor is used to make the voltage in the RockBLOCK stable. The whole system is shown in the next image.![prueba_final_montaje (1)](https://github.com/user-attachments/assets/21aaaf6e-eebb-4d7e-885e-1abba4f104d7)


## Code
The code used for this project follows these steps: 
- It initializes the GPS and tries to get a GPS fix.
- It shows in the OLED display that the system is waiting for a GPS fix
- When the GPS is fixed, the communication between GPS and Arduino is ended
- The RockBLOCK is initialized
- It attempts to send a message, which is a text message with the latitude and longitude of the GPS
- If successful, the OLED display shows a message saying "Sent" and it shows the latitude and longitude as well
- If not successful, the OLED display shows a message saying "Not sent"
- The program stops, so it does not attempt to send another message

## Credits and line rent
In order to send messages using RockBLOCK 9603, it is required to rent a line and an amount of credits in RockBLOCK Admin. . Line rental is 14,25 euros 14,66$. 100 credits cost 15,44 euros e 15,88$. 
Each message consumes one credit for every 50 bytes, with a maximum message size of 340 bytes. However, there are different plans and prices available.
