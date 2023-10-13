FROM ubuntu:latest

RUN apt-get update && apt upgrade
RUN apt-get install -y wget
RUN apt-get install -y python3-pip
RUN apt-get install -y python3-venv 
RUN python3 -m pip install -U platformio

RUN mkdir /home/pio_build

COPY . /home/pio_build 

WORKDIR /home/pio_build 