#! /usr/bin/perl -w
#
# Copyright 2004-2005 Oswald Buddenhagen <ossi@kde.org>
#
# Permission to use, copy, modify, distribute, and sell this software and its
# documentation for any purpose is hereby granted without fee, provided that
# the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation.
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# Except as contained in this notice, the name of a copyright holders shall
# not be used in advertising or otherwise to promote the sale, use or
# other dealings in this Software without prior written authorization
# from the copyright holders.
#

use strict;

sub pegout($)
{
  print STDERR $_[0]."\n";
  exit 1;
}

sub relpath($$)
{
  my @src = split(/\//, shift);
  my @dst = split(/\//, shift);
  pop @dst;
  while (@src && @dst && $src[0] eq $dst[0]) {
    shift @src;
    shift @dst;
  }
  return "../"x@dst . join("/", @src);
}

sub getl()
{
  while (<INFILE>) {
    next if (/^#/);
    chop;
#    print "read: ".$_."\n";
    return;
  }
  $_ = "";
}

sub dedb($)
{
  my $t = shift;
  $t =~ s,</?(command|guilabel|quote)>,\",g;
  $t =~ s,</?(acronym|envar|filename|option|systemitem( [^ >]+)?)>,,g;
  $t =~ s,<emphasis>([^<]+)</emphasis>,uc($1),ge;
  $t =~ s,&nbsp;, ,g;
  $t =~ s,&lt;,<,g;
  $t =~ s,&gt;,>,g;
  $t =~ s,&kdm;,KDM,g;
  $t =~ s,&XDMCP;,XDMCP,g;
  $t =~ s,&X-Server;,X-server,g;
  return $t;
}

sub mkvname($)
{
  my $v = shift;
  if ($v !~ /^[A-Z]{2}/) {
    $v = lcfirst $v;
  }
  return $v;
}

sub emit_conds($)
{
  my $ret = "";
  for my $c (keys %{$_[0]}) {
    my ($then, $else) = ("", "");
    for my $d (@{${$_[0]}{$c}}) {
      my $bas = "# define ".$d->[0];
      if ($d->[1] =~ /\n/) {
        $then .= $bas." \\\n".$d->[1]."\n";
      } else {
        $then .= $bas." ".$d->[1]."\n";
      }
      $else .= $bas."\n";
    }
    $ret .= "#if ".$c."\n".$then."#else\n".$else."#endif\n\n";
  }
  return $ret;
}

sub add_cond($$$$)
{
  if ($_[0]) {
    my $vn = uc($_[1]);
    for my $i (@{$_[2]}) {
      push @{${$_[3]}{$_[0]}}, [ $vn."_".$i->[1], $i->[0] ];
      $i->[0] = $vn."_".$i->[1];
    }
  }
}

my $do_doc = 0;
if ($ARGV[0] eq "--doc") {
 $do_doc = 1;
 shift @ARGV;
}

@ARGV != 2 && pegout("usage: $0 [--doc] <def-file> <out-file>");

open (INFILE, $ARGV[0]) || pegout("$0: cannot open definition file ".$ARGV[0]);

my %ex_conds = ();
my %ex_sects = ();  # $name -> $index
my @ex_config = ();  # ($name, $comment, $entries)

my $raw_out = "";

my %ov_enum_conds = ();
my $ov_enums = "";
my $ov_enum_defs = "";

my $ov_defaults = "";

my %arr_ov_vars = ();
my $ov_rd_sects = "";
my $ov_rd_ents = "";

my $ov_gen_sects = "";
my $ov_gen_ents = "";
my $max_prio = 0;

my %ov_sec_conds = ();
my %ov_ent_conds = ();
my $ov_sect_defs = "";
my $ov_sect_refs = "";

my %ov_glob_conds = ();
my $ov_globs = "";
my %ov_loc_conds = ();
my $ov_locs = "";

my %ov_glob_decl_conds = ();
my $ov_glob_decls = "";
my %ov_glob_def_conds = ();
my %ov_glob_defs = ();
my %ov_loc_def_conds = ();
my %ov_loc_defs = ();

my %ov_greet_conds = ();
my $ov_greet_init = "";
my @ov_greet_decls = ();
my %ov_greet_defs = ();

my %ov_xm_conds = ();
my @ov_xm = ("", "");

my %ov_km_conds = ();
my %ov_km = ();

my %sect_names = ();
my $sect = "";
my $sect_if;

my $key_if;

my %key_names;

my $kid_seq = 0x1000;

my $doc = "";
my $doc_ref = "";

sub emit_section()
{
  my $ts = $sect;
  $ts =~ s/-/_/;
  my @oa = (
    ["static Ent ents".$ts."[] = { \\\n".$ov_rd_ents."};", "ENTS"],
    ["static Ent ents".$ts."[] = { \\\n".$ov_gen_ents."};", "GENS"],
    ["sec".$ts." = { \"".$sect."\", ents".$ts.", as(ents".$ts.") },", "SEC"],
    ["&sec".$ts.",", "SECS"]
  );
  $ov_rd_ents = "";
  $ov_gen_ents = "";
  add_cond($sect_if, $ts, \@oa, \%ov_sec_conds);
  $ov_rd_sects .= " \\\n".$oa[0][0]." \\\n";
  $ov_gen_sects .= " \\\n".$oa[1][0]." \\\n";
  $ov_sect_defs .= "    ".$oa[2][0]." \\\n";
  $ov_sect_refs .= "\t".$oa[3][0]." \\\n";
  $doc_ref .= "</variablelist>\n</sect2>\n\n";
}

my %th = (
  "int" => [ "C_TYPE_INT", "", "int\t", "", "GetCfgInt", "" ],
  "bool" => [ "C_TYPE_INT", " | C_BOOL", "int\t", "bool\t", "GetCfgInt", "GetCfgInt" ],
  "enum" => [ "C_TYPE_INT", " | C_ENUM", "int\t", "", "GetCfgInt", "" ],
  "group" => [ "C_TYPE_INT", " | C_GRP", "int\t", "", "GetCfgInt", "" ],
  "string" => [ "C_TYPE_STR", "", "char\t*", "QString\t", "GetCfgStr", "GetCfgQStr" ],
  "path" => [ "C_TYPE_STR", " | C_PATH", "char\t*", "QString\t", "GetCfgStr", "GetCfgQStr" ],
  "list" => [ "C_TYPE_ARGV", "", "char\t**", "QStringList\t", "GetCfgStrArr", "GetCfgQStrList" ]
);

my @tl = ("QFont\t", "QStringList\t", "QString\t", "char\t**", "char\t*", "int\t", "bool\t");

sub init_defs($)
{
  for my $t (@tl) {
    $_[0]{$t} = "";
  }
}

init_defs(\%ov_glob_defs);
init_defs(\%ov_loc_defs);
init_defs(\%ov_greet_defs);

sub emit_defs($)
{
  my $ret = "";
  for my $t (@tl) {
    $ret .= $_[0]{$t};
  }
  return $ret;
}

while (<INFILE>) {
  chop;
  if (/^<code>$/) {
    while (<INFILE>) {
      last if (/^<\/code>\n$/);
      $raw_out .= $_;
    }
  } elsif (/^<kdmrc>$/) {
    my $comm = "";
    while (<INFILE>) {
      last if (/^<\/kdmrc>\n$/);
      chop;
      if (/^\[(.*)\]$/) {
        defined($ex_sects{$1}) &&
          pegout("redefinition of example section [$1]");
        push @ex_config, [$1, dedb($comm), "", ""];
        $ex_sects{$1} = $#ex_config;
        $comm = "";
      } else {
        if (!$_) {
          $comm .= " \\\n\"\\n\"";
        } elsif ($_ eq " _") {
          $comm .= " \\\n\"#\\n\"";
        } else {
          s/"/\\"/g;
          $comm .= " \\\n\"#".$_."\\n\"";
        }
      }
    }
  } elsif (/^<docu>$/) {
    while (<INFILE>) {
      last if (/^<\/docu>\n$/);
      $doc .= $_;
    }
  } elsif (/^<legacy>$/) {
    while (<INFILE>) {
      next if (/^($|#)/);
      my ($proc, $kif);
      if (/^If: (.+)$/) {
        $kif = $1;
        getl();
      } else {
        $kif = "";
      }
      if (/^Proc: (.+)$/) {
        $proc = $1;
        getl();
      } else {
        pegout("expecting Proc keyword in legacy section");
      }
      my $nsrc = 0;
      my $mcnt = 0;
      while (/^Source: (.+)$/) {
        my $src = $1;
        if ($src =~ /^xdm:(.*)$/) {
          my $what = $1;
          my $dsp = ($what =~ s/^\*\.//);
          my @oa = ([ "{ \"".$what."\", (char *)-1, 0, ".$proc." },", "XMO" ]);
          add_cond($kif, $what, \@oa, \%ov_xm_conds);
          $ov_xm[$dsp] .= $oa[0][0]." \\\n";
        } elsif ($src =~ /^kdm:(.*)\/(.*)$/) {
          my ($sec, $key) = ($1, $2);
          my @oa = ([ "{ \"".$key."\", (char *)-1, 0, ".$proc." },", "KMO".($mcnt++) ]);
          add_cond($kif, $key, \@oa, \%ov_km_conds);
          $ov_km{$sec} .= $oa[0][0]." \\\n";
        } else {
          pegout("invalid legacy option '$_'");
        }
        $nsrc++;
        getl();
      }
      $nsrc || pegout("no sources for legacy processor ".$proc);
      last if (/^<\/legacy>$/);
      pegout("unidentified section body '".$_."' in legacy section") if ($_);
    }
  } else {
    next if (/^($|#)/);
    if (/^Key: (.+)$/) {
      my $key = $1;
      $sect || pegout("defining key ".$key." outside any section");
      defined($key_names{$key}) &&
        pegout("redefinition of key ".$key." in section [".$sect."]");
      $key_names{$key} = "";
      getl();
      if (/^If: (.+)$/) {
        $key_if = $1;
        getl();
      } else {
        $key_if = "";
      }
      my $kif = $sect_if ?
        ($key_if ? "(".$sect_if.") && (".$key_if.")" : $sect_if) : $key_if;
      my ($e_comm, $e_desc) = ("", "");
      my $type = "";
      if (/^Type: (.+)$/) {
        $type = $1;
        if ($type eq "enum") {
          my $enum = "static const char *e".$key."[] = { ";
          my $n_e_def = 0;
          while (getl(), /^ ([A-Za-z]+)(\/([A-Z_]+))?: (.+)$/) {
            my $e_nam = $1;
            $enum .= "\"".$e_nam."\", ";
            defined($3) &&
              ($ov_enum_defs .= "#define ".$3." ".($n_e_def++)."\n");
            my ($comm, $desc) = (dedb($4), $4);
            $comm =~ s/\"/\\\"/g;
            $e_comm .= " \\\n\"# \\\"".$e_nam."\\\" - ".$comm."\\n\"";
            $e_desc .=
              "<varlistentry>\n".
              "<term><parameter>".$e_nam."</parameter></term>\n".
              "<listitem><para>".$desc."</para></listitem>\n".
              "</varlistentry>\n";
          }
          $enum .= "0 };";
          my @oa = ( [ $enum, "ENUM" ] );
          add_cond($kif, $key, \@oa, \%ov_enum_conds);
          $ov_enums .= $oa[0][0]." \\\n";
          $n_e_def && ($ov_enum_defs .= "\n");
        } elsif ($type =~ /^(int|bool|group|string|path|list)$/) {
          getl();
        } else {
          pegout("unknown Type ".$type." for key ".$key." in section [".$sect."]");
        }
      } else {
        pegout("expecting Type for key ".$key." in section [".$sect."]");
      }
      my ($odflt, $dflt, $cdflt, $ddflt);
      my $quot = ($type =~ /^(int|bool|enum|group)$/);
      if (/^Default: (\*?)(.+)$/) {
        my $defd = $1;
        ($odflt, $dflt) = ($2, $2);
        $quot && ($dflt = "\"".$dflt."\"");
        $defd && ($ov_defaults .= "#define def_".$key." ".$dflt."\n");
        getl();
      } else {
        pegout("expecting Default for key ".$key." in section [".$sect."]");
      }
      if (/^CDefault: (.+)$/) {
        if ($1 eq "-") {
          $ddflt =
          $cdflt = "";
        } else {
          $ddflt =
          $cdflt = $1;
          $cdflt =~ s/"/\\"/g;
          $cdflt = " \\\n\"# Default is ".$cdflt."\\n\"";
        }
        getl();
      } else {
        $ddflt = $odflt;
        if ($quot) {
          $cdflt = " \\\n\"# Default is ".$odflt."\\n\"";
        } else {
          $cdflt = " \\\n\"# Default is \\\"\" ".$dflt." \"\\\"\\n\"";
        }
      }
      if (/^DDefault: -$/) {
        $ddflt = "";
        getl();
      }
      my $pproc;
      if (/^PostProc: (.+)$/) {
        $pproc = $1;
        getl();
      } else {
        $pproc = "";
      }
      my $nusers = 0;
      my ($vname, $kid, $xkid, $ctype, $cpptype, $cget, $cppget);
      while (/^User: (.+)$/) {
        my $user = $1;
        if ($user eq "dummy") {
          $vname = "dummy";
          $kid = "C_INTERNAL | C_TYPE_STR";
          $xkid = "";
        } elsif ($user =~ s/^(core|greeter|greeter-c|dep|config)(\((.+)\))?(:font)?$/$1/) {
          my ($hvn, $isfn) = (defined($3) ? $3 : mkvname($key), $4);
          ($kid, $xkid, $ctype, $cpptype, $cget, $cppget) = @{$th{$type}};
          $kid = sprintf "%#x | %s", $kid_seq, $kid;
          if ($user eq "dep") {
            $vname = $hvn;
            $xkid .= " | C_INTERNAL";
          } elsif ($user eq "config") {
            $vname = $hvn;
            $xkid .= " | C_INTERNAL | C_CONFIG";
          } else {
            $vname = "";
            if ($user eq "core") {
              if ($sect =~ /^-/) {
                my @oa = (
                  [ "{ ".$kid.", boffset(".$hvn.") },", "LOC" ],
                  [ $ctype.$hvn.";", "LDEF" ]
                );
                add_cond($kif, $hvn, \@oa, \%ov_loc_conds);
                $ov_locs .= " \\\n".$oa[0][0];
                $ov_loc_defs{$ctype} .= " \\\n\t".$oa[1][0];
              } else {
                my @oa = (
                  [ "{ ".$kid.", (char **) &".$hvn." },", "GLOB" ],
                  [ $ctype.$hvn.";", "GDEF" ],
                  [ "extern ".$ctype.$hvn.";", "GDECL" ]
                );
                add_cond($kif, $hvn, \@oa, \%ov_glob_conds);
                $ov_globs .= " \\\n".$oa[0][0];
                $ov_glob_defs{$ctype} .= " \\\n".$oa[1][0];
                $ov_glob_decls .= " \\\n".$oa[2][0];
              }
            } else { # greeter(-c)?
              my ($typ, $gtr, $isc);
              if ($isfn) {
                $typ = "QFont\t";
                $gtr = "Str2Font( GetCfgQStr( ".$kid." ) )";
                $isc = 0;
              } elsif ($user eq "greeter" && $cppget) {
                $typ = $cpptype;
                $gtr = $cppget."( ".$kid." )";
                $isc = 0;
              } else {
                $typ = $ctype;
                $gtr = $cget."( ".$kid.(($type eq "list") ? ", 0" : "")." )";
                $isc = 1;
              }
              my @oa = (
                [ "_".$hvn." = ".$gtr.";", "GRINIT" ],
                [ $typ."_".$hvn.";", "GRDEF" ],
                [ "extern ".$typ."_".$hvn.";", "GRDECL" ]
              );
              add_cond($kif, $hvn, \@oa, \%ov_greet_conds);
              $ov_greet_init .= " \\\n    ".$oa[0][0];
              $ov_greet_defs{$typ} .= " \\\n".$oa[1][0];
              $ov_greet_decls[$isc] .= " \\\n".$oa[2][0];
            }
          }
        } else {
          pegout("unrecognized User '".$user."' for key ".$key." in section [".$sect."]");
        }
        $nusers++;
        getl();
      }
      $nusers || pegout("expecting User for key ".$key." in section [".$sect."]");
      my $ninsts = 0;
      while (/^Instance: ?(.*)$/) {
        my $inst = $1;
        if ($inst ne "-") {
          my $on = 1 - ($inst =~ s/^#//);
          my $sec;
          if ($sect =~ /^-/) {
            ($inst =~ s/^([^\/]+)\///) || pegout("instance for key ".$key." in section [".$sect."] does not specify display");
            $sec = "X-".$1.$sect;
          } else {
            $sec = $sect;
          }
          if ($type eq "bool" && $inst eq "!") {
            $inst = ($dflt eq "\"true\"") ? "\"false\"" : "\"true\"";
          } elsif (!$inst) {
            $inst = $dflt;
          } else {
            $quot && ($inst = "\"".$inst."\"");
          }
          defined($ex_sects{$sec}) ||
            pegout("instantiating key ".$key." in section [".$sect."] in undeclared section");
          my @oa = ( [ "{ \"".$key."\",\t".$inst.", ".$on." },", "INST" ] );
          add_cond($key_if, $key, \@oa, \%ex_conds);
          $ex_config[$ex_sects{$sec}][2] .= $oa[0][0]." \\\n";
          $ex_config[$ex_sects{$sec}][3] = $sect_if;
        }
        $ninsts++;
        getl();
      }
      $ninsts ||
        print STDERR "Warning: key ".$key." in section [".$sect."] not instanciated\n";
      my ($update, $prio) = ("0", "");
      if (/^Update: ([^\/]+)(\/(\d+))?$/) {
        ($update, $prio) = ($1, $3);
        getl();
      }
      if ($prio) {
        ($max_prio < $prio) && ($max_prio = $prio);
      } else {
        $prio = 0;
      }
      my $mcnt = 0;
      while (/^Merge: (.+)$/) {
        my $merge = $1;
        if ($merge =~ /^xdm(:([^\(]+))?(\((.+)\))?$/) {
          my ($what, $proc) = ($2, $4);
          my @oa = (
            [ "{ \"".($what ? $what : lcfirst($key))."\", ".
              "\"".(($sect =~ /^-/) ? "X-%s" : "").$sect."\", ".
              ($what ? "\"".$key."\"" : "0").", ".
              ($proc ? $proc : "0")." },", "XM" ]
          );
          add_cond($kif, $key, \@oa, \%ov_xm_conds);
          $ov_xm[$sect =~ /^-/] .= $oa[0][0]." \\\n";
        } elsif ($merge =~ /^kdm:([^\(]+)(\((.+)\))?$/) {
          my ($where, $func) = ($1, $3);
          my $sec = "";
          ($where =~ s/^([^\/]+)\///) && ($sec = $1);
          my @oa = (
            [ "{ \"".($where ? $where : $key)."\", ".
              ($sec ? "\"".$sect."\"" : "0").", ".
              ($where ? "\"".$key."\"" : "0").", ".
              ($func ? $func : "0")." },", "KM".($mcnt++) ]
          );
          add_cond($kif, $key, \@oa, \%ov_km_conds);
          $ov_km{$sec ? $sec : $sect} .= $oa[0][0]." \\\n";
        } else {
          pegout("bogus Merge '".$merge."' for key ".$key." in section [".$sect."]");
        }
        getl();
      }
      # todo: handle $func here, too
      my @oa = ( [ "{ \"".$key."\", 0, 0, 0 },", "KM" ] );
      add_cond($kif, $key, \@oa, \%ov_km_conds);
      $ov_km{$sect} .= $oa[0][0]." \\\n";
      my $comm = "";
      if (/^Comment:(( [-&])?)$/) {
        if ($1 eq " &") {
          $comm = "&";
          getl();
        } elsif ($1 ne " -") {
          while (getl(), /^ (.*)$/) {
            $comm .= $1."\n";
          }
          $comm ||
            print STDERR "Warning: key ".$key." in section [".$sect."] has empty Comment\n";
        } else {
          getl();
        }
      } else {
        print STDERR "Warning: key ".$key." in section [".$sect."] has no Comment\n";
      }
      if (/^Description:(( [-!])?)$/) {
        if ($1 ne " -") {
          $doc_ref .=
            "<varlistentry>\n".
            "<term id=\"option-".lc($key)."\"><option>".$key."</option></term>\n".
            "<listitem>\n";
          ($1 eq " !") &&
            ($e_desc = "");
          my $desc = "";
          while (getl(), /^ (_|(.*))$/) {
            $desc .= $2."\n";
          }
          $desc ||
            print STDERR "Warning: key ".$key." in section [".$sect."] has empty Description\n";
          ($comm eq "&") &&
            ($comm = $desc);
          $desc = "<para>\n".$desc."</para>\n";
          if ($e_desc) {
            $e_desc = "<variablelist>\n".$e_desc."</variablelist>\n";
            ($desc =~ s/%ENUM%/$e_desc/) ||
              ($desc .= $e_desc);
          }
          $doc_ref .= $desc;
          if ($ddflt) {
            if ($ddflt eq '""') {
              $doc_ref .= "<para>Empty by default.</para>\n";
            } else {
              $ddflt =~ s/\"//g;
              $ddflt =~ s,KDMCONF ,\${<envar>kde_confdir</envar>}/kdm,;
              $ddflt =~ s,KDMDATA ,\${<envar>kde_datadir</envar>}/kdm,;
              $ddflt =~ s,XBINDIR ,\${<envar>x_bindir</envar>},;
              $doc_ref .= "<para>The default is <quote>".$ddflt."</quote>.</para>\n";
            }
          }
          $doc_ref .= "</listitem>\n</varlistentry>\n\n";
        } else {
          getl();
        }
      } else {
        print STDERR "Warning: key ".$key." in section [".$sect."] has no Description\n";
      }
      pegout("unidentified section body '".$_."' in section [".$sect."]") if ($_);
      if ($vname) {
        ($vname ne "dummy") &&
          ($arr_ov_vars{$vname} = $kif);
        $vname = "&V".$vname;
      } elsif ($pproc) {
        $vname = "(void *)".$pproc;
      } elsif ($type eq "enum") {
        $vname = "e".$key;
      } else {
        $vname = "0";
      }
	  $comm = dedb($comm);
      $comm =~ s/"/\\"/g;
      $comm =~ s/([^\n]*)\n/ \\\n\"# $1\\n\"/g;
      @oa = (
        [ "{ \"".$key."\", ".$kid.$xkid.", ".$vname.", ".$dflt." },", "RENT" ],
        [ "{ \"".$key."\", ".$prio.", ".$update.",".$comm.$e_comm.$cdflt." },", "GENT" ],
      );
      add_cond($key_if, $key, \@oa, \%ov_ent_conds);
      $ov_rd_ents .= $oa[0][0]." \\\n";
      $ov_gen_ents .= $oa[1][0]." \\\n";
      $kid_seq++;
    } elsif (/^Section: (.+)$/) {
      emit_section() if ($sect);
      $sect = $1;
      defined($sect_names{$sect}) && pegout("redefinition of section [".$sect."]");
      $sect_names{$sect} = "";
      %key_names = ();
      getl();
      if (/^If: (.+)$/) {
        $sect_if = $1;
        getl();
      } else {
        $sect_if = "";
      }
      my ($sref, $stit, $sna);
      if ($sect =~ /^-(.*)$/) {
        $sref = lc($1);
        $stit = "X-*-".$1;
        $sna = "section class";
      } else {
        $sref = lc($sect);
        $stit = $sect;
        $sna = "section";
      }
      $doc_ref .=
        "\n<sect2 id=\"kdmrc-".$sref."\">\n".
        "<title>The [".$stit."] ".$sna." of &kdmrc;</title>\n\n";
      if (/^Description:(( -)?)$/) {
        if ($1 ne " -") {
          my $desc = 0;
          $doc_ref .= "<para>\n";
          while (getl(), /^ (_|(.*))$/) {
            $doc_ref .= $2."\n";
            $desc = 1;
          }
          $doc_ref .= "</para>\n";
          $desc ||
            print STDERR "Warning: section [".$sect."] has empty Description\n";
        } else {
          getl();
        }
      } else {
        print STDERR "Warning: section [".$sect."] has no Description\n";
      }
      $doc_ref .= "\n<variablelist>\n\n";
      pegout("unidentified section body '".$_."' in section [".$sect."]") if ($_);
    } else {
      pegout("invalid section leadin: '".$_."'");
    }
  }
}
emit_section();
close INFILE;

my $srcf = relpath($ARGV[0], $ARGV[1]);

open (OUTFILE, ">".$ARGV[1]) || pegout("$0: cannot create output file ".$ARGV[1]);

if (!$do_doc) {

print OUTFILE
  "/* generated from $srcf by $0 - DO NOT EDIT! */\n\n".
  "#ifndef CONFIG_DEFS\n".
  "#define CONFIG_DEFS\n\n".
  $raw_out."\n\n".
  $ov_enum_defs."\n".
  $ov_defaults."\n\n";

my $ov_vars = "";
my %ov_var_conds = ();
for my $v (keys %arr_ov_vars) {
  my @oa = ( ["V".$v.",", "VAR"] );
  add_cond($arr_ov_vars{$v}, $v, \@oa, \%ov_var_conds);
  $ov_vars .= "    ".$oa[0][0]." \\\n";
}
print OUTFILE
  emit_conds(\%ov_var_conds).
  "#define CONF_READ_VARS \\\n \\\n".
  "static Value \\\n".
    $ov_vars.
    "    Vdummy;\n\n\n";

print OUTFILE
  emit_conds(\%ov_ent_conds).
  emit_conds(\%ov_sec_conds).
  "#define CONF_SECTS \\\n \\\n".
  "static Sect \\\n".
    $ov_sect_defs.
  "    *allSects[]\t= { \\\n".
    $ov_sect_refs.
  "    };\n\n\n";

print OUTFILE
  emit_conds(\%ov_enum_conds).
  "#define CONF_READ_ENTRIES \\\n \\\n".
  $ov_enums.
  $ov_rd_sects." \\\n".
  "CONF_SECTS\n\n\n";

print OUTFILE
  "#define CONF_MAX_PRIO ".$max_prio."\n\n".
  "#define CONF_GEN_ENTRIES \\\n".
  $ov_gen_sects." \\\n".
  "CONF_SECTS\n\n\n";

print OUTFILE
  emit_conds(\%ov_glob_conds).
  "#define CONF_CORE_GLOBALS \\\n".
  $ov_globs."\n\n\n".
  "#define CONF_CORE_GLOBAL_DECLS \\\n".
  $ov_glob_decls."\n\n\n".
  "#define CONF_CORE_GLOBAL_DEFS \\\n".
  emit_defs(\%ov_glob_defs)."\n\n\n";

print OUTFILE
  emit_conds(\%ov_loc_conds).
  "#define CONF_CORE_LOCALS \\\n".
  $ov_locs."\n\n\n".
  "#define CONF_CORE_LOCAL_DEFS \\\n".
  emit_defs(\%ov_loc_defs)."\n\n\n\n";

print OUTFILE
  emit_conds(\%ov_greet_conds).
  "#define CONF_GREET_INIT \\\n".
  $ov_greet_init."\n\n\n".
  "#define CONF_GREET_C_DECLS \\\n".
  $ov_greet_decls[1]."\n\n\n".
  "#define CONF_GREET_CPP_DECLS \\\n".
  $ov_greet_decls[0]."\n\n\n".
  "#define CONF_GREET_DEFS \\\n".
  emit_defs(\%ov_greet_defs)."\n\n\n";

my ($ov1, $ov2) = ("", "");
my %ex_sec_conds = ();
for my $i (@ex_config) {
  my $vn;
  if ($i->[0] =~ /^X-(.+)-(.+)$/) {
    if ($1 eq "*") {
      $vn = "dEntsAny".$2;
    } elsif ($1 eq ":*") {
      $vn = "dEntsLocal".$2;
    } else {
      my ($t1, $t2) = ($1, $2);
      $t1 =~ s/[-:]/_/g;
      $vn = "dEnts".$t1.$t2;
    }
  } else {
    $vn = "dEnts".$i->[0];
  }
  my @oa = (
    [ "static DEnt ".$vn."[] = { \\\n".$i->[2]."};", "DSEC" ],
    [ "{ \"".$i->[0]."\",\t".$vn.",\tas(".$vn."),".$i->[1]." },", "DSECS" ]
  );
  add_cond($i->[3], $vn, \@oa, \%ex_sec_conds);
  $ov1 .= $oa[0][0]." \\\n \\\n";
  $ov2 .= $oa[1][0]." \\\n";
}
print OUTFILE
  emit_conds(\%ex_conds).
  emit_conds(\%ex_sec_conds).
  "#define CONF_GEN_EXAMPLE \\\n \\\n".
  $ov1.
  "static DSect dAllSects[] = { \\\n".
    $ov2.
  "};\n\n\n";

print OUTFILE
  emit_conds(\%ov_xm_conds).
  "#define CONF_GEN_XMERGE \\\n \\\n".
  "XResEnt globents[] = { \\\n".
  $ov_xm[0].
  "}, dpyents[] = { \\\n".
  $ov_xm[1].
  "};\n\n\n";

my $ov_km_sects = "";
my $ov_km_sect_refs = "";
for my $s (keys %ov_km) {
  my $ts = $s;
  $ts =~ s/-/_/;
  $ov_km_sects .=
    "KUpdEnt upd".$ts."[] = { \\\n".
    $ov_km{$s}.
    "}; \\\n \\\n";
  $ov_km_sect_refs .= "{ \"".$s."\", upd".$ts.", as(upd".$ts.") }, \\\n";
}
print OUTFILE
  emit_conds(\%ov_km_conds).
  "#define CONF_GEN_KMERGE \\\n \\\n".
  $ov_km_sects.
  "KUpdSec kupsects[] = { \\\n".
  $ov_km_sect_refs.
  "};\n";
print OUTFILE
  "\n#endif /* CONFIG_DEFS */\n";

} else {

$doc =~ s/%REF%/$doc_ref/;
print OUTFILE
  "<!-- generated from $srcf - DO NOT EDIT! -->\n\n".
  $doc;

}

close OUTFILE;
