# PG3D-Simple-Injector
Simple DLL file injector made in C++ for the video game Pixel Gun 3D

### WARNING

- This Program is FREE SOFTWARE.
- It will always be available for free, with the source code included.
- All features will be unlocked and there will be no requirements to obtain the program.
- If someone asks you to pay for this program or complete a task to obtain it you are getting scammed!

### DISCLAIMER

This tool is provided as-is without any warranty. The authors or contributors are not responsible for any misuse or damage caused by this tool. It is intended for educational and testing purposes only. Use it responsibly and ethically, ensuring you have appropriate permissions to interact with the target processes.

### USE

When you run the program, the injector will check if the `Pixel Gun 3D.exe` process is running, if not, it will automatically open the video game. Once it detects the process, it will inject the DLL with the specified name (by default it is `Test.dll`), the DLL to be injected must be in the same folder as the injector and must have the name specified by the program, so Otherwise, the console will display an error and exit. If the program finds the specified dll, it will inject it into the process as soon as it is detected to have been executed.

Although it is originally designed for the video game Pixel Gun 3D, you can modify the program to work in other processes. Also, you can specify the name of the DLL to be injected.

### CHANGE DLL NAME

Here you can see the definition of the name of the DLL to inject, within the code you can assign the name of the DLL that you want to inject
  
	#define DLL_NAME "Test.dll"
    

[![Hi.png](https://i.postimg.cc/4dBTsYwg/Hi.png)](https://postimg.cc/XGGPg7TH)

![](https://img.shields.io/github/stars/DariSVR/PG3D-Simple-Injector) ![](https://img.shields.io/github/forks/DariSVR/PG3D-Simple-Injector)  ![](https://img.shields.io/github/issues/DariSVR/PG3D-Simple-Injector)

### End
