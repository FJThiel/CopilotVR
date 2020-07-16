# CopilotVR
## UCL Accessibility Prototype

![Montage showcasing different applications of CopilotVR](/DocumentationPictures/Montage_downsized.jpg?raw=true "Montage showcasing different applications of CopilotVR")

---


## Disclaimer
This implementation is based on the OpenVR overlay example code provided by Valve under the BSD-3 license. The copyright for these files remains with them.

This application also uses library hooks and undocumented functions of the OpenVR library. While it has not happened to our knowledge so far, there is a chance that Valve will penalise players that use these hooks in the future.  
Use at your own risk. We take no responsibility for any actions Valve takes against you that are in relation to this prototype.

This software is also a prototype currently in development. Bugs and crashes are to be expected and it is not feature complete.

## Licencing
This piece of software is published under the MIT Licence unless marked otherwise. For licence notes for the parts of this project that are not covered by this MIT licence, refer to the files in the "LicenceNotices" subdirectory of the repository. 

This includes:
- GLM (Happy Bunny and MIT Licence)
- OpenVR (3-Clause BSD Licence)
- Qt5 (LGPL-V3)

## About CopilotVR
CopilotVR is a prototype application currently developed at the University College London. It hooks into the OpenVR libraries to make arbitary VR games more accessible to players with motion impairments. It does this by allowing a second input device to manipulate the player's position in the virtual 3d space. This second channel could be used by a second player (the copilot) or the original player themselves.

This second input device can be any device that is recognised by SteamVR. It was tested by us using an Xbox gamepad, but other controllers (e.g. the Xbox adaptive controller) should be possible.

To use a device that is not a gamepad, the SteamVR binding interface can be used to map the motions to the new input device.

## Contributing to CopilotVR
Everyone is invited to contribute to this project and help to make VR more accessible. Feel free to make your own improvements and create forks or pull requests. If you don't feel confident in your coding abilities, you can still help by testing hardware and games with CopilotVR. If you do, please let us know what works and what does not.

## Building CopilotVR

#### Requirements
* Visual Studio (I used VS 2017)
* Qt (I used 5.9.8, the MSVC x64 part is enough)
* CMake

#### Known Working Configurations
| Visual Studio |   Qt   | CMake  |
| :-----------: |  :--:  | :----: |
| 2017          | 5.9.8  | 3.12.3 |
| 2017          | 5.12.2 | 3.8.2  |

#### Preparation
1. Clone repository

2. Initialise the submodules  
<code>git submodule init</code>  
<code>git submodule update</code>

#### Build the OpenVRInputEmulator

3. Navigate to this directory  
<code>\<repo\>\CopilotVR\third_party\OpenVR-InputEmulator</code>  
(\<repo> is the location of your repository)



4. Open the VRInputEmulator.sln  
   Note: you may need to retarget the solution

5. Right-click on the project lib_vrinputemulator and select "manage nuget packages". Then install boost-vc141 and all its dependencies
   * NuGet may have issues with long path names. If that happens a viable workaround is temporarily relocating the project and moving it back afterwards

6. Add the following path as an include folder to the project  
<code> ..\\..\openvr\headers</code>

   **Note:** In Visual Studio, include folders are set by right clicking on the project -> Properties -> Configuration Properties -> VC++ Directories -> Include Directories  
   **Important:** You need to do this for both the Debug and the Release configuration. You can switch between them through the drop-down menu at the top of the properties window



7. Build the library by right-clicking the project lib_vrinputemulator and select "build". Do this for both the Release and the Debug configuration.  
   **Note**: The resulting .lib files will be in the following paths respectively:  
   <code>\<repo\>\CopilotVR\third_party\OpenVR-InputEmulator\Debug\lib\x64</code>  
   <code>\<repo\>\CopilotVR\third_party\OpenVR-InputEmulator\Release\lib\x64</code>  
   **Note**: Do not move these files, they will be automatically included later

#### Building CopilotVR itself

8. Open CMake and load the CMake file in  
<code>\<repo\>\CopilotVR</code> 

9. For the automation to work best, set the build directory to  
   <code>\<repo>\CopilotVR\build</code>

10. Configure it with cmake  
   **Important:** Select x64 when asked for a generator platform. Don't leave that field empty!  
   **Note:** The first configure will fail because the Qt includes are missing. We will add them in the next step.
   
11. Add the missing Qt includes by adding the following path in the field Qt5_DIR and press configure again:  
   <code>\<Your QT Path\>\msvc2017_64\lib\cmake\Qt5</code>

12. After a successful configuration, press "generate"

13. Open CopilotVR.sln in the folder  
<code>\<repo\>\CopilotVR\build</code>

14. Install boost-vc141 with nuget in the same way as described in step 5. for the project CopilotVR

15. Build the overlay. The resulting files can be found in  
<code>\<repo\>\CopilotVR\bin\win64</code>

#### Troubleshooting
- Cmake gives me the message "Could not find INPUTEMULATOR"  
This is most likely caused by the cmake script not being able to find the library files generated by building the VrInputEmulator. Make sure that you have actually built it in both release and debug configuration and that the resulting libraries are in the places they are meant to be (see step 7).

- Visual studio claims that it can't find boost though it shows up in Nuget as installed.  
This comes most likely from running cmake again after boost was already installed via Nuget. The only way to fix this that I have found so far is opening the NuGet Console in Visual Studio and entering the following command:  
<code>Get-Package | Uninstall-Package -RemoveDependencies -Force</code>  
This will uninstall all packages so you can reinstall boost manually again.


## Installation

#### Prerequisites:
* SteamVR (obviously)
* Xbox Gamepad (for the start, feel free to try it with other input devices later)

#### Install the OpenVR Input Emulator

1. Download and execute the installer of the OpenVR Input Emulator. The setup can be found on the [GitHub Page of the software](https://github.com/matzman666/OpenVR-InputEmulator/). Note that the most current build of the emulator is broken and will crash SteamVR. We will fix it in the next steps.

2. Navigate into the folder where SteamVR is installed. By default this is:
<code> C:\Program Files (x86)\Steam\steamapps\common\SteamVR</code>

3. Then navigate into the subfolder "drivers". You will find a folder there with the name "00vrinputemulator". Open it and go down to "\bin\win64".

4. There you will find a file with the name "driver_00vrinputemulator.dll". Replace it with the one you can download [here](https://github.com/sharkyh20/OpenVR-InputEmulator/releases/tag/SteamVR-Fix).

5. Test that the OpenVR Input Emulator is installed. For that, just start SteamVR, put on the headset, and press the menu button on one of the controllers. The overlay view will open and in the row, at the bottom, a new icon for the Input Emulator will have appeared if the setup was successful (see image below).

![SteamVR ribbon with the icon for the Input Emulator](/DocumentationPictures/SteamVRRibbon_InputEmulatorIcon.jpeg?raw=true "SteamVR ribbon with the icon for the InputEmulator")

#### Installing CopilotVR

You can store the application wherever you want on your computer. However, I would advise
against network shares or thumb drives. Make sure that all files that are in the folder stay together.

## Using CopilotVR

1. Connect input device

2. Start SteamVR

3. Put on headset
Whenever the application starts, it queries the SteamVR API for the overall offsets created
through the room setup. However, if the headset has not initialised itself yet at this point,
sometimes garbage is returned.

4. Start the application
This is done by executing the "CopilotVR.exe". A command-line should open that gives you feedback about whether it could connect to the Input Emulator and whether it was
able to query the initial matrices from SteamVR.

There is no further feedback. If the window looks like the picture below, the software is running now.

![Command line window showing a successful start of CopilotVR](/DocumentationPictures/CopilotVRCommandLine.PNG?raw=true "Command line window showing a successful start of CopilotVR")

You can close it at any time by closing the command line.

A settings panel for CopilotVR can be found when the SteamVR menu is opened in VR. Depending on the SteamVR version, it will be a button on the bottom or the lower-left corner of the SteamVR overlay.

![Settings panel of CopilotVR](/DocumentationPictures/CopilotVRPanel.jpeg?raw=true "Settings panel of CopilotVR")

## Configuring a New Input Device

By default, the software is set up for an Xbox One gamepad. However, you can build input
mappings for every controller that you can connect to SteamVR.

This is done through the SteamVR Binding UI. You can find it by clicking on the three
horizontal bars on the top left corner of the SteamVR Window, selecting "Devices" and then
"Controller settings". 

![Dialogue path of SteamVR to reach the controller settings option.](/DocumentationPictures/SteamVRDialogue.png?raw=true "Dialogue path of SteamVR to reach the controller settings option.")

This will open the controller settings window. Note that in
the recently updated user interface of SteamVR, the overlay may not show up in the regular binding UI. Instead we advise to use the option "Show old Binding UI". It has a slightly different colour than the other options as shown below.

![SteamVR Controller Settings window.](/DocumentationPictures/SteamVRDeviceSettings.PNG?raw=true "SteamVR Controller Settings window.")

Once the UI is open, click on "Show more Applications" and scroll down the list. If you have ran the application before, one of the entries will be "CopilotVR". If it does not show up, close the window, start the application, and open the window again.

![SteamVR application bindings window.](/DocumentationPictures/SteamVRControllerBinding.PNG?raw=true "SteamVR application bindings window.")

Once you select it, a new window will open (shown below). On the right side of the window you can select what controller you want to configure by clicking on the name of the current controller ("Gamepad" in the picture) and select the desired controller from the list. On the left, you can either edit the current binding (by clicking on "Edit") or create a new binding by clicking on the surface labeled with "Create New Binding".

![SteamVR CopilotVR bindings window.](/DocumentationPictures/SteamVRControllerBindingDevices.PNG?raw=true "SteamVR CopilotVR bindings window.")

Editing or creating a new binding will open a new window looking like the one below. Configuring the binding is a standard process and no different for our overlay as it would be for any SteamVR
game, so for the sake of brevity, I won't go into details here.

![SteamVR CopilotVR mapping window.](/DocumentationPictures/SteamVRControllerBindingMappings.PNG?raw=true "SteamVR CopilotVR mapping window.")

Once you have the mapping configured, saved, and selected, a new file will show up in the
folder where "actions.json" is located. If you ever want to share your mapping with other
partners you can do it via this file. Also, feel free to shoot us a message with what crazy device
you connected and with what mapping!


## Known Issues

### Rotation is completely messed up for controllers.
Yes. We are still working on that. The controller seems to rotate around a different origin than the headset and so far we were not able to determine where that lies. If you managed to get it working, please create a pull request and let us know.

### We closed the application while we were translated and now we are stuck somewhere.  
The software currently does not reset the player when it is closed. A simple restart of SteamVR will put everything right again.

### It crashed and now everything is on fire.  
Happens. It is a hacked-together prototype that will fail at times. Feel free to create an Issue when it does with as many details as you have. I can not promise a quick fix though.