# Electronic-Code-Lock-Project-Using-FPGA Description

Objective:
To develop an FPGA-driven electronic locking system that enables secure authentication through a numerical password. The system should offer a user operation mode for entering the password and an administrator mode for changing the existing password.

Part 1 - Hardware:

Hardware development revolves around the DE1 board with Altera's Cyclone II FPGA. The hardware design is implemented using SOPC Builder (Platform Designer in newer versions) to integrate the following components:

    CPU (Nios II Processor): The core of the system which will execute the embedded software for lock management.
    CLK_50 (Clock): The main clock for the FPGA synchronizing system operations.
    PLL: To generate additional clock frequencies if needed.
    SDRAM_controller: Manages communication with the SDRAM memory.
    JTAG UART: Enables communication with the FPGA via JTAG port for debugging.
    SYS_id (System ID): A unique identifier for the system.
    Timer: A component to measure time, useful for the one-hour delay after three incorrect attempts.
    KEY (for push buttons): Four buttons for user interaction.
    SW (switches): Potentially used for mode selection (user/administrator).
    HEX (for the 7-segment display): To display the password entered in real-time.
    LEDR (for the red LEDs): Status indicators, for example, to indicate an incorrect password.
    LEDG (for the green LEDs): Status indicators, to indicate a correct password.

The steps in hardware design include importing the components into the Platform Designer, making the necessary connections between them, saving the design in a .qsys file, and generating the VHDL HDL file.

Part 2 - Software:

Using the Nios II EDS tool will enable the development of the embedded software in C that manages interactions between devices and implements access control logic. The main software features will include:

    User Interface: A straightforward interface with four buttons to enter, select, clear digits, and validate the password.
    Dynamic Display: User entries will be shown in real-time on the 7-segment displays.
    State Indication via LEDs: Green and red LEDs will indicate the status of the password entry attempt.
    Security After Failures: A timing mechanism that locks out entry attempts for an hour after three failed attempts.
    Mode Management: The system can operate in two modes, the password entry mode, and the administrator mode for changing the password, with secure access to the administrator mode.

Advanced Features:

    Entry Mode: The default functionality where a user can input a password using the buttons.
    Administrator Mode: Accessible by holding button 1 for 30 seconds and after verifying the current password, allowing secure password modification.

Conclusion:

This electronic locking system offers a secure and flexible solution for FPGA-based access control. It combines an intuitive user interface with advanced security mechanisms to ensure effective protection against unauthorized access. The ability to customize the password and the locking mechanism after incorrect attempts enhances the system's security profile.
