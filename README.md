# IoT-Project-Smart-Plug

The Smart plug has ESP8266 as the main microcontroller to process and control each socket's switch on and off states, store the socket's data and condition to MySQL, and control and monitor data via the localhost web app(Node-red), which is inside Raspberry Pi 4.

Inside the Raspberry Pi 4, which acts as a LAMP server (Linux, Apache, MySQL, PHP server), we use MySQL for the database (We also install phpMyAdmin for analyzing data), then use Node-RED to construct the web app for controlling the Smart Plug.

## Smart Plug functioning demonstration
Demonstration Video => [Project Demonstration YouTube Video](https://youtu.be/FFx-KPFvHos)

## Official Smart Plug Page
Official Page => [Smart Plug's Notion Page](https://gem-toast-375.notion.site/Smart-plug-d28bfbf477734b41ad5f5a393770ff33)

## What can the Smart Plug do?
1. Collect and display the energy consumption of the plug.
2. Control the state of the socket.
3. Send LINE messages to notify when the state change.

This project is part of the Internet of Things and Smart Robotics and AI Systems class at King Mongkut's Institute of Technology Ladkrabang (KMITL). The team members are in the third year of Robotics and AI Engineering Major.
