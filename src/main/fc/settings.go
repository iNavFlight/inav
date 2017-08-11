package main

import (
	"bytes"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"sort"
	"strings"

	"gopkg.in/yaml.v2"
)

const (
	settingsFile = "settings.yaml"
	compactNames = true
)

type Table struct {
	Name   string   `yaml:"name"`
	Values []string `yaml:"values"`
	uses   [][]string
}

func (t *Table) VarName() string {
	return fmt.Sprintf("table_%s", t.Name)
}

func (t *Table) ConstantName() string {
	return fmt.Sprintf("TABLE_%s", strings.ToUpper(t.Name))
}

func (t *Table) Conditions() string {
	var or []string
	for _, v := range t.uses {
		if len(v) == 0 {
			// An use without #ifdef, should always be included
			return ""
		}
		defines := make([]string, len(v))
		for ii, c := range v {
			defines[ii] = fmt.Sprintf("defined(%s)", c)
		}
		if len(defines) > 0 {
			or = append(or, strings.Join(defines, " && "))
		}
	}
	return strings.Join(or, " || ")
}

func (t *Table) hasUse(conditions []string) bool {
	for _, v := range t.uses {
		if len(v) == len(conditions) {
			for ii, c := range v {
				if c != conditions[ii] {
					return false
				}
			}
			return true
		}
	}
	return false
}

func (t *Table) AddUse(conditions []string) {
	if !t.hasUse(conditions) {
		cpy := make([]string, len(conditions))
		copy(cpy, conditions)
		t.uses = append(t.uses, cpy)
	}
}

func (t *Table) Used() bool {
	return len(t.uses) > 0
}

type Group struct {
	Name      string    `yaml:"name"`
	Type      string    `yaml:"type"`
	Condition string    `yaml:"condition"`
	Members   []*Member `yaml:"members"`
}

func (g *Group) DefaultValueType() string {
	switch g.Name {
	case "PG_CONTROLRATE_PROFILE":
		return "CONTROL_RATE_VALUE"
	case "PG_PID_PROFILE":
		return "PROFILE_VALUE"
	}
	return "MASTER_VALUE"
}

type Member struct {
	Name      string  `yaml:"name"`
	Field     string  `yaml:"field"`
	Type      string  `yaml:"type"`
	Condition string  `yaml:"condition"`
	Min       *string `yaml:"min"`
	Max       *string `yaml:"max"`
	Table     string  `yaml:"table"`
}

type Settings struct {
	Tables []*Table `yaml:"tables"`
	Groups []*Group `yaml:"groups"`
}

func findHeaders(rootDir string) ([]string, error) {
	var headers []string
	skipDirs := map[string]struct{}{
		"target":  struct{}{},
		"vcp":     struct{}{},
		"vcpf4":   struct{}{},
		"vcp_hal": struct{}{},
	}
	err := filepath.Walk(rootDir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if info.IsDir() {
			if _, skip := skipDirs[info.Name()]; skip {
				return filepath.SkipDir
			}
		}
		if filepath.Ext(path) == ".h" {
			base := filepath.Base(path)
			if strings.Contains(base, "stm32") {
				return nil
			}
			rel, err := filepath.Rel(rootDir, path)
			if err != nil {
				return err
			}
			headers = append(headers, rel)
		}
		return nil
	})
	if err != nil {
		return nil, err
	}
	return headers, nil
}

func splitWords(s string) []string {
	return strings.Split(s, "_")
}

var (
	typeRe = regexp.MustCompile("In instantiation of 'void type_detect_helper\\(T\\) \\[with T = (.*?)\\]'\\:")
)

func detectType(gr *Group, m *Member, rootDir string, headers []string) (string, error) {
	var buf bytes.Buffer
	buf.WriteString("#define NAV\n")
	buf.WriteString("#define STATS\n")
	for _, h := range headers {
		buf.WriteString("#include \"")
		buf.WriteString(filepath.ToSlash(h))
		buf.WriteString("\"\n")
	}
	buf.WriteString("template <typename T> void type_detect_helper(T t) {\n")
	buf.WriteString("t.__this_method_does_not_exist();\n")
	buf.WriteString("}\n")

	buf.WriteString("int main() {\n")
	buf.WriteString(gr.Type)
	buf.WriteString(" var;\n")
	buf.WriteString("type_detect_helper(var.")
	buf.WriteString(m.Field)
	buf.WriteString(");\n")
	buf.WriteString("return 0;")
	buf.WriteString("}\n")

	tmpDir, err := ioutil.TempDir("", "")
	if err != nil {
		return "", err
	}
	defer os.RemoveAll(tmpDir)

	tmpFile := filepath.Join(tmpDir, "detect.cpp")
	if err := ioutil.WriteFile(tmpFile, buf.Bytes(), 0644); err != nil {
		return "", err
	}

	// Create a dummy target.h
	if err := ioutil.WriteFile(filepath.Join(tmpDir, "target.h"), nil, 0644); err != nil {
		return "", err
	}

	cmd := exec.Command("arm-none-eabi-g++", "-I", rootDir, "-I", tmpDir, tmpFile)
	var stderr bytes.Buffer
	cmd.Stderr = &stderr
	cmd.Run()
	match := typeRe.FindStringSubmatch(stderr.String())
	if len(match) != 2 {
		return "", errors.New(stderr.String())
	}
	switch match[1] {
	case "short int":
		return "int16_t", nil
	case "short unsigned int":
		return "uint16_t", nil
	case "signed char":
		return "int8_t", nil
	case "unsigned char":
		return "uint8_t", nil
	case "long unsigned int":
		return "uint32_t", nil
	case "float":
		return "float", nil
	}
	return "", fmt.Errorf("unknown type %q\n%s", match[1], stderr.String())
}

func varType(typ string) (string, error) {
	switch typ {
	case "uint8_t", "bool":
		return "VAR_UINT8", nil
	case "int8_t":
		return "VAR_INT8", nil
	case "uint16_t":
		return "VAR_UINT16", nil
	case "int16_t":
		return "VAR_INT16", nil
	case "uint32_t":
		return "VAR_UINT32", nil
	case "float":
		return "VAR_FLOAT", nil
	}
	return "", fmt.Errorf("unknown variable type %q", typ)
}

func main() {
	wd, err := os.Getwd()
	if err != nil {
		panic(err)
	}
	var settingsDir string
	if len(os.Args) > 1 {
		settingsDir = filepath.Join(wd, os.Args[1])
	} else {
		settingsDir = wd
	}
	abs, err := filepath.Abs(settingsDir)
	if err != nil {
		panic(err)
	}
	srcRoot := filepath.Join(abs, "..", "..", "main")
	headers, err := findHeaders(srcRoot)
	if err != nil {
		panic(err)
	}
	data, err := ioutil.ReadFile(settingsFile)
	if err != nil {
		panic(fmt.Errorf("error reading settings file: %v", err))
	}
	var s Settings
	if err := yaml.Unmarshal(data, &s); err != nil {
		panic(fmt.Errorf("error decoding %s: %v", settingsFile, err))
	}

	tables := make(map[string]*Table)
	for _, tbl := range s.Tables {
		if tbl.Name == "" {
			panic(errors.New("empty table name"))
		}
		if _, dup := tables[tbl.Name]; dup {
			panic(fmt.Errorf("duplicate table name %q", tbl.Name))
		}
		tables[tbl.Name] = tbl
	}

	words := make(map[string]struct{})
	var direct int
	var indexed int
	var maxWordCount int
	var maxNameLength int

	hasBooleans := false

	for _, gr := range s.Groups {
		if gr.Name == "" {
			panic(errors.New("empty group name"))
		}
		for _, m := range gr.Members {
			if m.Name == "" {
				panic(fmt.Errorf("empty name in member in group %q", gr.Name))
			}
			if m.Table != "" {
				if tables[m.Table] == nil {
					panic(fmt.Errorf("field %q references non-existing table %q", m.Name, m.Table))
				}
			}
			if m.Field == "" {
				m.Field = m.Name
			}
			if m.Type == "" {
				typ, err := detectType(gr, m, srcRoot, headers)
				if err != nil {
					panic(fmt.Errorf("could not detect type for member %q in group %q: %v", m.Name, gr.Name, err))
				}
				m.Type = typ
			}
			if m.Type == "bool" {
				hasBooleans = true
				m.Type = "uint8_t"
				m.Table = "off_on"
			}
			direct += len(m.Name) + 1
			nameWords := splitWords(m.Name)
			indexed += len(nameWords)
			if c := len(nameWords); c > maxWordCount {
				maxWordCount = c
			}
			if nl := len(m.Name); nl > maxNameLength {
				maxNameLength = nl
			}
			for _, w := range nameWords {
				if _, ok := words[w]; !ok {
					words[w] = struct{}{}
					indexed += len(w) + 1
				}
			}
		}
	}

	fmt.Println(len(words), "WORDS")
	fmt.Println(direct, "DIRECT", indexed, "INDEXED")
	fmt.Println(maxWordCount, "MAX WORDS")

	if hasBooleans {
		tables["off_on"] = &Table{
			Name:   "off_on",
			Values: []string{"OFF", "ON"},
		}
	}

	var conditions []string

	for _, gr := range s.Groups {
		if gr.Condition != "" {
			conditions = append(conditions, gr.Condition)
		}
		for _, m := range gr.Members {
			if m.Condition != "" {
				conditions = append(conditions, m.Condition)
			}
			if m.Table != "" {
				// Mark the table use
				tbl := tables[m.Table]
				tbl.AddUse(conditions)
			}
			if m.Condition != "" {
				conditions = conditions[:len(conditions)-1]
			}
		}
		if gr.Condition != "" {
			conditions = conditions[:len(conditions)-1]
		}
	}

	if len(conditions) > 0 {
		panic(errors.New("unbalanced conditions"))
	}

	var buf bytes.Buffer

	var formatValueName func(s string) string
	if compactNames {
		var compactedWords []string
		for k := range words {
			compactedWords = append(compactedWords, k)
		}
		sort.Strings(compactedWords)
		buf.WriteString("static const char *words[] = {\n")
		buf.WriteString("\tNULL,\n")
		for _, w := range compactedWords {
			fmt.Fprintf(&buf, "\t%q,\n", w)
		}
		buf.WriteString("};\n")
		formatValueName = func(s string) string {
			words := splitWords(s)
			data := make([]byte, maxWordCount)
			for ii, w := range words {
				var pos int
				for jj, cw := range compactedWords {
					if w == cw {
						pos = jj
						break
					}
				}
				data[ii] = byte(pos)
			}
			var sbuf bytes.Buffer
			sbuf.WriteByte('{')
			for ii := 0; ii < len(data); ii++ {
				fmt.Fprintf(&sbuf, "%d, ", data[ii])
			}
			sbuf.WriteByte('}')
			return sbuf.String()
		}
	} else {
		formatValueName = func(s string) string {
			return fmt.Sprintf("%q", s)
		}
	}

	// Write the tables
	var tableNames []string
	for k, v := range tables {
		if v.Used() {
			tableNames = append(tableNames, k)
		}
	}
	sort.Strings(tableNames)
	for _, k := range tableNames {
		tbl := tables[k]
		cond := tbl.Conditions()
		if cond != "" {
			fmt.Fprintf(&buf, "#if %s\n", cond)
		}
		fmt.Fprintf(&buf, "static const char *%s[] = {\n", tbl.VarName())
		for _, v := range tbl.Values {
			fmt.Fprintf(&buf, "\t%q,\n", v)
		}
		buf.WriteString("};\n")
		if cond != "" {
			buf.WriteString("#endif\n")
		}
	}

	buf.WriteString("enum {\n")
	for _, k := range tableNames {
		tbl := tables[k]
		cond := tbl.Conditions()
		if cond != "" {
			fmt.Fprintf(&buf, "#if %s\n", cond)
		}
		fmt.Fprintf(&buf, "\t%s,\n", tbl.ConstantName())
		if cond != "" {
			buf.WriteString("#endif\n")
		}
	}
	buf.WriteString("\tLOOKUP_TABLE_COUNT,\n")
	buf.WriteString("};\n")

	buf.WriteString("static const lookupTableEntry_t lookupTables[] = {\n")
	for _, k := range tableNames {
		tbl := tables[k]
		cond := tbl.Conditions()
		if cond != "" {
			fmt.Fprintf(&buf, "#if %s\n", cond)
		}
		fmt.Fprintf(&buf, "\t{ %s, sizeof(%s) / sizeof(char*) },\n", tbl.VarName(), tbl.VarName())
		if cond != "" {
			buf.WriteString("#endif\n")
		}
	}

	buf.WriteString("};\n")

	buf.WriteString("const clivalue_t valueTable[] = {\n")

	for _, gr := range s.Groups {
		fmt.Fprintf(&buf, "// %s\n", gr.Name)
		if gr.Condition != "" {
			conditions = append(conditions, gr.Condition)
			fmt.Fprintf(&buf, "#ifdef %s\n", gr.Condition)
		}
		for ii, m := range gr.Members {
			if m.Condition != "" {
				if len(conditions) == 0 || conditions[len(conditions)-1] != m.Condition {
					conditions = append(conditions, m.Condition)
					fmt.Fprintf(&buf, "#ifdef %s\n", m.Condition)
				}
			}
			fmt.Fprintf(&buf, "\t{ %s, ", formatValueName(m.Name))
			typ, err := varType(m.Type)
			if err != nil {
				panic(err)
			}
			fmt.Fprintf(&buf, "%s | %s", typ, gr.DefaultValueType())
			if m.Table != "" {
				tbl := tables[m.Table]
				buf.WriteString(" | MODE_LOOKUP")
				fmt.Fprintf(&buf, ", .config.lookup = { %s }", tbl.ConstantName())
			} else {
				if m.Min != nil && m.Max != nil {
					fmt.Fprintf(&buf, ", .config.minmax = {%s, %s}", *m.Min, *m.Max)
				} else if m.Max != nil {
					buf.WriteString(" | MODE_MAX")
					fmt.Fprintf(&buf, ", .config.max = {%s}", *m.Max)
				}
			}
			buf.WriteString(", ")
			fmt.Fprintf(&buf, "offsetof(%s, %s) },\n", gr.Type, m.Field)
			if m.Condition != "" {
				if conditions[len(conditions)-1] == m.Condition &&
					!(ii < len(gr.Members)-1 && gr.Members[ii+1].Condition == m.Condition) {
					conditions = conditions[:len(conditions)-1]
					buf.WriteString("#endif\n")
				}
			}
		}
		if gr.Condition != "" {
			conditions = conditions[:len(conditions)-1]
			buf.WriteString("#endif\n")
		}
	}
	buf.WriteString("};\n")
	if err := ioutil.WriteFile("settings_generated.c", buf.Bytes(), 0644); err != nil {
		panic(err)
	}

	buf.WriteString("}\n")

	buf.Reset()

	// Generate the header file
	buf.WriteString("#pragma once\n")
	if compactNames {
		buf.WriteString("#define CLIVALUE_COMPACT_NAMES\n")
	}
	fmt.Fprintf(&buf, "#define CLIVALUE_MAX_NAME_BYTES %d\n", maxWordCount)
	fmt.Fprintf(&buf, "#define CLIVALUE_MAX_NAME_LENGTH %d\n", maxNameLength)
	if err := ioutil.WriteFile("settings_generated.h", buf.Bytes(), 0644); err != nil {
		panic(err)
	}

	for _, t := range s.Tables {
		if !t.Used() {
			fmt.Fprintf(os.Stderr, "WARNING: unused table %q\n", t.Name)
		}
	}
}
