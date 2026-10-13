"""
Helper functions for discovering MSP message structure.

Use these to find out what fields a message has, what types they are,
and which ones use enums.
"""

from pathlib import Path
from typing import Dict, List, Any, Optional

from inav_msp import MSPCodec, InavMSP


def get_message_info(code: InavMSP, codec: Optional[MSPCodec] = None) -> Dict[str, Any]:
    """
    Get detailed information about an MSP message.

    Args:
        code: The MSP code (e.g., InavMSP.MSP2_INAV_LOGIC_CONDITIONS_SINGLE)
        codec: Optional MSPCodec instance (will create one if not provided)

    Returns:
        Dict with message details including field names, types, descriptions
    """
    if codec is None:
        codec = MSPCodec()

    spec = codec.messages[InavMSP(code).name]

    def describe(side: Optional[Dict[str, Any]]) -> Dict[str, Any]:
        fields = (side or {}).get("fields") or []
        return {
            "fields": [
                {
                    "name": f["name"],
                    "kind": f["kind"],
                    "type": f.get("code") or f.get("type") or f["kind"],
                    "enum": f.get("enum"),
                    "optional": f.get("optional", False),
                    "bitmask": f.get("bitmask", False),
                    "description": f.get("description", ""),
                }
                for f in fields
            ],
            "field_names": [f["name"] for f in fields],
        }

    return {
        "code": spec["code"],
        "name": spec["name"],
        "msp_version": spec["mspv"],
        "not_implemented": spec["not_implemented"],
        "request": describe(spec.get("request")),
        "reply": describe(spec.get("reply")),
    }


def print_message_info(code: InavMSP, codec: Optional[MSPCodec] = None) -> None:
    """
    Pretty-print information about an MSP message.

    Args:
        code: The MSP code (e.g., InavMSP.MSP2_INAV_LOGIC_CONDITIONS_SINGLE)
        codec: Optional MSPCodec instance
    """
    info = get_message_info(code, codec)

    print(f"\n{'='*70}")
    print(f"{info['name']}")
    print(f"{'='*70}")
    print(f"Code:        {info['code']}")
    print(f"MSP Version: {info['msp_version']}")

    # Request fields
    print(f"\n--- REQUEST ---")
    if info['request']['fields']:
        for field in info['request']['fields']:
            enum_str = f" (enum: {field['enum']})" if field['enum'] else ""
            print(f"  {field['name']:20s} {field['type']:10s}{enum_str}")
            if field['description']:
                print(f"    → {field['description']}")
    else:
        print("  (no request payload)")

    # Reply fields
    print(f"\n--- REPLY ---")
    if info['reply']['fields']:
        for field in info['reply']['fields']:
            enum_str = f" (enum: {field['enum']})" if field['enum'] else ""
            bitmask_str = " (bitmask)" if field['bitmask'] else ""
            print(f"  {field['name']:20s} {field['type']:10s}{enum_str}{bitmask_str}")
            if field['description']:
                print(f"    → {field['description']}")
    else:
        print("  (no reply payload)")

    print(f"{'='*70}\n")


def list_all_messages(filter_pattern: Optional[str] = None) -> List[str]:
    """
    List all available MSP messages.

    Args:
        filter_pattern: Optional string to filter message names (case-insensitive)

    Returns:
        List of message names
    """
    messages = list(InavMSP.__members__.keys())

    if filter_pattern:
        pattern = filter_pattern.upper()
        messages = [m for m in messages if pattern in m]

    return sorted(messages)


def print_all_messages(filter_pattern: Optional[str] = None) -> None:
    """
    Pretty-print all available MSP messages.

    Args:
        filter_pattern: Optional string to filter message names
    """
    messages = list_all_messages(filter_pattern)

    print(f"\nFound {len(messages)} MSP messages")
    if filter_pattern:
        print(f"(filtered by '{filter_pattern}')")
    print()

    for msg in messages:
        code = InavMSP[msg]
        print(f"  {msg:50s} = {code.value:5d}")


# Example usage
if __name__ == "__main__":
    print("MSP Message Introspection Examples")
    print("="*70)

    # Example 1: Get info about a specific message
    print("\nExample 1: MSP2_INAV_LOGIC_CONDITIONS_SINGLE")
    print_message_info(InavMSP.MSP2_INAV_LOGIC_CONDITIONS_SINGLE)

    # Example 2: Search for messages
    print("\nExample 2: Search for LOGIC-related messages")
    print_all_messages("LOGIC")

    # Example 3: Programmatic access
    print("\nExample 3: Get field names programmatically")
    info = get_message_info(InavMSP.MSP_API_VERSION)
    print(f"Request fields: {info['request']['field_names']}")
    print(f"Reply fields: {info['reply']['field_names']}")
