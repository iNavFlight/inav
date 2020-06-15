#!/usr/bin/env python3

import yaml  # pyyaml / python-yaml

CLI_MD_PATH = "docs/Cli.md"
SETTINGS_YAML_PATH = "src/main/fc/settings.yaml"

CLI_MD_SECTION_HEADER = "## CLI Variable Reference"

# Read the contents of the markdown CLI docs
def read_cli_md():
    with open(CLI_MD_PATH, "r") as cli_md:
        return cli_md.readlines()

# Parse the YAML settings specs
def parse_settings_yaml():
    with open(SETTINGS_YAML_PATH, "r") as settings_yaml:
        return yaml.load(settings_yaml, Loader=yaml.Loader)

def generate_md_table_from_yaml(settings_yaml):
    """Generate a sorted markdown table with description & default value for each setting"""
    params = {}
    
    # Extract description and default value of each setting from the YAML specs (if present)
    for group in settings_yaml['groups']:
        for member in group['members']:
            if any(key in member for key in ["docs_description", "docs_default_value"]):
                params[member['name']] = {
                        "description": member["docs_description"] if "docs_description" in member else "",
                        "default": member["docs_default_value"] if "docs_default_value" in member else ""
                    }
    
    # MD table header
    md_table_lines = [
        "| Variable Name | Default Value | Description |\n",
        "| ------------- | ------------- | ----------- |\n",
        ]
    
    # Sort the settings by name and build the rows of the table
    for param in sorted(params.items()):
        md_table_lines.append("| {} | {} | {} |\n".format(param[0], param[1]['default'], param[1]['description']))
    
    # Return the assembled table
    return md_table_lines

def replace_md_table_in_cli_md(cli_md_lines, md_table_lines):
    """Update the settings table in the CLI docs

    Copy all the original lines up to $CLI_MD_SECTION_HEADER (including the following newline), then insert
    the updated table in place of the next block of text (replace everything until an empty line is found).
    """
    new_lines = []
    lines_to_skip = 0
    skip_until_empty_line = False
    for line in cli_md_lines:
        if lines_to_skip > 0:
            lines_to_skip -= 1
            if lines_to_skip == 0:
                skip_until_empty_line = True
            continue
        elif skip_until_empty_line:
            if line != "\n":
                continue
            else:
                skip_until_empty_line = False
                new_lines.append("\n")
                new_lines += md_table_lines
        if line.startswith(CLI_MD_SECTION_HEADER):
            lines_to_skip = 2
        new_lines.append(line)
    
    return new_lines

# Write the contents of the markdown CLI docs
def write_cli_md(lines):
    with open(CLI_MD_PATH, "w") as cli_md:
        cli_md.writelines(lines)

if __name__ == "__main__":
    settings_yaml = parse_settings_yaml()
    md_table_lines = generate_md_table_from_yaml(settings_yaml)
    cli_md_lines = read_cli_md()
    cli_md_lines = replace_md_table_in_cli_md(cli_md_lines, md_table_lines)
    write_cli_md(cli_md_lines)
