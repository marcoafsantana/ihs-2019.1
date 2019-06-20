# GuitarHeroAltera
GuitarHeroAltera is a project that adapt the game GuitarHero for Altera FPGA.

## Requirements
Linux Distro
SDL2 & SDL2_mixer C libraries
Arduino (with firmware.io code)
DE2I-150 Altera (with driver/altera_driver.c PCI driver)

## Compiling & runing
enter in the project folder and run (you will need super user permission):

 ```bash
 cd driver
 make
 ./init_driver.sh
 cd ..
 make
 sudo ./app
 ```

## Group
the creators of this project are the following Computer Engineering students:

* Pedro Henrique Silva (email: phos@cin.ufpe.br)
* Bianca Iz Sarabia (email: biols@cin.ufpe.br)
* Mateus Cavalcanti (email: mcs8@cin.ufpe.br)
* Lucas Dias (email: ldm@cin.ufpe.br)
* Marco Santana (email: mafs3@cin.ufpe.br)