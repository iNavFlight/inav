This directory contains the ChibiOS/RT "official" bindings with the RelianceEdge
library by Datalight : https://github.com/datalightinc/reliance-edge

In order to use Reliance Edge within ChibiOS/RT project:
1. unzip Reliance Edge under $(CHIBIOS)/ext/reliance-edge
2. include $(CHIBIOS)/os/various/reledge_bindings/SAMA5D2x/reledge.mk in your makefile.
3. Add $(RELEDGESRC) to $(CSRC)
4. Add $(RELEDGEINC) to $(INCDIR)

Note:
1. These files modified for use with version 2.2 of reliance edge

