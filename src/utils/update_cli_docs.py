#!/usr/bin/env python3

import re
from sys import argv
import yaml

CLI_MD_PATH = "docs/Cli.md"
SETTINGS_YAML_PATH = "src/main/fc/settings.yaml"

CLI_MD_SECTION_HEADER = "## CLI Variable Reference"

def read_cli_md():
    with open(CLI_MD_PATH, "r") as cli_md:
        return cli_md.readlines()

def parse_params_from_md_table(cli_md_lines):
    table_offset = cli_md_lines.index("## CLI Variable Reference\n") + 4
    params, i = {}, table_offset
    while cli_md_lines[i] != "" and i < len(cli_md_lines) - 1:
        param = [ s.strip() for s in cli_md_lines[i].strip("\n").split("|") ][1:-1]
        params[param[0]] = { 'default': param[1], 'description': param[2] }
        i += 1
    return params

def read_settings_yaml():
    with open(SETTINGS_YAML_PATH, "r") as settings_yaml:
        return settings_yaml.readlines()

def update_settings_yaml_from_cli_md(cli_md_params, settings_yaml_lines):
    new_lines = []
    skip = False
    for line in settings_yaml_lines:
        if skip:
            if line == "\n":
                skip = False
        elif line.startswith("tables:"):
            skip = True
        elif "- name:" in line:
            for param in cli_md_params:
                match = re.match("^\\s+- name: {}\n$".format(param), line)
                if match:
                    match = match.group(0)
                    prefix = match[:match.index("- name: ") + 8]
                    line = "{}\n{}\"{}\"\n".format(
                            match.strip("\n"),
                            prefix.replace("- name", "  docs_description"),
                            cli_md_params[param]['description'].replace("\"", "\\\""),
                        )
                    if cli_md_params[param]['default']:
                        line = "{}{}\"{}\"\n".format(
                                line,
                                prefix.replace("- name", "  docs_default_value"),
                                cli_md_params[param]['default'].replace("\"", "\\\""),
                            )
                    cli_md_params[param]['documented'] = True
                    break
        new_lines.append(line)
    
    for param in cli_md_params:
        if not "documented" in cli_md_params[param] or not cli_md_params[param]['documented']:
            print("\"{}\" is undocumented".format(param))

    return new_lines

def write_settings_yaml(lines):
    with open(SETTINGS_YAML_PATH, "w") as settings_yaml:
        settings_yaml.writelines(lines)

def parse_settings_yaml():
    with open(SETTINGS_YAML_PATH, "r") as settings_yaml:
        return yaml.load(settings_yaml, Loader=yaml.Loader)

def generate_md_table_from_yaml(settings_yaml):
    params = {}
    for group in settings_yaml['groups']:
        for member in group['members']:
            if any(key in member for key in ["docs_description", "docs_default_value"]):
                params[member['name']] = {
                        "description": member["docs_description"],
                        "default": member["docs_default_value"] if "docs_default_value" in member else ""
                    }

    md_table_lines = [
        "| Variable Name | Default Value | Description |\n",
        "| ------------- | ------------- | ----------- |\n",
        ]
    for param in sorted(params.items()):
        md_table_lines.append("| {} | {} | {} |\n".format(param[0], param[1]['default'], param[1]['description']))

    return md_table_lines

def replace_md_table_in_cli_md(cli_md_lines, md_table_lines):
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

def write_cli_md(lines):
    with open(CLI_MD_PATH, "w") as cli_md:
        cli_md.writelines(lines)

if __name__ == "__main__":
    if len(argv) == 2 and argv[1] == "--reverse":
        cli_md_lines = read_cli_md()
        cli_md_params = parse_params_from_md_table(cli_md_lines)
        settings_yaml_lines = read_settings_yaml()
        settings_yaml_lines = update_settings_yaml_from_cli_md(cli_md_params, settings_yaml_lines)
        write_settings_yaml(settings_yaml_lines)
    else:
        settings_yaml = parse_settings_yaml()
        md_table_lines = generate_md_table_from_yaml(settings_yaml)
        cli_md_lines = read_cli_md()
        cli_md_lines = replace_md_table_in_cli_md(cli_md_lines, md_table_lines)
        write_cli_md(cli_md_lines)

