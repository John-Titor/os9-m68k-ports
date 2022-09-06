# Drivers for 16x5x-series UARTS

## Building

The COM driver and descriptors are intended to be built as part of building a 
port. Configuruation information for the descriptor is obtained from the port's
`systype.d` file, which should instantiate one or more COMx macros in the usual
fashion.

    COMx macro
        SCFDesc <baseaddr>,<vector>,<level>,<priority>,<parity>,<speed>,<driver>
        endm

x			0..3 depending on the port number
basaddr		Base address of the port's registers.
vector		Interrupt vector handling port interrupts.
level		Interrupt level associated with the vector.
priority	Relative interrupt priority for the vector.
parity		PD_PAR value encoding parity, stop bits, word size (n81 = $00)
speed		PD_BAU value encoding default line speed
driver		Name of the associated driver, COM_yyy where yyy is 59x
