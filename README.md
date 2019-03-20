# CLOUDS
This project was developed during my master thesis. It renders volumetric clouds in real time, simulates visual properties of clouds and supports scenes with solid objects crossing the cloud layer.
Clouds are rendered using ray marching of different noise textures. Lighting is realized by applying three components: Beer's law, powder effect, Henyey Greenstein function. This work was inspired by the game *Horizon Zero Dawn* and the presentations of *Andrew Schneider*.


**INSTALLATION GUIDE**
* Clone the repository to any location 
* WINDOWS:
  * CMake and Visual Studio must be installed
  * create *bin* directory so you have this structure
* Download the dependencies assimp, GLFW, glm and GLEW using this link
  * Run SetEnvironment.bat
