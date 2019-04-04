# CLOUDS
This project was developed during my master thesis. It renders volumetric clouds in real time, simulates visual properties of clouds and supports scenes with solid objects crossing the cloud layer.
Clouds are rendered using ray marching of different noise textures. Lighting is realized by applying three components: Beer's law, powder function, Henyey Greenstein function. This work was inspired by the game *Horizon Zero Dawn* and the presentations of *Andrew Schneider*.


**INSTALLATION GUIDE**
* Download the following dependencies: `Assimp`, `GLFW`, `GLM` and `GLEW` using this [link](https://drive.google.com/open?id=1Yg7Qc6SBZAdagv43I3BRsuyRpjcaYKdc) to an arbitrary location

  Run the script `SetEnvironment.bat` to set the necessary environment variables
* Clone the repository to any location 
* WINDOWS:
  * CMake and Visual Studio must be installed
  * Create *bin* directory in root folder, after that you should have the following structure
  ```bash
  ├── bin
  ├── cmake
  ├── ext
  ├── libs
  ├── res
  └── src
  ```
  * Jump to *bin* directory and run following command. 
  ```
  cmake ..
  ```  
  * This will generate the Visual Studio solution file. You can open this, start coding and compiling.
