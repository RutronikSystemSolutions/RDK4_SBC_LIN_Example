# RDK4 TLE9262-3BQX LIN Example

Rutronik Development Kit 4 Programmable System-on-Chip CY8C4149AZE-S598 " System Basis Chip Local Interconnect Network" Code Example. 

This code example demonstrates a LIN 2.2 ISO17987 connectivity using a TLE9262-3BQX System Basis Chip. 

This code example project has derived from the "[PSoC™ 4: Basic LIN slave example in MTB](https://github.com/Infineon/mtb-example-psoc4-lin-slave)". 

<img src="images/lin_test.jpg" style="zoom:25%;" />

## Requirements

- [ModusToolbox™ software](https://www.cypress.com/products/modustoolbox-software-environment) v2025.4.0
- The latest hardware release is [RDK4 Rev2](https://github.com/RutronikSystemSolutions/RDK4_Hardware_Files).

## Supported toolchains (make variable 'TOOLCHAIN')

- GNU Arm&reg; Embedded Compiler v11.3.1 (`GCC_ARM`) - Default value of `TOOLCHAIN`

## Using the code example

Create the project and open it using one of the following:

<details><summary><b>In Eclipse IDE for ModusToolbox&trade; software</b></summary>





1. Click the **New Application** link in the **Quick Panel** (or, use **File** > **New** > **ModusToolbox&trade; Application**). This launches the [Project Creator](https://www.infineon.com/ModusToolboxProjectCreator) tool.

2. Pick a kit supported by the code example from the list shown in the **Project Creator - Choose Board Support Package (BSP)** dialog.

   When you select a supported kit, the example is reconfigured automatically to work with the kit. To work with a different supported kit later, use the [Library Manager](https://www.infineon.com/ModusToolboxLibraryManager) to choose the BSP for the supported kit. You can use the Library Manager to select or update the BSP and firmware libraries used in this application. To access the Library Manager, click the link from the **Quick Panel**.

   You can also just start the application creation process again and select a different kit.

   If you want to use the application for a kit not listed here, you may need to update the source files. If the kit does not have the required resources, the application may not work.

3. In the **Project Creator - Select Application** dialog, choose the example by enabling the checkbox.

4. (Optional) Change the suggested **New Application Name**.

5. The **Application(s) Root Path** defaults to the Eclipse workspace which is usually the desired location for the application. If you want to store the application in a different location, you can change the *Application(s) Root Path* value. Applications that share libraries should be in the same root path.

6. Click **Create** to complete the application creation process.

For more details, see the [Eclipse IDE for ModusToolbox&trade; software user guide](https://www.infineon.com/MTBEclipseIDEUserGuide) (locally available at *{ModusToolbox&trade; software install directory}/docs_{version}/mt_ide_user_guide.pdf*).

</details>

### Operation

The third-party software "[LIN Serial Analyzer v3.0.0](https://www.microchip.com/en-us/software-library/lin_analyzer)" and the PC adapter "[APGDT001 LIN SERIAL ANALYZER](https://www.microchip.com/en-us/development-tool/APGDT001)" were used to demonstrate the RDK4 LIN Slave.

The software must be configured to work with a 19.2 Kbit/s baud rate, which is the maximum baud rate of the LIN 2.2.

<img src="images/LIN_Analyzer_Configuration.png" style="zoom:100%;" />

Please add the frames as it is shown below and set the "Enhanced Checksum Type".

```
10 00 FF FF FF FF FF FF FF
10 11 FF FF FF FF FF FF FF
10 22 FF FF FF FF FF FF FF
10 33 FF FF FF FF FF FF FF
11
```

<img src="images/LIN_Serial_Analyzer_Debug_Tool.png" style="zoom:100%;" />

If the firmware is already programmed into the RDK4 and LIN is connected to the APGDT001 adapter as well as the "Battery" power supply, the commands can be transmitted by pressing "Send Once or Send Continuous".

The first bite controls the RDK4 LEDs as it is shown below:

| Command | Slave response            |
| ------- | ------------------------- |
| 0x11    | Turns on LED1 (Red LED)   |
| 0x22    | Turns on LED2 (Green LED) |
| 0x33    | Turns on LED3 (Blue LED)  |
| 0x00    | Turns off all LEDs        |

The LIN configuration in ModusToolbox IDE is done using a tool called "LIN Configurator". 

The LIN baud rate and protocol are configured as shown in the figure below:

<img src="images/LIN_Configurator_General.png" style="zoom:100%;" />

The LIN Frames are configured as it is shown in the figure below:

<img src="images/LIN_Configurator_Frames.png" style="zoom:100%;" />

The LIN Signals are configured as it is shown in the figure below:

<img src="images/LIN_Configurator_Signals.png" style="zoom:100%;" />

The LIN wire and LIN RX signals are shown in figures below:

<img src="images/lin_wire.png" style="zoom:100%;" />

<img src="images/lin_rx.png" style="zoom:100%;" />



#### SBC Development Mode

A special mode, called SBC Development Mode, is available during software development or debugging of the system. The watchdog counter is stopped and does not need to be triggered. This mode can be accessed by setting the TEST [**FO3**] pin to GND during SBC Init Mode.

## Legal Disclaimer

The evaluation board including the software is for testing purposes only and, because it has limited functions and limited resilience, is not suitable for permanent use under real conditions. If the evaluation board is nevertheless used under real conditions, this is done at one’s responsibility; any liability of Rutronik is insofar excluded. 

<img src="images/rutronik_origin_kaunas.png" style="zoom:50%;" />



