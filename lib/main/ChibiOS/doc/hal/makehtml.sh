#!/bin/bash
rm html/*
doxygen Doxyfile_html
rm html/*.md5
rm html/*.map

