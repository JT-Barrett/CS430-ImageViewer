# CS430-ImageViewer

This repository is for Computer Graphics Project 5: Simple Image Viewer. The program is meant to take in a PPM P3 or P6 file and open a window in which it can be viewed. It also provides the user the ability to apply affine transformations to the image using the keyboard.

# How to Run the Program

This project compiles on Windows using Visual Studio 15. THIS WILL NOT BUILD USING THE VISUALSTUDIO GUI, BUT RATHER IT'S COMMAND PROMPT. To get to this command prompt, press the windows key and enter "Developer Command Prompt", and the command propmt for your VS instalation should show up.

You will need to download this whole repository and insert all the files into the location of your main .c file in a new Visual Studio Project. Once that is done, you will need to navigate to that directory in the console and call nmake. If nmake is not able to be found navigate to the tools directory of your visual studio installation and call vsvars32.bat (for me this is >cd C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools). Once this is done, navigate back to the project directory and calling nmake will produce an executable and you can simple use the execution pattern "ezview source.ppm".

Here is a guide for using the transformations:
- WASD - Sheer
- Arrow Keys - Translate
- R - Zoom In
- F - Zoom out
- Q & E - Rotate 90 Degrees
