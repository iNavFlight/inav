"""
Example: Using messages without convenience methods.

This demonstrates how to access MSP2_INAV_LOGIC_CONDITIONS_SINGLE,
which doesn't have a pre-built convenience method.

The pattern shown here works for ANY of the 249 MSP messages.
"""

import argparse
from inav_msp import MSPApi, InavMSP
from inav_msp.lib import InavEnums


def parse_args():
    parser = argparse.ArgumentParser(description="Logic Conditions example")
    parser.add_argument("--port", default="/dev/ttyACM0", help="Serial port")
    parser.add_argument("--baudrate", type=int, default=115200, help="Baudrate")
    parser.add_argument("--tcp", help="TCP endpoint (e.g., localhost:9000)")
    parser.add_argument("--index", type=int, default=0, help="Condition index to fetch")
    return parser.parse_args()


def fetch_logic_condition(api: MSPApi, index: int):
    """
    Fetch a single logic condition using MSP2_INAV_LOGIC_CONDITIONS_SINGLE.

    This is the pattern for accessing ANY message:
    1. Pack request data
    2. Send request
    3. Process reply
    """
    # Step 1: Pack request with required fields
    # (Use introspection.py to find out what fields are needed)
    request = api._pack_request(
        InavMSP.MSP2_INAV_LOGIC_CONDITIONS_SINGLE,
        {"conditionIndex": index}
    )

    # Step 2: Send the request
    info, reply = api._request(
        InavMSP.MSP2_INAV_LOGIC_CONDITIONS_SINGLE,
        request
    )

    # Step 3: Process reply (convert enums for readability)
    condition = {
        "enabled": bool(reply["enabled"]),
        "activatorId": reply["activatorId"] if reply["activatorId"] != 255 else None,
        "operation": InavEnums.logicOperation_e(reply["operation"]),
        "operandAType": InavEnums.logicOperandType_e(reply["operandAType"]),
        "operandAValue": reply["operandAValue"],
        "operandBType": InavEnums.logicOperandType_e(reply["operandBType"]),
        "operandBValue": reply["operandBValue"],
        "flags": reply["flags"],
    }

    return info, condition


def format_operand(operand_type, value):
    """Format an operand for display."""
    type_name = operand_type.name.replace("LOGIC_CONDITION_OPERAND_TYPE_", "")

    if operand_type == InavEnums.logicOperandType_e.LOGIC_CONDITION_OPERAND_TYPE_VALUE:
        return f"{type_name}({value})"
    elif operand_type == InavEnums.logicOperandType_e.LOGIC_CONDITION_OPERAND_TYPE_RC_CHANNEL:
        return f"{type_name}(CH{value})"
    elif operand_type == InavEnums.logicOperandType_e.LOGIC_CONDITION_OPERAND_TYPE_LC:
        return f"{type_name}(LC{value})"
    elif operand_type == InavEnums.logicOperandType_e.LOGIC_CONDITION_OPERAND_TYPE_GVAR:
        return f"{type_name}(GVAR{value})"
    else:
        return f"{type_name}({value})"


def main():
    args = parse_args()

    # Connect to flight controller
    if args.tcp:
        print(f"Connecting via TCP: {args.tcp}")
        api = MSPApi(tcp_endpoint=args.tcp)
    else:
        print(f"Connecting via serial: {args.port} @ {args.baudrate}")
        api = MSPApi(port=args.port, baudrate=args.baudrate)

    with api:
        print(f"\n{'='*70}")
        print(f"Fetching Logic Condition #{args.index}")
        print(f"{'='*70}")

        # Fetch the condition
        info, condition = fetch_logic_condition(api, args.index)

        # Display result
        print(f"\nRequest completed in {info['latency_ms']:.1f}ms")
        print(f"\nCondition #{args.index}:")
        print(f"  Enabled:     {condition['enabled']}")
        print(f"  Activator:   {condition['activatorId'] or 'None'}")

        # Operation
        op_name = condition['operation'].name.replace("LOGIC_CONDITION_", "")
        print(f"  Operation:   {op_name}")

        # Operands
        print(f"  Operand A:   {format_operand(condition['operandAType'], condition['operandAValue'])}")
        print(f"  Operand B:   {format_operand(condition['operandBType'], condition['operandBValue'])}")
        print(f"  Flags:       {condition['flags']}")

        print(f"\n{'='*70}")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nInterrupted by user")
    except Exception as e:
        print(f"\n\nError: {e}")
        import traceback
        traceback.print_exc()
