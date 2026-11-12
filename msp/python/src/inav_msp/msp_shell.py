#!/usr/bin/env python3
"""
Interactive MSP API shell.

Examples:
  python msp_shell.py --tcp-endpoint 127.0.0.1:5760
  python -m inav_msp.msp_shell --tcp 127.0.0.1:5760
  python msp_shell.py --udp-endpoint 127.0.0.1:27072 --force-mspv2
  > get_raw_gps
  > set_flight_axis_angle_override pitch_deg=10
  > set_altitude_target altitude_datum=1 altitude_m=50
"""

import argparse
import ast
import enum
import inspect
import sys
import traceback
from typing import Any, Optional, get_args, get_origin, get_type_hints

try:
    import readline  # type: ignore  # noqa: F401
except ImportError:
    pass

from inav_msp.msp_api import MSPApi
from inav_msp.utils import format_nested_dict


class ShellInputError(ValueError):
    pass


def _parse_value(raw: str) -> Any:
    try:
        return ast.literal_eval(raw)
    except (ValueError, SyntaxError):
        keywords = {"true": True, "false": False, "none": None, "null": None}
        return keywords.get(raw.lower(), raw)


def _parse_arg_token(token: str) -> tuple[str, Any]:
    if "=" not in token:
        raise ShellInputError(f"Argument '{token}' is missing '='")
    key, raw = token.split("=", 1)
    if not key:
        raise ShellInputError(f"Argument '{token}' has empty key")
    return key, _parse_value(raw)


def _render_result(cmd: str, result: Any) -> None:
    if result is None:
        print(f"{cmd}: OK")
        return
    if isinstance(result, (dict, list, tuple)):
        print(f"{cmd}:\n{format_nested_dict(result, start_indent=1)}")
        return
    print(f"{cmd}: {format_nested_dict(result)}")


def _list_commands(api: MSPApi) -> list[str]:
    names = []
    for name in dir(api):
        if name.startswith("_"):
            continue
        if callable(getattr(api, name)):
            names.append(name)
    return sorted(names)


def _signature_help(func) -> list[str]:
    sig = inspect.signature(func)
    params = []
    for name, param in sig.parameters.items():
        if param.kind in (inspect.Parameter.VAR_POSITIONAL, inspect.Parameter.VAR_KEYWORD):
            continue
        params.append(name)
    return params


def _enum_type(annotation: Any) -> Optional[type[enum.IntEnum]]:
    origin = get_origin(annotation)
    candidates = (annotation,) if origin is None else get_args(annotation)
    for candidate in candidates:
        if isinstance(candidate, type) and issubclass(candidate, enum.IntEnum):
            return candidate
    return None


def _coerce_value(value: Any, annotation: Any) -> Any:
    enum_type = _enum_type(annotation)
    if enum_type is not None and isinstance(value, str):
        return enum_type[value.rsplit(".", 1)[-1]]
    if annotation is bool:
        if isinstance(value, bool):
            return value
        if value in (0, 1):
            return bool(value)
        raise ShellInputError(f"Boolean arguments must be true/false or 1/0, got {value!r}")
    return value


def main(argv: Optional[list[str]] = None) -> int:
    ap = argparse.ArgumentParser(description="Interactive shell for MSPApi")
    ap.add_argument("--tcp", "--tcp-endpoint", dest="tcp_endpoint", help="MSP TCP endpoint in HOST:PORT format")
    ap.add_argument("--udp", "--udp-endpoint", dest="udp_endpoint", help="MSP UDP endpoint in HOST:PORT format")
    ap.add_argument("--port", help="Serial MSP port")
    ap.add_argument("--baudrate", type=int, default=115200, help="Baudrate for serial MSP")
    ap.add_argument("--force-mspv2", action="store_true", help="Force MSP v2 framing")
    ap.add_argument("--read-timeout", type=float, default=1.0, help="Transport read timeout in milliseconds")
    ap.add_argument("--write-timeout", type=float, default=1.0, help="Transport write timeout in milliseconds")
    ap.add_argument("--max-retries", type=int, default=1, help="Total request attempts; keep at 1 for write commands")
    args = ap.parse_args(argv)

    init_kwargs: dict[str, Any] = {}
    if args.tcp_endpoint and args.udp_endpoint:
        ap.error("Provide only one of --tcp-endpoint or --udp-endpoint")
    if args.tcp_endpoint:
        init_kwargs["tcp_endpoint"] = args.tcp_endpoint
        init_kwargs["port"] = None
    elif args.udp_endpoint:
        init_kwargs["udp_endpoint"] = args.udp_endpoint
        init_kwargs["port"] = None
    else:
        if not args.port:
            ap.error("Provide --tcp-endpoint, --udp-endpoint or --port")
        init_kwargs["port"] = args.port
        init_kwargs["baudrate"] = args.baudrate
    init_kwargs["force_msp_v2"] = args.force_mspv2
    init_kwargs["read_timeout_ms"] = args.read_timeout
    init_kwargs["write_timeout_ms"] = args.write_timeout
    init_kwargs["max_retries"] = args.max_retries

    api = MSPApi(**init_kwargs)
    api.open()

    print("MSP shell. Type 'help' for commands, 'quit' to exit.")
    commands = _list_commands(api)

    try:
        while True:
            try:
                line = input("> ")
            except KeyboardInterrupt:
                print()
                break
            except EOFError:
                print()
                break
            text = line.strip()
            if not text:
                continue
            if text in ("quit", "exit"):
                break
            if text == "help":
                print("Commands:")
                for name in commands:
                    print(f"  {name}{inspect.signature(getattr(api, name))}")
                continue

            parts = text.split()
            cmd = parts[0]
            if cmd not in commands:
                print(f"Unknown command '{cmd}'. Type 'help' for list.")
                continue
            func = getattr(api, cmd)
            if len(parts) == 2 and parts[1] == "help":
                params = _signature_help(func)
                hints = get_type_hints(func)
                print(f"Usage: {cmd}{inspect.signature(func)}")
                for name in params:
                    annotation = hints.get(name, inspect.signature(func).parameters[name].annotation)
                    enum_type = _enum_type(annotation)
                    if enum_type is not None:
                        members = ", ".join(f"{member.name}={member.value}" for member in enum_type)
                        print(f"  {name}: {members}")
                continue

            try:
                kwargs = {}
                positional = []
                for token in parts[1:]:
                    if "=" in token:
                        key, value = _parse_arg_token(token)
                        if key in kwargs:
                            raise ShellInputError(f"Argument '{key}' provided multiple times")
                        kwargs[key] = value
                    else:
                        positional.append(token)

                params = _signature_help(func)
                if cmd in ("set_flight_axis_angle_override", "set_flight_axis_rate_override"):
                    if positional:
                        raise ShellInputError(
                            "Use named axes for overrides (angle: roll_deg/pitch_deg/yaw_deg, "
                            "rate: roll_dps/pitch_dps/yaw_dps)."
                        )
                    if not kwargs:
                        print("No axes provided -> sending empty mask to disable all overrides.")

                if len(positional) > len(params):
                    raise ShellInputError(f"{cmd} expects at most {len(params)} arguments")
                for index, raw in enumerate(positional):
                    key = params[index]
                    if key in kwargs:
                        raise ShellInputError(f"Argument '{key}' provided both positionally and by name")
                    kwargs[key] = _parse_value(raw)

                signature = inspect.signature(func)
                unknown = set(kwargs) - set(signature.parameters)
                if unknown:
                    raise ShellInputError(f"{cmd} has no argument named '{sorted(unknown)[0]}'")
                hints = get_type_hints(func)
                kwargs = {
                    key: _coerce_value(value, hints.get(key, signature.parameters[key].annotation))
                    for key, value in kwargs.items()
                }
            except (ShellInputError, KeyError) as exc:
                print(f"Input error: {exc}")
                continue

            previous_info = api.info
            try:
                result = func(**kwargs)
                _render_result(cmd, result)
                if api.info is not previous_info and api.info:
                    latency = api.info.get("latency_ms")
                    if latency is not None:
                        print(f"latency={latency:.2f}ms, attempt={api.info.get('attempt')}, transport={api.info.get('transport')}")
            except KeyboardInterrupt:
                print("\nInterrupted.")
                break
            except Exception:
                traceback.print_exc()
    finally:
        api.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
