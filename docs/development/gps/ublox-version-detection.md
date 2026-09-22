# u-blox version detection

The UBX-MON-VER hardware string and `PROTVER` extension serve different purposes.
An unrecognized hardware string must not prevent parsing `PROTVER`: protocol
version selects the existing legacy CFG or CFG-VALSET configuration path.
Only complete 30-byte extension fields following the 40-byte MON-VER header are
examined. Protocol versions are parsed as decimal major/minor integers.

Unknown hardware remains `Unknown` in CLI status. This does not imply that the
receiver's advertised UBX protocol is unknown. The navigation rate is taken from
`gps_ublox_nav_hz` when the protocol indicates a modern receiver.

The `gps_ublox_protocol_unittest` target exercises the real UBX byte parser and
configuration state machine with synthetic serial input. Its unknown-hardware
fixture advertises protocol 50.10 and answers CFG-VALSET, but does not acknowledge
legacy CFG commands. It checks that configuration completes and PVT data reaches
the solution callback, without claiming a position fix. Known M8/M10 paths,
malformed/truncated MON-VER data and disabled auto-configuration are also covered.

This fixes a protocol-detection prerequisite for receivers such as ZED-X20P; it
is not a declaration of full device support. Hardware identification, automatic
baud negotiation, UART2 configuration and constellation-specific configuration
on unknown hardware are unchanged. Validate these separately on the receiver.
The reported ZED-X20P workaround disabled auto-config and auto-baud together, so
it did not isolate both paths. No captured ZED-X20P MON-VER packet was available
for the regression fixture.

Reference: [u-blox X20 HPG 2.02 interface description, MON-VER and configuration interface](https://content.u-blox.com/sites/default/files/documents/u-blox-X20-HPG-2.02_InterfaceDescription_UBXDOC-304424225-19967.pdf).
