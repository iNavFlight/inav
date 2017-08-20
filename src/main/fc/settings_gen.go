package main

import (
	"bufio"
	"bytes"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"math"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"unicode"
	"unicode/utf8"

	"gopkg.in/yaml.v2"
)

func writeUVarInt(w *bytes.Buffer, x uint32) {
	for x >= 0x80 {
		w.WriteByte(byte(x) | 0x80)
		x >>= 7
	}
	w.WriteByte(byte(x))
}

func encodeBytes(bs []byte) string {
	var buf bytes.Buffer
	buf.WriteByte('{')
	for _, b := range bs {
		fmt.Fprintf(&buf, "%d, ", int(b))
	}
	buf.WriteByte('}')
	return buf.String()
}

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

func (t *Table) Enabled(trueConditions map[string]struct{}) bool {
	for _, v := range t.uses {
		enabled := true
		for _, cond := range v {
			if _, found := trueConditions[cond]; !found {
				enabled = false
				break
			}
		}
		if enabled {
			return true
		}
	}
	return false
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

type NameEncoder struct {
	MaxEncodedLength int
	Names            []string
	// Value is number of uses
	words map[string]int
	// Most used words first
	wordsByUsage []string
	// Words that shouldn't be split because
	// their encoding is too long
	nonSplit map[string]struct{}
	// Key is the word, value is its encoding
	encodedWords map[string][]byte
}

func (e *NameEncoder) Initialize() error {
	e.nonSplit = make(map[string]struct{})
	if err := e.updateWords(); err != nil {
		return err
	}
	if err := e.encodeNames(); err != nil {
		return err
	}
	return nil
}

func (e *NameEncoder) indexOfWord(s string) int {
	for ii, v := range e.wordsByUsage {
		if v == s {
			return ii + 1
		}
	}
	return -1
}

func (e *NameEncoder) splitWords(s string) []string {
	if _, ok := e.nonSplit[s]; ok {
		return []string{s}
	}
	split := strings.Split(s, "_")
	/* TODO:	if len(split) >= 4 {
		return []string{s}
	}
	*/
	return split
}

func (e *NameEncoder) updateWords() error {
	e.words = make(map[string]int)
	for _, v := range e.Names {
		nameWords := e.splitWords(v)
		for _, w := range nameWords {
			e.words[w]++
		}
	}
	e.wordsByUsage = e.wordsByUsage[:0]
	for k := range e.words {
		e.wordsByUsage = append(e.wordsByUsage, k)
	}
	// Sort words by use, most used first
	sort.Slice(e.wordsByUsage, func(i, j int) bool {
		return e.words[e.wordsByUsage[i]] > e.words[e.wordsByUsage[j]]
	})
	return nil
}

func (e *NameEncoder) encodeNames() error {
	e.encodedWords = make(map[string][]byte)
	for _, v := range e.Names {
		words := e.splitWords(v)
		var indexes []int
		for _, w := range words {
			pos := e.indexOfWord(w)
			if pos < 0 {
				return fmt.Errorf("word %q not in the words list", w)
			}
			indexes = append(indexes, pos)
		}
		var buf bytes.Buffer
		for _, v := range indexes {
			if e.UsesDirectIndexing() {
				buf.WriteByte(byte(v))
			} else {
				writeUVarInt(&buf, uint32(v))
			}
		}
		data := buf.Bytes()
		if len(data) > e.MaxEncodedLength {
			// TODO: print in verbose mode
			//fmt.Printf("encoding %q took %d bytes (>%d), adding it as single word\n", v, len(data), e.MaxEncodedLength)
			e.nonSplit[v] = struct{}{}
			e.updateWords()
			return e.encodeNames()
		}
		e.encodedWords[v] = data
	}
	return nil
}

func (e *NameEncoder) UsesDirectIndexing() bool {
	return len(e.wordsByUsage) < 255
}

func (e *NameEncoder) WordList() []string {
	words := make([]string, len(e.wordsByUsage))
	copy(words, e.wordsByUsage)
	return words
}

func (e *NameEncoder) NWords() int {
	return len(e.wordsByUsage)
}

func (e *NameEncoder) FormatEncodedName(s string) (string, error) {
	enc := e.encodedWords[s]
	if len(enc) == 0 {
		return "", fmt.Errorf("name %q was not encoded", s)
	}
	if len(enc) < e.MaxEncodedLength {
		bs := make([]byte, e.MaxEncodedLength)
		copy(bs, enc)
		enc = bs
	}
	return encodeBytes(enc), nil
}

func (e *NameEncoder) EstimatedSize(settingsCount int) int {
	var size int
	for k := range e.words {
		size += len(k) + 1
	}
	return size + e.MaxEncodedLength*settingsCount
}

type ValueEncoder struct {
	values         map[int64]int
	sortedValues   []int64
	constantValues map[string]int64
	min            int64
	max            int64
}

func NewValueEncoder(values []int64, constantValues map[string]int64) (*ValueEncoder, error) {
	m := make(map[int64]int)
	var min, max int64
	for _, v := range values {
		if v < min {
			min = v
		}
		if v > max {
			max = v
		}
		m[v]++
	}
	var sortedValues []int64
	for k := range m {
		sortedValues = append(sortedValues, k)
	}
	sort.SliceStable(sortedValues, func(i, j int) bool {
		return sortedValues[i] < sortedValues[j]
	})
	return &ValueEncoder{
		values:         m,
		sortedValues:   sortedValues,
		constantValues: constantValues,
		min:            min,
		max:            max,
	}, nil
}

func (e *ValueEncoder) MinValueType() (string, error) {
	boundaries := []int{8, 16, 32}
	for _, b := range boundaries {
		if float64(e.min) >= -math.Pow(2, float64(b-1)) {
			return fmt.Sprintf("int%d_t", b), nil
		}
	}
	return "", fmt.Errorf("cannot represent minimum value %d with int32_t", e.min)
}

func (e *ValueEncoder) MaxValueType() (string, error) {
	boundaries := []int{8, 16, 32}
	for _, b := range boundaries {
		if float64(e.max) < math.Pow(2, float64(b)) {
			return fmt.Sprintf("uint%d_t", b), nil
		}
	}
	return "", fmt.Errorf("cannot represent maximum value %d with uint32_t", e.max)
}

func (e *ValueEncoder) Values() []int64 {
	return e.sortedValues
}

func (e *ValueEncoder) RequiredIndexBytes() (int, error) {
	bits := math.Ceil(math.Log2(float64(len(e.values))))
	bytes := int(math.Ceil(bits / 8))
	if bytes > 1 {
		return 0, fmt.Errorf("too many bytes required for value index: %d", bytes)
	}
	return bytes, nil
}

func (e *ValueEncoder) EncodeValues(values ...*string) (string, error) {
	var buf bytes.Buffer
	for _, v := range values {
		sv := "0" // Default value
		if v != nil && *v != "" {
			sv = *v
		}
		// Check if it's a constant
		iv, ok := e.constantValues[sv]
		if !ok {
			// Parse it
			var err error
			iv, err = strconv.ParseInt(sv, 0, 64)
			if err != nil {
				return "", err
			}
		}
		pos := -1
		for ii, ev := range e.sortedValues {
			if ev == iv {
				pos = ii
				break
			}
		}
		if pos < 0 {
			return "", fmt.Errorf("value %q was not encoded", sv)
		}
		buf.WriteByte(byte(pos))
	}
	return encodeBytes(buf.Bytes()), nil
}

type Settings struct {
	Tables []*Table `yaml:"tables"`
	Groups []*Group `yaml:"groups"`
}

type SettingsGenerator struct {
	rootDir        string
	outputDir      string
	s              *Settings
	trueConditions map[string]struct{}
	tables         map[string]*Table
	hasBooleans    bool
	settingsCount  int
	// Types that need to be resolved
	pendingTypes map[*Member]*Group
	// Maximum unpacked name length, so
	// the C code can allocate an appropiately
	// sized buffer.
	maxNameLength  int
	nameEncoder    *NameEncoder
	constantValues map[string]int64
	valueEncoder   *ValueEncoder
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
	if err := g.checkConditions(); err != nil {
		return nil, err
	}
	if err := g.sanitizeFields(); err != nil {
		return nil, err
	}
	g.initilizeTableUsage()
	if err := g.initializeNameEncoder(); err != nil {
		return nil, err
	}
	if err := g.initializeValueEncoder(); err != nil {
		return nil, err
	}
	return g, nil
}

func (g *SettingsGenerator) SettingsCount() int {
	return g.settingsCount
}

func (g *SettingsGenerator) checkConditions() error {
	var buf bytes.Buffer
	g.addHeader(&buf, "platform.h")
	conds := make(map[string]struct{})
	addCondition := func(c string) {
		if c == "" {
			return
		}
		if _, found := conds[c]; found {
			return
		}
		conds[c] = struct{}{}
		fmt.Fprintf(&buf, "#ifdef %s\n", c)
		fmt.Fprintf(&buf, "#pragma message(%q)\n", c)
		buf.WriteString("#endif\n")
	}
	for _, gr := range g.s.Groups {
		addCondition(gr.Condition)
		for _, m := range gr.Members {
			addCondition(m.Condition)
		}
	}
	_, stderr, err := g.compileTestFile(&buf)
	if err != nil {
		return err
	}
	pragmaRe := regexp.MustCompile("#pragma message\\(\"(.*)\"\\)")
	matches := pragmaRe.FindAllStringSubmatch(string(stderr), -1)
	g.trueConditions = make(map[string]struct{})
	for _, m := range matches {
		g.trueConditions[m[1]] = struct{}{}
	}
	return nil
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
		if gr.Name == "" {
			return errors.New("empty group name")
		}
		for _, m := range gr.Members {
			if m.Name == "" {
				return fmt.Errorf("empty name in member in group %q", gr.Name)
			}
			if m.Table != "" {
				if g.tables[m.Table] == nil {
					return fmt.Errorf("field %q references non-existing table %q", m.Name, m.Table)
				}
			}
			if m.Min != nil && *m.Min == "" {
				m.Min = nil
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
	return g.forEachEnabledMember(func(_ *Group, m *Member) error {
		g.settingsCount++
		if nl := len(m.Name); nl > g.maxNameLength {
			g.maxNameLength = nl
		}
		return nil
	})
}

func (g *SettingsGenerator) addHeader(w io.Writer, h string) {
	fmt.Fprintf(w, "#include \"%s\"\n", h)
}

func (g *SettingsGenerator) compileTestFile(r io.Reader) (stdout []byte, stderr []byte, err error) {
	data, err := ioutil.ReadAll(r)
	if err != nil {
		return nil, nil, err
	}
	tmpDir, err := ioutil.TempDir("", "")
	if err != nil {
		return nil, nil, err
	}
	defer os.RemoveAll(tmpDir)

	tmpFile := filepath.Join(tmpDir, "test.cpp")
	if err := ioutil.WriteFile(tmpFile, data, 0644); err != nil {
		return nil, nil, err
	}

	cflags := strings.Split(os.Getenv("CFLAGS"), " ")
	var args []string
	for _, flag := range cflags {
		// Don't generate temporary files
		if flag == "-MMD" || flag == "-MP" || strings.HasPrefix(flag, "-save-temps") {
			continue
		}
		if strings.HasPrefix(flag, "-std") {
			flag = "-std=c++11"
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
	var stdoutBuf bytes.Buffer
	cmd.Stdout = &stdoutBuf
	var stderrBuf bytes.Buffer
	cmd.Stderr = &stderrBuf
	cmd.Run()
	return stdoutBuf.Bytes(), stderrBuf.Bytes(), nil
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

	_, stderr, err := g.compileTestFile(&buf)
	if err != nil {
		return err
	}

	sc := bufio.NewScanner(bytes.NewReader(stderr))
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
				return fmt.Errorf("no member found at line %d, compiler output: %s", line, string(stderr))
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

func (g *SettingsGenerator) initializeNameEncoder() error {
	var best *NameEncoder
	var names []string
	err := g.forEachEnabledMember(func(_ *Group, m *Member) error {
		names = append(names, m.Name)
		return nil
	})
	if err != nil {
		return err
	}
	for ii := 3; ii < 7; ii++ {
		enc := &NameEncoder{
			MaxEncodedLength: ii,
			Names:            names,
		}
		if err := enc.Initialize(); err != nil {
			return err
		}
		if best == nil || best.EstimatedSize(g.settingsCount) > enc.EstimatedSize(g.settingsCount) {
			best = enc
		}
	}
	g.nameEncoder = best
	return nil
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

func (g *SettingsGenerator) initializeValueEncoder() error {
	var values []int64
	var constants []string
	addValue := func(sval string) error {
		r, sz := utf8.DecodeRuneInString(sval)
		if sz == 0 {
			return fmt.Errorf("invalid rune decoded in value %q", sval)
		}
		if unicode.IsDigit(r) || r == '-' {
			val, err := strconv.ParseInt(sval, 10, 64)
			if err != nil {
				return err
			}
			values = append(values, val)
		} else {
			constants = append(constants, sval)
		}
		return nil
	}
	err := g.forEachEnabledMember(func(gr *Group, m *Member) error {
		if m.Min != nil && *m.Min != "" {
			if err := addValue(*m.Min); err != nil {
				return err
			}
		}
		if m.Max != nil && *m.Max != "" {
			if err := addValue(*m.Max); err != nil {
				return err
			}
		}
		return nil
	})
	var constantValues map[string]int64
	if len(constants) > 0 {
		// Since we're relying on errors rather than
		// warnings to find these constants, the compiler
		// might reach the maximum number of errors and stop
		// compilation, so we might need multiple passes.
		uniqueConstants := make(map[string]struct{})
		for _, v := range constants {
			uniqueConstants[v] = struct{}{}
		}
		re := regexp.MustCompile("required from 'class expr_(.*?)<(.*)ll>'")
		constantValues = make(map[string]int64)
		for len(constantValues) < len(uniqueConstants) {
			var buf bytes.Buffer
			g.addHeader(&buf, "platform.h")
			for _, gr := range g.s.Groups {
				for _, h := range gr.Headers {
					g.addHeader(&buf, h)
				}
			}
			buf.WriteString(`template <int64_t V> class Fail
				{
					static_assert(V == 42 && 0 == 1, "FAIL");
					public:
						Fail() { };
						int64_t v = V;
				};
				`)

			ii := 0
			for v := range uniqueConstants {
				if _, found := constantValues[v]; found {
					continue
				}
				cls := fmt.Sprintf("expr_%s", v)
				fmt.Fprintf(&buf, "template <int64_t V> class %s: public Fail<V> {};\n", cls)
				fmt.Fprintf(&buf, "%s<%s> var_%d;\n", cls, v, ii)
				ii++
			}
			_, stderr, err := g.compileTestFile(&buf)
			if err != nil {
				return err
			}
			matches := re.FindAllStringSubmatch(string(stderr), -1)
			if len(matches) == 0 {
				fmt.Println(string(stderr))
				return fmt.Errorf("no more matches")
			}
			for _, m := range matches {
				c := m[1]
				v := m[2]
				val, err := strconv.ParseInt(v, 0, 64)
				if err != nil {
					return fmt.Errorf("error parsing value for %s: %v", c, err)
				}
				constantValues[c] = val
			}
		}
		for _, c := range constants {
			v, ok := constantValues[c]
			if !ok {
				return fmt.Errorf("could not resolve constant %q", c)
			}
			values = append(values, v)
		}
	}

	enc, err := NewValueEncoder(values, constantValues)
	if err != nil {
		return err
	}
	g.valueEncoder = enc
	return nil
}

func (g *SettingsGenerator) forEachEnabledGroup(f func(*Group) error) error {
	for _, gr := range g.s.Groups {
		if gr.Condition != "" {
			if _, found := g.trueConditions[gr.Condition]; !found {
				continue
			}
		}
		if err := f(gr); err != nil {
			return err
		}
	}
	return nil
}

func (g *SettingsGenerator) enabledMemberCount(gr *Group) int {
	var count int
	for _, m := range gr.Members {
		if m.Condition != "" {
			if _, found := g.trueConditions[m.Condition]; !found {
				continue
			}
		}
		count++
	}
	return count
}

func (g *SettingsGenerator) forEachEnabledMember(f func(*Group, *Member) error) error {
	for _, gr := range g.s.Groups {
		if gr.Condition != "" {
			if _, found := g.trueConditions[gr.Condition]; !found {
				continue
			}
		}
		for _, m := range gr.Members {
			if m.Condition != "" {
				if _, found := g.trueConditions[m.Condition]; !found {
					continue
				}
			}
			if err := f(gr, m); err != nil {
				return err
			}
		}
	}
	return nil
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

func (g *SettingsGenerator) CanUseByteOffsetoff() bool {
	var buf bytes.Buffer
	g.addHeader(&buf, "cstddef")
	g.forEachEnabledGroup(func(gr *Group) error {
		for _, h := range gr.Headers {
			g.addHeader(&buf, h)
		}
		return nil
	})
	g.forEachEnabledMember(func(gr *Group, m *Member) error {

		fmt.Fprintf(&buf, "static_assert(offsetof(%s, %s) < 255, \"%s.%s is too big\");\n",
			gr.Type, m.Field, gr.Type, m.Field)
		return nil
	})
	_, stderr, _ := g.compileTestFile(&buf)
	return !strings.Contains(string(stderr), "static assertion failed")
}

func (g *SettingsGenerator) writeHeaderFile() error {
	var buf bytes.Buffer
	buf.WriteString("#pragma once\n")
	// Write clivalue_t size constants
	fmt.Fprintf(&buf, "#define CLIVALUE_MAX_NAME_LENGTH %d\n", g.maxNameLength+1) // +1 for the terminating '\0'
	fmt.Fprintf(&buf, "#define CLIVALUE_ENCODED_NAME_MAX_BYTES %d\n", g.nameEncoder.MaxEncodedLength)
	if g.nameEncoder.UsesDirectIndexing() {
		buf.WriteString("#define CLIVALUE_ENCODED_NAME_USES_DIRECT_INDEXING\n")
	}
	fmt.Fprintf(&buf, "#define CLIVALUE_TABLE_COUNT %d\n", g.SettingsCount())
	cliValueOffsetType := "uint16_t"
	if g.CanUseByteOffsetoff() {
		cliValueOffsetType = "uint8_t"
	}
	fmt.Fprintf(&buf, "typedef %s clivalue_offset_t;\n", cliValueOffsetType)
	var pgnCount int
	err := g.forEachEnabledGroup(func(g *Group) error {
		pgnCount++
		return nil
	})
	if err != nil {
		return err
	}
	fmt.Fprintf(&buf, "#define CLIVALUE_PGN_COUNT %d\n", pgnCount)
	// Write type definitions and constants for min/max values
	minType, err := g.valueEncoder.MinValueType()
	if err != nil {
		return err
	}
	fmt.Fprintf(&buf, "typedef %s clivalue_min_t;\n", minType)
	maxType, err := g.valueEncoder.MaxValueType()
	if err != nil {
		return err
	}
	fmt.Fprintf(&buf, "typedef %s clivalue_max_t;\n", maxType)
	requiredIndexBytes, err := g.valueEncoder.RequiredIndexBytes()
	if err != nil {
		return err
	}
	fmt.Fprintf(&buf, "#define CLIVALUE_MIN_MAX_INDEX_BYTES %d\n", requiredIndexBytes*2)
	// Write lookup table constants
	tableNames := g.orderedTableNames()
	buf.WriteString("enum {\n")
	for _, k := range tableNames {
		tbl := g.tables[k]
		if !tbl.Enabled(g.trueConditions) {
			continue
		}
		fmt.Fprintf(&buf, "\t%s,\n", tbl.ConstantName())
	}
	buf.WriteString("\tLOOKUP_TABLE_COUNT,\n")
	buf.WriteString("};\n")
	// Write table pointers
	for _, k := range tableNames {
		tbl := g.tables[k]
		if !tbl.Enabled(g.trueConditions) {
			continue
		}
		fmt.Fprintf(&buf, "extern const char *%s[];\n", tbl.VarName())
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
	// Write PGN arrays
	var pgnSteps []int
	var pgns []string
	err := g.forEachEnabledGroup(func(gr *Group) error {
		pgnSteps = append(pgnSteps, g.enabledMemberCount(gr))
		pgns = append(pgns, gr.Name)
		return nil
	})
	if err != nil {
		return err
	}
	buf.WriteString("const pgn_t cliValuePgn[] = {\n")
	for _, pgn := range pgns {
		fmt.Fprintf(&buf, "\t%s,\n", pgn)
	}
	buf.WriteString("};\n")
	buf.WriteString("const uint8_t cliValuePgnCounts[] = {\n")
	for _, v := range pgnSteps {
		fmt.Fprintf(&buf, "\t%d,\n", v)
	}
	buf.WriteString("};\n")
	// Write word list
	buf.WriteString("static const char *cliValueWords[] = {\n")
	buf.WriteString("\tNULL,\n")
	for _, w := range g.nameEncoder.WordList() {
		fmt.Fprintf(&buf, "\t%q,\n", w)
	}
	buf.WriteString("};\n")

	// Write the tables
	tableNames := g.orderedTableNames()
	for _, k := range tableNames {
		tbl := g.tables[k]
		if !tbl.Enabled(g.trueConditions) {
			continue
		}
		fmt.Fprintf(&buf, "const char *%s[] = {\n", tbl.VarName())
		for _, v := range tbl.Values {
			fmt.Fprintf(&buf, "\t%q,\n", v)
		}
		buf.WriteString("};\n")
	}

	buf.WriteString("const lookupTableEntry_t cliLookupTables[] = {\n")
	for _, k := range tableNames {
		tbl := g.tables[k]
		if !tbl.Enabled(g.trueConditions) {
			continue
		}
		fmt.Fprintf(&buf, "\t{ %s, sizeof(%s) / sizeof(char*) },\n", tbl.VarName(), tbl.VarName())
	}

	buf.WriteString("};\n")

	// Write min/max values table
	buf.WriteString("const uint32_t cliValueMinMaxTable[] = {\n")
	for _, v := range g.valueEncoder.Values() {
		fmt.Fprintf(&buf, "\t%d,\n", v)
	}
	buf.WriteString("};\n")

	indexBytes, err := g.valueEncoder.RequiredIndexBytes()
	if err != nil {
		return err
	}
	switch indexBytes {
	case 1:
		buf.WriteString("typedef uint8_t clivalue_min_max_idx_t;\n")
		buf.WriteString("#define CLIVALUE_INDEXES_GET_MIN(val) (val->config.minmax.indexes[0])\n")
		buf.WriteString("#define CLIVALUE_INDEXES_GET_MAX(val) (val->config.minmax.indexes[1])\n")
	default:
		return fmt.Errorf("can't encode indexed values requiring %d bytes", indexBytes)
	}

	// Write clivalues
	buf.WriteString("const clivalue_t cliValueTable[] = {\n")

	var lastGroup *Group
	err = g.forEachEnabledMember(func(gr *Group, m *Member) error {
		if gr != lastGroup {
			fmt.Fprintf(&buf, "\t// %s\n", gr.Name)
			lastGroup = gr
		}
		enc, err := g.nameEncoder.FormatEncodedName(m.Name)
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
			encoded, err := g.valueEncoder.EncodeValues(m.Min, m.Max)
			if err != nil {
				return err
			}
			fmt.Fprintf(&buf, ", .config.minmax.indexes = %s", encoded)
		}
		fmt.Fprintf(&buf, ", offsetof(%s, %s) },\n", gr.Type, m.Field)
		return nil
	})
	if err != nil {
		return err
	}
	buf.WriteString("};\n")
	return ioutil.WriteFile(filepath.Join(g.outputDir, "settings_generated.c"), buf.Bytes(), 0644)
}

func (g *SettingsGenerator) WriteFiles() error {
	if err := g.writeHeaderFile(); err != nil {
		return err
	}
	return g.writeImplementationFile()
}

func (g *SettingsGenerator) PrintStats() {
	fmt.Println(g.SettingsCount(), "settings")
	fmt.Printf("word table has %d entries\n", g.nameEncoder.NWords())
	fmt.Printf("each setting name uses %d bytes\n", g.nameEncoder.MaxEncodedLength)
	fmt.Printf("%d bytes estimated for setting name storage\n", g.nameEncoder.EstimatedSize(g.SettingsCount()))
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
	if err := gen.WriteFiles(); err != nil {
		panic(err)
	}

	gen.PrintStats()
	gen.PrintWarnings()
}
