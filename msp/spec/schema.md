# The MSP YAML schema

**Spec version 1 (draft)**

## 1. Scope

The MSP YAML schema is a small interface definition language for binary protocols and the
data structures around them. It was written to describe INAV's MSP protocol,
enums, and the types its libraries share, but nothing in the core language is INAV-specific.

YAML is the carrier syntax only. The language is defined here, not by YAML.

The core language has twelve concepts and no more:

package, imports, constants, aliases, enums, bitmasks, structs, fields,
arrays, optional fields, messages, comments.

There is no inheritance, no polymorphism, no discriminated unions, no
ontology, and no root type. What you see in a file is what exists.

### 1.1. Non-goals

The MSP YAML schema deliberately does not include the OCCID IDL features it was derived
from: `parent`, `variants`, implicit discriminator enums, class expansion
files, module manifests, `extend_variants`, ontological ordering, tags and
profiles, and the rule that every file declares a root model. Those are
application semantics, not IDL semantics.

Those features can be reintroduced by a separate layer (`occid-ontology`)
that consumes the MSP YAML schema structs and applies its own parent/variant rules. The
generic compiler knows nothing about it.

---

## 2. YAML subset

- YAML version 1.2
- Allowed: mappings, sequences, plain scalars, double-quoted scalars, comments
- Forbidden: anchors, aliases, YAML tags, flow-style collections, duplicate
  keys, tabs
- Unknown keys are errors
- String literals in schema documents are double-quoted
- Extra indentation levels are legal for readability where they do not change
  the parsed structure

---

## 3. Document

A document is a single YAML file with this header:

```yaml
version: 1
package: inav.gps
```

Both keys are required. Optionally:

```yaml
imports:
  - inav.units
  - inav.geo
```

Followed by any of the section keys, in any order, all optional:

`constants`, `aliases`, `enums`, `bitmasks`, `structs`, `messages`

A file may declare nothing but a header. A file may declare only messages, or
only enums. There is no required section and no primary type.

### 3.1. Packages

- Package regex: `^[a-z][a-z0-9_]*(\.[a-z][a-z0-9_]*)*$`
- A package must be unique across all loaded documents
- The filename does not have to match the package, and the directory layout
  carries no meaning. Both are organizational only.

### 3.2. Imports

An import names a package, never a file path. How a package name resolves to a
file is up to the compiler's search path and is outside this spec.

A name from an imported package is referenced by its bare name. If two
imported packages export the same bare name, the reference is ambiguous and
must be written fully qualified as `package.Name`. Ambiguity that is never
referenced is not an error.

Imports are not transitive: importing `inav.geo` does not bring in what
`inav.geo` itself imports. Import cycles are an error.

---

## 4. Names

- Identifier regex: `^[A-Za-z_][A-Za-z0-9_]*$`
- Type names — structs, enums, bitmasks, aliases — share one namespace per
  package and must be unique within it
- Constants have their own namespace, as do messages
- Field names must be unique within their struct or payload

Conventions, not enforced: `PascalCase` for types, `snake_case` for fields,
`SCREAMING_SNAKE_CASE` for constants and enum members, `MSP_LIKE_NAMES` for
messages.

---

## 5. Primitive types

```
bool
int8    int16    int32    int64
uint8   uint16   uint32   uint64
float32 float64
char    cstring
```

`char` is an 8-bit character intended for text. It is distinct from `uint8` so
that generators can emit `char` rather than `uint8_t` and so that `char[]`
reads as a string.

`cstring` is a NUL-terminated character sequence. It is self-delimiting: it
carries its own length on the wire, so unlike `char[]` it may appear anywhere
in a payload rather than only at the end, and it takes no array size. INAV uses
both forms — `MSP2_COMMON_SETTING_INFO` opens with a NUL-terminated name and
then continues with eleven more fields.

There is no `string` type. A string is `char[N]` when it is fixed-width,
`char[len_field]` when a preceding field gives its length, and `char[]` when it
runs to the end of the payload. Adding `string` would require this spec to
mandate an encoding and a length prefix; the wire formats the MSP YAML schema describes do
not agree on either.

There is no `bytes`, `any`, `list`, `map`, or `tuple`. Use `uint8[]` for opaque
bytes.

Everything is little-endian and unpadded unless a backend documents otherwise.
Structs are flattened on the wire — nesting is an authoring convenience, not a
layout construct.

---

## 6. Type expressions

| Form | Meaning |
| --- | --- |
| `T` | a primitive, alias, enum, bitmask, or struct |
| `T[N]` | fixed array of `N` elements |
| `T[COUNT]` | array sized by a declared constant |
| `T[field_name]` | array sized by the value of a preceding field |
| `T[]` | array running to the end of the payload |
| `optional T` | see section 11 |
| `package.T` | a fully qualified name (section 3.2) |

Rules:

- Arrays are one-dimensional. `T[4][4]` is not valid; declare a struct.
- `N` is an integer literal greater than zero.
- `COUNT` must name a declared integer constant with a positive value.
- `field_name` must name an unsigned integer field earlier in the same struct
  or payload. Forward references are an error.
- `T[]` may only appear on the last field of a payload, and only inside a
  message payload — a struct with an open-ended tail could not be nested
  safely.
- `optional` may combine with any array form.
- `cstring` takes no array size; it is already variable-length.

---

## 7. Constants

```yaml
constants:
  MAX_SUPPORTED_MOTORS: uint8 = 18
  BUILD_DATE_LENGTH: uint8 = 11
  NAV_MAX_WAYPOINTS: uint16 = 120
```

Expanded form, when a constant needs prose:

```yaml
constants:
  NAV_MAX_WAYPOINTS:
    type: uint16
    value: 120
    description: "Maximum waypoints storable in a mission."
```

Rules:

- The value is a literal: integer, float, or bool. Matching the declared type
  is required.
- Expressions are not supported in version 1. `LED_MODE_COUNT * 4 + 2` must be
  written as its computed value. This is a deliberate limitation — see
  section 16.
- Constants are usable as array sizes and as `repeat` counts.

A constant whose value is fixed by build configuration rather than by the
schema declares `configurable` instead of `value`, naming the build symbol that
supplies it:

```yaml
constants:
  MAX_SUPPORTED_MOTORS:
    type: uint8
    configurable: TARGET_MOTOR_COUNT
    description: "Per-target motor count."
```

`configurable` and `value` are mutually exclusive. A configurable constant may
be used as an array size or repeat count, but generation fails unless a value
is supplied for it, so a wrong layout cannot be emitted silently.

---

## 8. Aliases

```yaml
aliases:
  timeMs_t: uint32
  gpsCoordinate_t: int32
  boxBitmask_t: uint64
```

An alias target must be a primitive or another alias. Aliasing a struct, enum,
or bitmask is an error, as is an alias cycle.

An alias is a name for a representation, not a distinct type: a `timeMs_t`
field and a `uint32` field are interchangeable. Backends may emit a real
typedef or collapse the alias to its primitive.

---

## 9. Enums and bitmasks

### 9.1. Enums

An enum is a set of mutually exclusive values.

```yaml
enums:
  GpsFix:
    prefix: GPS_FIX
    storage: uint8
    description: "GPS fix quality."
    values:
      - NONE = 0
      - TWO_D = 1
      - THREE_D = 2
```

- `storage` is required and must be an integer primitive
- `prefix` is optional; when present it is prepended to each member name with
  an underscore in generated output
- A member is `NAME`, `NAME = INT`, or `NAME = EARLIER_MEMBER`
- The first unassigned member is `0`; later unassigned members increment from
  the previous member
- `NAME = EARLIER_MEMBER` declares an alias and must reference a member
  declared before it. Forward references are an error.
- Member names must be unique within the enum, and every value must fit in
  `storage`
- Values need **not** be unique. Two members may independently carry the same
  value, which C enums do routinely: INAV declares `SERVO_FLAPPERON_2 = 4` and
  `SERVO_BICOPTER_LEFT = 4` because they are the same output index named for
  different airframes. A decoder resolving a value back to a name takes the
  first member that declares it.

### 9.2. Bitmasks

A bitmask is a set of independently settable flags. It is declared
explicitly — no enum is ever silently reinterpreted as flags.

```yaml
bitmasks:
  SensorFlags:
    prefix: SENSOR
    storage: uint16
    values:
      - ACC = 0
      - BARO = 1
      - MAG = 2
      - GPS = 3
```

Values are **bit positions**, not pre-shifted masks. A position must be unique
and less than the bit width of `storage`. Unlike an enum, a bitmask may not
repeat a position — two names for one flag are indistinguishable on decode.

Unassigned members take the next free position, starting at `0`.

Because every member is a position, no member can express "no flags set" —
position `0` already means bit 0. A bitmask that needs a named empty value
declares it with `zero`:

```yaml
bitmasks:
  BootEventFlags:
    prefix: BOOT_EVENT_FLAGS
    storage: uint16
    zero: NONE
    values:
      - WARNING = 0
      - ERROR = 1
```

`zero` names a member emitted with the value `0`. It is optional, valid only on
a bitmask, and its name must not collide with a flag.

---

## 10. Structs and fields

```yaml
structs:
  GeoLocation:
    description: "WGS-84 position."
    fields:
      latitude:
        type: int32
        unit: degrees_e7
      longitude:
        type: int32
        unit: degrees_e7
      altitude:
        type: int32
        unit: centimeters

  GpsSolution:
    fields:
      fix: GpsFix
      satellites: uint8
      location: GeoLocation
      ground_speed: uint16
      ground_course: uint16
      hdop: uint16
```

`description` is optional everywhere. `fields` is required and must be
non-empty — an empty struct has no representation.

Field order is declaration order and is significant: it is the wire order.

### 10.1. Shorthand and expanded field forms

Shorthand is `name: TypeExpression` and is preferred when the field needs
nothing else:

```yaml
fix: GpsFix
satellites: uint8
target_name: char[32]
```

Expanded form is used when a field carries metadata:

```yaml
pwm:
  type: uint16
  unit: microseconds
  description: "Output pulse width."
```

Allowed keys in expanded form:

| Key | Required | Meaning |
| --- | --- | --- |
| `type` | yes | a type expression |
| `unit` | no | free-text unit label |
| `description` | no | free text |
| `value` | no | fixed wire value (see below) |

A field is either a plain field, which has `type`, or a repeated group, which
has `repeat` and `fields` instead (section 12). No field has both.

`unit` has no meaning to the compiler. It is carried through to generated
output as documentation. Suggested labels: `degrees_e7`, `centimeters`,
`meters`, `centimeters_per_second`, `microseconds`, `milliseconds`, `hz`,
`degrees`, `decidegrees`, `percent`, `millivolts`, `centiamps`.

`value` marks a field whose wire value is fixed — a reserved or legacy byte
that must be transmitted as a specific constant. INAV has 63 of them. Encoders
emit it without asking the caller; decoders may verify it. It is not a default:
the field always occupies its space on the wire and callers cannot override it.

There is no `required` key; a field is required unless marked `optional`.
There is no `default` and no `const`. the MSP YAML schema describes wire layout, not
configuration policy.

---

## 11. Optional fields

`optional T` means the field may be absent from the payload.

There is no presence bitmap and no tag on the wire, so presence is positional:

- Optional fields must form a contiguous suffix of their struct or payload. An
  optional field followed by a required field is an error.
- If an optional field is present, every optional field before it is present.
- A decoder stops when the payload is exhausted; every optional field not
  reached is absent.

This is exactly how a protocol grows: new fields are appended to the end of an
existing message and older peers simply stop early.

A trailing `T[]` array may follow optional fields. Because the optional fields
are fixed width, a decoder consumes them in order while bytes remain and the
array takes whatever is left. This is ambiguous if a payload arrives with a
partial optional run, so a decoder should treat a short remainder as an error
rather than guess. `MSP_SIMULATOR` relies on this shape.

---

## 12. Repeated groups

A group of fields that repeats as a unit:

```yaml
structs:
  ServoMixRules:
    fields:
      rules:
        repeat: MAX_SERVO_RULES
        fields:
          target_channel: uint8
          input_source: uint8
          rate: int16
          speed: uint8
```

`repeat` accepts:

| Value | Meaning |
| --- | --- |
| integer literal | that many iterations |
| constant name | the constant's value |
| preceding field name | the value of that unsigned integer field |
| `until_end` | repeat until the payload is exhausted |

Rules:

- A repeated group has `repeat` and `fields` and no `type`
- Groups may not nest, and `until_end` may only appear on the last group in a
  payload
- `until_end` is only valid inside a message payload, for the same reason as
  `T[]`

`repeat` may also be set directly on a message payload, repeating the whole
payload:

```yaml
messages:
  MSP_MOTOR_MIXER:
    id: 0x1005
    request: null
    reply:
      repeat: MAX_SUPPORTED_MOTORS
      fields:
        throttle: uint16
        roll: uint16
        pitch: uint16
        yaw: uint16
```

---

## 13. Messages

A message is a protocol operation: an identifier plus an optional request
payload and an optional reply payload.

```yaml
messages:
  MSP_FC_VERSION:
    id: 3
    description: "Firmware version."
    request: null
    reply:
      fields:
        major: uint8
        minor: uint8
        patch: uint8
```

Keys:

| Key | Required | Meaning |
| --- | --- | --- |
| `id` | yes | integer identifier, unique within the package |
| `request` | yes | payload or `null` |
| `reply` | yes | payload or `null` |
| `description` | no | free text |

`request` and `reply` are written explicitly, `null` included, so that a
message with no payload is distinguishable from an unfinished declaration.

A payload is a mapping with `fields`, optionally `repeat` (section 12), and
optionally `description`. Payload fields follow section 10 in full.

Payloads are declared inline rather than referencing a named struct. Most
payloads are used exactly once and naming them would invent hundreds of
single-use type names. Where a shape genuinely is shared, declare a struct and
use it as a field type:

```yaml
  MSP2_INAV_ADSB_VEHICLE:
    id: 0x2090
    request: null
    reply:
      repeat: until_end
      fields:
        vehicle: AdsbVehicle
```

---

## 14. Comments

YAML `#` comments are permitted anywhere YAML permits them and have no
meaning to the compiler. Use `description` for text that should survive into
generated output.

---

## 15. Errors

The following are always errors:

**Document**
- missing `version` or `package`
- malformed package name
- duplicate package across loaded documents
- unknown top-level key
- unknown key inside any section
- import cycle
- unresolved import
- ambiguous bare name that is actually referenced

**Names**
- identifier not matching the identifier regex
- duplicate type name within a package
- duplicate field name within a struct or payload
- duplicate constant or message name
- duplicate message `id` within a package
- reference to an undeclared type, constant, or field

**Types**
- invalid type expression
- multidimensional array
- `T[N]` where `N` is not a positive integer literal
- `T[COUNT]` where `COUNT` is not a positive integer constant
- `T[field]` where the field does not precede it, or is not an unsigned integer
- `T[]` outside the final position of a message payload

**Aliases, enums, bitmasks**
- alias targeting a struct, enum, or bitmask
- alias cycle
- missing or non-integer `storage`
- enum value that does not fit in `storage`
- enum alias referencing a member that is not declared earlier
- duplicate bit position within a bitmask
- bit position greater than or equal to the width of `storage`
- `zero` on an enum rather than a bitmask, or colliding with a flag name
- a constant declaring both `configurable` and `value`, or neither

**Structs and fields**
- `fields` missing or empty
- expanded field without `type`
- `required`, `default`, or `const` keys
- optional field followed by a required field
- `cstring` given an array size

**Repeated groups**
- group with both `repeat` and `type`
- group without `fields`
- nested repeated group
- `until_end` outside the final position of a message payload

**Messages**
- missing `id`, `request`, or `reply`
- payload that is neither `null` nor a mapping with `fields`

---

## 16. Known gaps

Recorded deliberately, so they read as decisions rather than oversights.

**Constant expressions.** INAV sizes at least one payload by
`LED_MODE_COUNT * LED_DIRECTION_COUNT + LED_SPECIAL_COLOR_COUNT`. Version 1
requires that be authored as a literal. Building an expression evaluator is a
step toward reimplementing the C preprocessor, and is not worth it until
several more cases appear.

**Untyped fields.** A small number of MSP payloads carry data with no fixed C
type. Model these as `uint8[]` and describe the real shape in `description`.
There is no `any`.

**Deprecation metadata.** INAV tracks replaced and unimplemented commands.
The MSP YAML schema has no `deprecated` or `replaced_by` key. If it is added later it
belongs on `messages` and should not affect wire layout.

**Conditional fields.** Fields that exist only under a build flag are not
expressible. Every declaration is unconditional.

**Length-dispatched messages.** A handful of MSP commands select a payload
shape from the received length. Where the shapes are successive prefixes of
one another — `MSP_SET_VTX_CONFIG` has seven, each extending the last — the
optional-tail rule in section 11 expresses them in a single declaration, which
is what that rule is for. Where the shapes genuinely differ, as in
`MSP_SET_OSD_CONFIG` (`selector, video_system, …` at one length versus
`item_index, item_position` at another), the MSP YAML schema cannot express them: they are
two messages sharing one identifier, and section 15 requires identifiers be
unique. Author the dominant shape and describe the other in `description`.
Resolving this properly means a variant construct on `messages`, which is a
version 2 question.

**Endianness and alignment** are fixed by the backend, not declarable.

---

## 17. Generated C

Illustrative, not normative. Backends decide their own naming.

```yaml
version: 1
package: inav.gps

aliases:
  gpsCoordinate_t: int32

enums:
  GpsFix:
    prefix: GPS_FIX
    storage: uint8
    values:
      - NONE = 0
      - TWO_D = 1
      - THREE_D = 2

bitmasks:
  SensorFlags:
    prefix: SENSOR
    storage: uint16
    values:
      - ACC = 0
      - BARO = 1
      - MAG = 2
      - GPS = 3
```

```c
typedef int32_t gpsCoordinate_t;

typedef enum {
    GPS_FIX_NONE = 0,
    GPS_FIX_TWO_D = 1,
    GPS_FIX_THREE_D = 2,
} gpsFix_e;

typedef uint16_t sensorFlags_t;

enum {
    SENSOR_ACC  = 1U << 0,
    SENSOR_BARO = 1U << 1,
    SENSOR_MAG  = 1U << 2,
    SENSOR_GPS  = 1U << 3,
};
```

A struct maps to a packed C struct in declaration order; a fixed array to a C
array; a counted or open-ended array to a pointer plus a length the caller
supplies. Aliases may be emitted as typedefs or collapsed to their primitive.
