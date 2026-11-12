# INAV MSP

INAV's MultiWii Serial Protocol is defined once, in `schema/msp_v2.yaml`. The firmware's MSP headers and wire structs, the client libraries and the reference documentation are all generated from it by `build.sh`. Enums and constants are not copied into the schema: they are harvested from INAV's C source, so the generated code always matches the firmware it is built with.

## Libraries

| Language | Name | Folder |
| --- | --- | --- |
| C | `inav_msp` headers (`msp_protocol*.h`, `msp_msgs.h`, `msp_enums.h`, `msp_consts.h`); the firmware compiles against these | [`c/`](c/) |
| Python | `inav-msp` (import `inav_msp`) | [`python/`](python/) |
| Arduino / PlatformIO | INAV MSP (`#include <inav_msp.h>`) | [`arduino/`](arduino/) |
| JavaScript | `inav-msp` | [`js/`](js/) |

The message reference is generated into [`docs/development/msp/README.md`](../docs/development/msp/README.md), with the enums in `enums.md`.

## Layout

- `schema/msp_v2.yaml`: the hand-written wire schema. `schema/enums.yaml` and `constants.yaml` are generated.
- `spec/schema.md`: the schema language.
- `generator/`: the harvesters (`harvest_enums.py`, `harvest_consts.py`, `extract_wire_types.py`), the per-language generators (`gen_c.py`, `gen_python.py`, `gen_pydantic.py`, `gen_js.py`, `gen_docs.py`), their templates, and the schema conformance suite.
- `c/`, `python/`, `arduino/`, `js/`: the libraries. Each keeps its generated code apart from its hand-written runtime (`generated/` folders, or the generated C headers).

## Changing the protocol

1. Edit `schema/msp_v2.yaml`, or the C source for an enum or constant.
2. Run `./msp/build.sh generate` from the repository root.
3. For a new message, write its handler in `src/main/fc/fc_msp.c` against the generated struct, using the helpers in `src/main/msp/msp_handler.h`.
4. Commit the schema and the regenerated files together.

## Build and test

```bash
./msp/build.sh generate                 # regenerate everything from the schema and the source
./msp/build.sh test                     # C compile test, schema conformance, Python tests, JS against Python
./msp/build.sh test --with-arduino      # also build the Arduino example with PlatformIO
./msp/build.sh release                  # generate, then test
```

`test` needs Python 3 with PyYAML, `gcc` and `node`. CI (`.github/workflows/msp.yml`) runs `generate`, fails if any generated file differs from the committed one, then runs `test`.
