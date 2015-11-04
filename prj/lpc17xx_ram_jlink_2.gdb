monitor reg r13 = (0x10000000)
monitor reg pc = (0x10000004)

break ResetHandler
break main
continue
