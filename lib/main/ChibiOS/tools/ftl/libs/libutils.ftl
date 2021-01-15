[#ftl]
[#--
    ChibiOS/RT - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
  --]

[#--
  -- Returns the trimmed text "s" making sure it is terminated by a dot.
  -- The empty string is always returned as an empty string, the dot is not
  -- added.
  --]
[#function WithDot s]
  [#local s = s?trim /]
  [#if s == ""]
    [#return s /]
  [/#if]
  [#if s?ends_with(".")]
    [#return s /]
  [/#if]
  [#return s + "." /]
[/#function]

[#--
  -- Returns the trimmed text "s" making sure it is not terminated by a dot.
  --]
[#function WithoutDot s]
  [#local s = s?trim /]
  [#if s?ends_with(".")]
    [#return s?substring(0, s?length - 1) /]
  [/#if]
  [#return s /]
[/#function]

[#--
  -- Returns the trimmed text "s" making sure it is terminated by a dot if the
  -- text is composed of multiple phrases, if the text is composed of a single
  -- phrase then makes sure it is *not* terminated by a dot.
  -- A phrase is recognized by the pattern ". " into the text.
  -- The empty string is always returned as an empty string, the dot is never
  -- added.
  --]
[#function IntelligentDot s]
  [#local s = s?trim /]
  [#if s?contains(". ")]
    [#return WithDot(s) /]
  [/#if]
  [#return WithoutDot(s) /]
[/#function]

[#--
  -- Formats a text string in a sequence of strings no longer than "len" (first
  -- line) or "lenn" (subsequent lines).
  -- White spaces are normalized between words, sequences of white spaces become
  -- a single space.
  --]
[#function StringToText len1 lenn s]
  [#local words=s?word_list /]
  [#local line="" /]
  [#local lines=[] /]
  [#list words as word]
    [#if lines?size == 0]
      [#local len = len1 /]
    [#else]
      [#local len = lenn /]
    [/#if]
    [#if (line?length + word?length + 1 > len)]
      [#local lines = lines + [line?trim] /]
      [#local line = word + " " /]
    [#else]
      [#local line = line + word + " " /]
    [/#if]
  [/#list]
  [#if line != ""]
    [#local lines = lines + [line?trim] /]
  [/#if]
  [#return lines /]
[/#function]

[#--
  -- Emits a string "s" as a formatted text, the first line is prefixed by the
  -- "p1" parameter, subsequent lines are prefixed by the "pn" paramenter.
  -- Emitted lines are no longer than the "len" parameter.
  -- White spaces are normalized between words.
  --]
[#macro FormatStringAsText p1 pn s len]
  [#local lines = StringToText(len - p1?length, len - pn?length, s) /]
  [#list lines as line]
    [#if line_index == 0]
${p1}${line}
    [#else]
${pn}${line}
    [/#if]
  [/#list]
[/#macro]

[#--
  -- Emits a C function body code reformatting the indentation using the
  -- specified tab size and line prefix.
  --]
[#macro EmitIndentedCCode start tab ccode]
  [#assign lines = ccode?string?split("^", "rm") /]
  [#list lines as line]
    [#if (line_index > 0) || (line?trim?length > 0)]
      [#if line?trim?length > 0]
        [#if line[0] == "#"]
${line?chop_linebreak}
        [#else]
${start + line?chop_linebreak}
        [/#if]
      [#else]

      [/#if]
    [/#if]
  [/#list]
[/#macro]
