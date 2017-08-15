package main

import (
	"bufio"
	"bytes"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"

	"gopkg.in/yaml.v2"
)

const (
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
	Headers   []string  `yaml:"headers"`
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

type SettingsGenerator struct {
	rootDir       string
	outputDir     string
	s             *Settings
	tables        map[string]*Table
	hasBooleans   bool
	settingsCount int
	// Types that need to be resolved
	pendingTypes map[*Member]*Group
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
		rootDir:   rootDir,
		outputDir: filepath.Dir(settingsFile),
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
	fmt.Printf("word table has %d entries\n", len(g.words))
	return g, nil
}

func (g *SettingsGenerator) SettingsCount() int {
	return g.settingsCount
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
					return fmt.Errorf("field %q references non-existing table %q", m.Name, m.Table)
				}
			}
			if m.Field == "" {
				m.Field = m.Name
			}
			if m.Type == "" {
				if g.pendingTypes == nil {
					g.pendingTypes = make(map[*Member]*Group)
				}
				g.pendingTypes[m] = gr
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
	if err := g.resolveTypes(); err != nil {
		return err
	}
	return nil
}

func (g *SettingsGenerator) addHeader(w io.Writer, h string) {
	fmt.Fprintf(w, "#include \"%s\"\n", h)
}

func (g *SettingsGenerator) resolveTypes() error {
	var buf bytes.Buffer
	buf.WriteString("#define NAV\n")
	buf.WriteString("#define STATS\n")
	g.addHeader(&buf, "target.h")
	for _, gr := range g.s.Groups {
		for _, h := range gr.Headers {
			g.addHeader(&buf, h)
		}
	}
	buf.WriteString("int main() {\n")
	ii := 0
	lines := make(map[int]*Member)
	curLine := strings.Count(buf.String(), "\n") + 1
	for k, v := range g.pendingTypes {
		ii++
		lines[curLine] = k
		curLine++
		varName := fmt.Sprintf("var_%d", ii)
		fmt.Fprintf(&buf, "%s %s; %s.%s.__type_detect_;\n", v.Type, varName, varName, k.Field)
	}
	buf.WriteString("return 0;\n")
	buf.WriteString("}\n")

	tmpDir, err := ioutil.TempDir("", "")
	if err != nil {
		return err
	}
	defer os.RemoveAll(tmpDir)

	tmpFile := filepath.Join(tmpDir, "detect.cpp")
	if err := ioutil.WriteFile(tmpFile, buf.Bytes(), 0644); err != nil {
		return err
	}

	cflags := strings.Split(os.Getenv("CFLAGS"), " ")
	var args []string
	for _, flag := range cflags {
		// Don't generate temporary files
		if flag == "-MMD" || flag == "-MP" || strings.HasPrefix(flag, "-save-temps") {
			continue
		}
		if flag != "" {
			if strings.HasPrefix(flag, "-D'") {
				// Cleanup flag. Done by the shell when called from
				// it but we must do it ourselves becase we're not
				// calling the compiler via shell.
				flag = "-D" + flag[3:len(flag)-1]
			}
			args = append(args, flag)
		}
	}
	args = append(args, tmpFile)
	cmd := exec.Command("arm-none-eabi-g++", args...)
	cmd.Stdout = os.Stdout
	var stderr bytes.Buffer
	cmd.Stderr = &stderr
	cmd.Run()
	sc := bufio.NewScanner(bytes.NewReader(stderr.Bytes()))
	typeRe := regexp.MustCompile("which is of non-class type '(.*)'")

	for sc.Scan() {
		text := sc.Text()
		if strings.Contains(text, "error: request for member '__type_detect_'") {
			sep1 := strings.IndexByte(text, ':')
			sep2 := strings.IndexByte(text[sep1+1:], ':') + sep1 + 1
			line, err := strconv.Atoi(text[sep1+1 : sep2])
			if err != nil {
				return err
			}
			m := lines[line]
			if m == nil {
				return fmt.Errorf("no member found at line %d, compiler output: %s", line, stderr.String())
			}
			match := typeRe.FindStringSubmatch(text[sep2:])
			if len(match) != 2 {
				return fmt.Errorf("malformed output line %q", text)
			}
			switch match[1] {
			case "int8_t {aka signed char}":
				m.Type = "int8_t"
			case "uint8_t {aka unsigned char}":
				m.Type = "uint8_t"
			case "int16_t {aka short int}":
				m.Type = "int16_t"
			case "uint16_t {aka short unsigned int}":
				m.Type = "uint16_t"
			case "uint32_t {aka long unsigned int}":
				m.Type = "uint32_t"
			case "float":
				m.Type = "float"
			default:
				return fmt.Errorf("unknown type %q for setting %q (field %s)", match[1], m.Name, m.Field)
			}
		}
	}
	for _, gr := range g.s.Groups {
		for _, m := range gr.Members {
			if m.Type == "" {
				return fmt.Errorf("could not determine type for %q (field %s in %s)", m.Name, m.Field, gr.Type)
			}
		}
	}
	return nil
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

func (g *SettingsGenerator) orderedTableNames() []string {
	var tableNames []string
	for k, v := range g.tables {
		if v.Used() {
			tableNames = append(tableNames, k)
		}
	}
	sort.Strings(tableNames)
	return tableNames
}

func (g *SettingsGenerator) writeHeaderFile() error {
	var buf bytes.Buffer
	buf.WriteString("#pragma once\n")
	// Write clivalue_t size constants
	fmt.Fprintf(&buf, "#define CLIVALUE_MAX_NAME_LENGTH %d\n", g.maxNameLength)
	fmt.Fprintf(&buf, "#define CLIVALUE_ENCODED_NAME_MAX_BYTES %d\n", maxEncodedWordLength)
	fmt.Fprintf(&buf, "#define CLIVALUE_TABLE_COUNT %d\n", g.SettingsCount())
	// Write lookup table constants
	tableNames := g.orderedTableNames()
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
	// Write table pointers
	for _, k := range tableNames {
		tbl := g.tables[k]
		cond := tbl.Conditions()
		if cond != "" {
			fmt.Fprintf(&buf, "#if %s\n", cond)
		}
		fmt.Fprintf(&buf, "extern const char *%s[];\n", tbl.VarName())
		if cond != "" {
			buf.WriteString("#endif\n")
		}
	}

	return ioutil.WriteFile(filepath.Join(g.outputDir, "settings_generated.h"), buf.Bytes(), 0644)
}

func (g *SettingsGenerator) writeImplementationFile() error {
	var buf bytes.Buffer
	g.addHeader(&buf, "platform.h")
	g.addHeader(&buf, "config/parameter_group_ids.h")
	g.addHeader(&buf, "settings.h")
	for _, gr := range g.s.Groups {
		for _, h := range gr.Headers {
			g.addHeader(&buf, h)
		}
	}
	// Write word list
	buf.WriteString("const char *cliValueWords[] = {\n")
	buf.WriteString("\tNULL,\n")
	for _, w := range g.wordsByUsage {
		fmt.Fprintf(&buf, "\t%q,\n", w)
	}
	buf.WriteString("};\n")

	// Write the tables
	tableNames := g.orderedTableNames()
	for _, k := range tableNames {
		tbl := g.tables[k]
		cond := tbl.Conditions()
		if cond != "" {
			fmt.Fprintf(&buf, "#if %s\n", cond)
		}
		fmt.Fprintf(&buf, "const char *%s[] = {\n", tbl.VarName())
		for _, v := range tbl.Values {
			fmt.Fprintf(&buf, "\t%q,\n", v)
		}
		buf.WriteString("};\n")
		if cond != "" {
			buf.WriteString("#endif\n")
		}
	}

	buf.WriteString("const lookupTableEntry_t cliLookupTables[] = {\n")
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

	buf.WriteString("const clivalue_t cliValueTable[] = {\n")

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
	return ioutil.WriteFile(filepath.Join(g.outputDir, "settings_generated.c"), buf.Bytes(), 0644)
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
	srcRoot := os.Args[1]
	settingsFile := os.Args[2]
	gen, err := New(srcRoot, settingsFile)
	if err != nil {
		panic(err)
	}
	fmt.Println(gen.SettingsCount(), "settings")
	if err := gen.WriteFiles(); err != nil {
		panic(err)
	}

	gen.PrintWarnings()
}
