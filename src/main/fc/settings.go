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
	settingsFile         = "settings.yaml"
	maxEncodedWordLength = 6
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
	case "PG_CONTROL_RATE_PROFILES":
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

var (
	typeRe = regexp.MustCompile("In instantiation of 'void type_detect_helper\\(T\\) \\[with T = (.*?)\\]'\\:")
)

type SettingsGenerator struct {
	rootDir              string
	allHeaders           []string
	typeDetectionHeaders []string
	s                    *Settings
	tables               map[string]*Table
	hasBooleans          bool
	settingsCount        int
	// Maximum unpacked name length, so
	// the C code can allocate an appropiately
	// sized buffer.
	maxNameLength int
	// Words that shouldn't be split because
	// their encoding is too long
	nonSplit map[string]struct{}
	// Value is number of uses
	words map[string]int
	// Most used words first
	wordsByUsage []string
	encodedWords map[string][]byte
}

func New(rootDir string, settingsFile string) (*SettingsGenerator, error) {
	g := &SettingsGenerator{
		rootDir: rootDir,
	}
	if err := g.findHeaders(); err != nil {
		return nil, err
	}
	data, err := ioutil.ReadFile(settingsFile)
	if err != nil {
		return nil, fmt.Errorf("error reading settings file: %v", err)
	}
	if err := yaml.Unmarshal(data, &g.s); err != nil {
		return nil, fmt.Errorf("error decoding %s: %v", settingsFile, err)
	}
	if err := g.mapTables(); err != nil {
		return nil, err
	}
	if err := g.sanitizeFields(); err != nil {
		return nil, err
	}
	g.initilizeTableUsage()
	g.updateWords()
	return g, nil
}

func (g *SettingsGenerator) SettingsCount() int {
	return g.settingsCount
}

func (g *SettingsGenerator) findHeaders() error {
	skipDirs := map[string]struct{}{
		"target":  struct{}{},
		"vcp":     struct{}{},
		"vcpf4":   struct{}{},
		"vcp_hal": struct{}{},
	}
	err := filepath.Walk(g.rootDir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if info.IsDir() {
			if _, skip := skipDirs[info.Name()]; skip {
				return filepath.SkipDir
			}
		}
		if filepath.Ext(path) == ".h" {
			rel, err := filepath.Rel(g.rootDir, path)
			if err != nil {
				return err
			}
			g.allHeaders = append(g.allHeaders, rel)
			base := filepath.Base(path)
			if strings.Contains(base, "stm32") {
				return nil
			}
			g.typeDetectionHeaders = append(g.typeDetectionHeaders, rel)
		}
		return nil
	})
	return err
}

func (g *SettingsGenerator) mapTables() error {
	g.tables = make(map[string]*Table)
	for _, tbl := range g.s.Tables {
		if tbl.Name == "" {
			return errors.New("empty table name")
		}
		if _, dup := g.tables[tbl.Name]; dup {
			return fmt.Errorf("duplicate table name %q", tbl.Name)
		}
		g.tables[tbl.Name] = tbl
	}
	return nil
}

func (g *SettingsGenerator) sanitizeFields() error {
	for _, gr := range g.s.Groups {
		g.settingsCount += len(gr.Members)
		if gr.Name == "" {
			return errors.New("empty group name")
		}
		for _, m := range gr.Members {
			if m.Name == "" {
				return fmt.Errorf("empty name in member in group %q", gr.Name)
			}
			if nl := len(m.Name); nl > g.maxNameLength {
				g.maxNameLength = nl
			}
			if m.Table != "" {
				if g.tables[m.Table] == nil {
					fmt.Println("TABLES", g.tables)
					return fmt.Errorf("field %q references non-existing table %q", m.Name, m.Table)
				}
			}
			if m.Field == "" {
				m.Field = m.Name
			}
			if m.Type == "" {
				typ, err := g.detectType(gr, m)
				if err != nil {
					return fmt.Errorf("could not detect type for member %q in group %q: %v", m.Name, gr.Name, err)
				}
				m.Type = typ
			}
			if m.Type == "bool" {
				g.hasBooleans = true
				m.Type = "uint8_t"
				m.Table = "off_on"
			}
		}
	}
	if g.hasBooleans {
		g.tables["off_on"] = &Table{
			Name:   "off_on",
			Values: []string{"OFF", "ON"},
		}
	}
	return nil
}

func (g *SettingsGenerator) detectType(gr *Group, m *Member) (string, error) {
	var buf bytes.Buffer
	buf.WriteString("#define NAV\n")
	buf.WriteString("#define STATS\n")
	for _, h := range g.typeDetectionHeaders {
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

	cmd := exec.Command("arm-none-eabi-g++", "-I", g.rootDir, "-I", tmpDir, tmpFile)
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

func (g *SettingsGenerator) varType(typ string) (string, error) {
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

func (g *SettingsGenerator) writeUVarInt(w *bytes.Buffer, x uint32) {
	for x >= 0x80 {
		w.WriteByte(byte(x) | 0x80)
		x >>= 7
	}
	w.WriteByte(byte(x))
}

func (g *SettingsGenerator) splitWords(s string) []string {
	if _, ok := g.nonSplit[s]; ok {
		return []string{s}
	}
	split := strings.Split(s, "_")
	/* TODO:	if len(split) >= 4 {
		return []string{s}
	}
	*/
	return split
}

func (g *SettingsGenerator) updateWords() {
	g.words = make(map[string]int)
	for _, gr := range g.s.Groups {
		for _, m := range gr.Members {
			nameWords := g.splitWords(m.Name)
			for _, w := range nameWords {
				g.words[w]++
			}
		}
	}
	g.wordsByUsage = g.wordsByUsage[:0]
	for k := range g.words {
		g.wordsByUsage = append(g.wordsByUsage, k)
	}
	// Sort words by use, most used first
	sort.Slice(g.wordsByUsage, func(i, j int) bool {
		return g.words[g.wordsByUsage[i]] > g.words[g.wordsByUsage[j]]
	})
}

func (g *SettingsGenerator) initilizeTableUsage() {
	var conditions []string

	for _, gr := range g.s.Groups {
		if gr.Condition != "" {
			conditions = append(conditions, gr.Condition)
		}
		for _, m := range gr.Members {
			if m.Condition != "" {
				conditions = append(conditions, m.Condition)
			}
			if m.Table != "" {
				// Mark the table use
				tbl := g.tables[m.Table]
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
}

func (g *SettingsGenerator) indexOfWord(s string) int {
	for ii, v := range g.wordsByUsage {
		if v == s {
			return ii + 1
		}
	}
	return -1
}

func (g *SettingsGenerator) encodeNames() error {
	g.encodedWords = make(map[string][]byte)
	for _, gr := range g.s.Groups {
		for _, m := range gr.Members {
			words := g.splitWords(m.Name)
			var indexes []int
			for _, w := range words {
				pos := g.indexOfWord(w)
				if pos < 0 {
					return fmt.Errorf("word %q not in the words list", w)
				}
				indexes = append(indexes, pos)
			}
			var buf bytes.Buffer
			for _, v := range indexes {
				g.writeUVarInt(&buf, uint32(v))
			}
			data := buf.Bytes()
			if len(data) > maxEncodedWordLength {
				fmt.Printf("encoding %q took %d bytes (>%d), adding it as single word\n", m.Name, len(data), maxEncodedWordLength)
				if g.nonSplit == nil {
					g.nonSplit = make(map[string]struct{})
				}
				g.nonSplit[m.Name] = struct{}{}
				g.updateWords()
				return g.encodeNames()
			}
			g.encodedWords[m.Name] = data
		}
	}
	return nil
}

func (g *SettingsGenerator) formatEncodedWord(s string) (string, error) {
	enc := g.encodedWords[s]
	if len(enc) == 0 {
		return "", fmt.Errorf("word %q was not encoded", s)
	}
	if len(enc) < maxEncodedWordLength {
		bs := make([]byte, maxEncodedWordLength)
		copy(bs, enc)
		enc = bs
	}
	var buf bytes.Buffer
	buf.WriteByte('{')
	for ii := 0; ii < len(enc); ii++ {
		fmt.Fprintf(&buf, "%d, ", enc[ii])
	}
	buf.WriteByte('}')

	return buf.String(), nil
}

func (g *SettingsGenerator) writeHeaderFile() error {
	var buf bytes.Buffer
	buf.WriteString("#pragma once\n")
	fmt.Fprintf(&buf, "#define CLIVALUE_MAX_NAME_LENGTH %d\n", g.maxNameLength)
	fmt.Fprintf(&buf, "#define CLIVALUE_ENCODED_NAME_MAX_BYTES %d\n", maxEncodedWordLength)
	return ioutil.WriteFile("settings_generated.h", buf.Bytes(), 0644)
}

func (g *SettingsGenerator) writeImplementationFile() error {
	var buf bytes.Buffer
	// Write word list
	buf.WriteString("static const char *words[] = {\n")
	buf.WriteString("\tNULL,\n")
	for _, w := range g.wordsByUsage {
		fmt.Fprintf(&buf, "\t%q,\n", w)
	}
	buf.WriteString("};\n")

	// Write the tables
	var tableNames []string
	for k, v := range g.tables {
		if v.Used() {
			tableNames = append(tableNames, k)
		}
	}
	sort.Strings(tableNames)
	for _, k := range tableNames {
		tbl := g.tables[k]
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
		tbl := g.tables[k]
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
		tbl := g.tables[k]
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

	// Write values
	var conditions []string

	buf.WriteString("const clivalue_t valueTable[] = {\n")

	for _, gr := range g.s.Groups {
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
			enc, err := g.formatEncodedWord(m.Name)
			if err != nil {
				return err
			}
			fmt.Fprintf(&buf, "\t{ %s, ", enc)
			typ, err := g.varType(m.Type)
			if err != nil {
				return err
			}
			fmt.Fprintf(&buf, "%s | %s", typ, gr.DefaultValueType())
			if m.Table != "" {
				tbl := g.tables[m.Table]
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
			fmt.Fprintf(&buf, ", %s, ", gr.Name)
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
	return ioutil.WriteFile("settings_generated.c", buf.Bytes(), 0644)
}

func (g *SettingsGenerator) WriteFiles() error {
	if err := g.encodeNames(); err != nil {
		return err
	}
	if err := g.writeHeaderFile(); err != nil {
		return err
	}
	return g.writeImplementationFile()
}

func (g *SettingsGenerator) PrintWarnings() {
	for _, t := range g.s.Tables {
		if !t.Used() {
			fmt.Fprintf(os.Stderr, "WARNING: unused table %q\n", t.Name)
		}
	}
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

	gen, err := New(srcRoot, filepath.Join(settingsDir, settingsFile))
	if err != nil {
		panic(err)
	}

	fmt.Println(gen.SettingsCount(), "settings")

	if err := gen.WriteFiles(); err != nil {
		panic(err)
	}

	gen.PrintWarnings()
}
