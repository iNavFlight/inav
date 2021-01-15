#!/usr/bin/perl
use strict;
use warnings;
use File::Basename;

# Desired indentation.
my $indentation = 2;

if ($#ARGV != 0) {
  print "\nUsage: stylecheck.pl source\n";
  exit;
}

my $source = $ARGV[0];

open(my $in,  "<", $source) or die "Can't open source: $!";

my $lineno = 0;
my @c_source = <$in>;
my $filename = $source;
$filename =~ y/\\/\//;
$filename = basename($filename);

my $i_level   = 0;
my $c_comment = "";
my $state     = "start";
my $cr        = "\r";
my $lf        = "\n";
my $tab       = "\t";
my $f_lineno  = 0;
my $f_params  = "";
my $t_lineno  = 0;
my $t_type    = "";
my $e_lineno  = 0;
my $e_name    = "";
my $d_lineno  = 0;
my $d_name    = "";
my $d_name2    = "";
my $d_name_app = "";
my $scope     = "";

sub style {
  my $desc = shift;

  print("style: $desc at line $lineno in \"$filename\"\n");
}

sub error {
  my $desc = shift;

  print("error: $desc at line $lineno in \"$filename\"\n");
}

# Indentation check.
sub check_indentation {

  shift =~ /^(\s*)/;
  if (length($1) != $i_level) {
    style "indentation violation";
  }
}

my $emptycnt = 0;
foreach my $line (@c_source) {

  $lineno += 1;

  #****************************************************************************
  # Check on EOL.
  if (not ($line =~ /$cr$lf$/)) {
    error "detected malformed EOL";
  }
  $line =~ s/$cr//;
  $line =~ s/$lf//;

  #****************************************************************************
  # Check on trailing spaces.
  if ($line =~ /\s$/) {
    style "detected trailing spaces";
  }

  #****************************************************************************
  # Check on TABs.
  if ($line =~ /$tab/) {
    style "detected TAB";
    $line =~ s/$tab/    /;
  }

  #****************************************************************************
  # Check on empty lines.
  my $tmp = $line;
  $tmp =~ s/\s//;
  if (length($tmp) == 0) {
    $emptycnt = $emptycnt + 1;
    if ($emptycnt == 2) {
      style "detected multiple empty lines"
    }
    next;
  }
  else {
    $emptycnt = 0;
  }

  #****************************************************************************
  # Stripping strings content for ease of parsing, all strings become _string_.
  $line =~ s/\\\"//;
  if ($line =~ s/(\"[^"]*\")/_string_/) {
#    print "string: $1 replaced by _string_\n";
  }

  #******************************************************************************
  # State machine handling.
  if ($state eq "start") {
    # Searching for a global code element or a comment start.

    # Indentation check.
    check_indentation $line;

    #******************************************************************************
    # Functions matching, triggered by the "(".
    if ($line =~ /^(static|)\s*(struct|union|)\s*([a-zA-Z_][a-zA-Z0-9_]*\s*[*]*)\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*\(/) {
      # $1=scope $2$3=return type $4=name
      $line =~ s/^(static|)\s*(struct|union|)\s*([a-zA-Z_][a-zA-Z0-9_]*\s*[*]*)\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*\(//;
#      print "function: " . $1 . " " . $2 . " " . $3 . " " . $4 . "(";

      # Size of the match up to parameters.
      my $size = $+[0] - $-[0];

      # Function line number.
      $f_lineno = $lineno;
      
      # Checking if all parameters are on the same line.
      if ($line =~ /.*\)\s*{\s*$/) {
        $line =~ s/\)\s*{\s*$//;
#        print $line . "\n";
        $f_params = $line;
        $i_level = $indentation;
        $state = "infunc";
      }
      else {
#        print $line;
        $f_params = $line;
        $i_level = $size;
        $state = "inparams";
      }
    }
    #******************************************************************************
    # Structures matching.
    elsif ($line =~ /^\s*struct\s+([a-zA-Z_][a-zA-Z0-9_]*)/) {
    }
    #******************************************************************************
    # Single line "typedef" matching.
    elsif ($line =~ /^\s*typedef\s+([\sa-zA-Z0-9_]*\*+)\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*;\s*$/ or
           $line =~ /^\s*typedef\s+([\sa-zA-Z0-9_]*)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*;\s*$/) {
    }
    #******************************************************************************
    # Single line "typedef function" matching.
    elsif ($line =~ /^\s*typedef\s+.*\(\s*\*\s*([a-zA-Z0-9_]*)\s*\)\(/) {
    }
    #******************************************************************************
    # Multi line "typedef struct" matching.
    elsif ($line =~ /^\s*typedef\s*struct\s*([a-zA-Z_][a-zA-Z0-9_]*|\s?)?/) {
      $t_lineno = $lineno;
      $t_type   = "struct " . $1;
      $state = "intypedef";
    }
    #******************************************************************************
    # Multi line "typedef enum" matching.
    elsif ($line =~ /^\s*typedef\s*enum\s*([a-zA-Z_][a-zA-Z0-9_]*|\s?)?/) {
      $t_lineno = $lineno;
      $t_type   = "enum " . $1;
      if ($line =~ /([a-zA-Z_][a-zA-Z0-9_]*)\s*;\s*$/) {
        # Special case of a single-line typedef enum.
      }
      else {
        $state = "intypedef";
      }
    }
    #******************************************************************************
    # Multi line "enum" matching.
    elsif ($line =~ /^\s*enum\s*([a-zA-Z_][a-zA-Z0-9_]*)/) {
      $e_name = $1;
      $e_lineno = $lineno;
      if ($line =~ /;\s*$/) {
        # Special case of a single-line enum.
      }
      else {
        $state = "inenum";
      }
    }
    #******************************************************************************
    # Struct variable matching.
    elsif ($line =~ /^\s*(static|)\s+([\sa-zA-Z0-9_]*)\s+([a-zA-Z_][a-zA-Z0-9_]*\[.*\]|[a-zA-Z_][a-zA-Z0-9_]*)\s*(;\s*$|=)/ or
           $line =~ /^\s*(static|)\s+([\sa-zA-Z0-9_]*\*+)\s*([a-zA-Z_][a-zA-Z0-9_]*\[.*\]|[a-zA-Z_][a-zA-Z0-9_]*)\s*(;\s*$|=)/) {
    }
    #******************************************************************************
    # Variable matching.
    elsif ($line =~ /^\s*(static|).*\s+([a-zA-Z_][a-zA-Z0-9_]*\s*\*+|[a-zA-Z_][a-zA-Z0-9_]*)\s*([a-zA-Z_][a-zA-Z0-9_]*\[.*\]|[a-zA-Z_][a-zA-Z0-9_]*)\s*(;\s*$|=)/) {
      # variable declaration.
    }
    #******************************************************************************
    # #nclude matching.
    elsif ($line =~ /^\s*#include\s+"([^"]+)"/) {
    }
    #******************************************************************************
    # #if matching.
    elsif ($line =~ /^\s*#if\s+(.+)/) {
    }
    #******************************************************************************
    # #ifdef matching.
    elsif ($line =~ /^\s*#ifdef\s+([\w_]+)/) {
    }
    #******************************************************************************
    # #define matching.
    elsif ($line =~ /^\s*#define\s+([\w_]+)\s*(.*)/) {
      $d_lineno = $lineno;
      $d_name   = $1;
      $d_name2   = $2;
      # enum typedef declaration.
      if ($line =~ /[^\\]$/) {
        # Special case of a single-line typedef enum.
      }
      else {
        $state = "indefine";
      }
    }
    #******************************************************************************
    # Comment start matching.
    elsif ("$line" =~ /^\s*\/\*/) {
      if ("$line" =~ /\*\//) {
        # Special case of single line comments.
        $line =~ /^\s*(\/\*.*\*\/)/;
      }
      else {
        # Start of multi-line comment.
        $line =~ /(\/\*.*)/;
        $c_comment = $1 . " ";
        $state = "incomment";
      }
    }
  }
  #******************************************************************************
  # Scanning for function parameters end and function body begin.
  elsif ($state eq "inparams") {

    # Indentation check.
    check_indentation $line;

    if ($line =~ /.*\)\s*{\s*$/) {
#      print $line . "\n";
      $line =~ s/\)\s*{\s*$//;
      $f_params = $f_params . $line;
      $i_level = $indentation;
      $state = "infunc";
      print "$f_params\n";
    }
    else {
      $f_params = $f_params . $line;
#      print $line;
    }
  }
  #******************************************************************************
  # Scanning for function end.
  elsif ($state eq "infunc") {
    
    # Checking for function end, the final "}".
    if ($line =~ /^\}/) {
      $i_level = 0;
      $state = "start";
      next;
    }
    
    # Indentation check.
    check_indentation $line;

    if ($line =~ /(\/\*.*\*\/)/) {
      # Single line comments inside functions.
    }
    elsif ("$line" =~ /(\/\*.*)/) {
      # Start of multi-line comment inside a function.
      $c_comment = $1 . " ";
      $state = "infunccomment";
    }
  }
  #******************************************************************************
  # Scanning for comment end within a function body.
  elsif ($state eq "infunccomment") {
    if ("$line" =~ /\*\/\s*$/) {
      # End of mult-line comment.
      $line =~ /(.*\*\/)/;
      $c_comment .= $1;

      $state = "infunc";
    }
    else {
      # Add the whole line.
      $c_comment .= $line . " ";
    }
  }
  #******************************************************************************
  # Scanning for comment end.
  elsif ($state eq "incomment") {
    if ("$line" =~ /\*\/\s*$/) {
      # End of mult-line comment.
      $line =~ /(.*\*\/)/;
      $c_comment .= $1;

      $state = "start";
    }
    else {
      # Add the whole line.
      $c_comment .= $line . " ";
    }
  }
  #******************************************************************************
  # Scanning for typedef end.
  elsif ($state eq "intypedef") {
    if ($line =~ /^\s*}\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*;\s*$/) {
      # typedef end because the final '} <name>;'.

      $state = "start";
    }
  }
  #******************************************************************************
  # Scanning for enum end.
  elsif ($state eq "inenum") {
    if ($line =~ /^\s*}\s*;\s*$/) {
      # enum end because the final '};'.

      $state = "start";
    }
  }
  #******************************************************************************
  # Scanning for define end.
  elsif ($state eq "indefine") {
    if ($line =~ /[^\\]$/) {
      # define end because the final 'not \'.
      # remove blank from starting of the line
      $line =~ s/^\s+|\s+$//g;
      # Add the whole line.
      $d_name2 .= $line;

      $state = "start";
    }
    else {
      # Add the whole line.
      # $line =~ s/^\s+|\s+$//g;
      $line =~ s/^\s*(.*?)\s*$/$1/;
      $d_name2 .= $line;
    }
  }
}

close $in or die "$in: $!";
