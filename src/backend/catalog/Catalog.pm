#----------------------------------------------------------------------
#
# Catalog.pm
#    Perl module that extracts info from catalog headers into Perl
#    data structures
#
# Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# $PostgreSQL$
#
#----------------------------------------------------------------------

package Catalog;

use strict;
use warnings;

require Exporter;
our @ISA       = qw(Exporter);
our @EXPORT    = ();
our @EXPORT_OK = qw(Catalogs RenameTempFile);

# Call this function with an array of names of header files to parse.
# Returns a nested data structure describing the data in the headers.
sub Catalogs
{
    my (%catalogs, $catname, $declaring_attributes, $most_recent);
    $catalogs{names} = [];

    # There are a few types which are given one name in the C source, but a
    # different name at the SQL level.  These are enumerated here.
    my %RENAME_ATTTYPE = (
        'Oid'           => 'oid',
        'NameData'      => 'name',
        'TransactionId' => 'xid'
    );

    foreach my $input_file (@_)
    {
        my %catalog;
        $catalog{columns} = [];
        $catalog{data} = [];

        open(INPUT_FILE, '<', $input_file) || die "$input_file: $!";

        # Scan the input file.
        while (<INPUT_FILE>)
        {
            # Strip C-style comments.
            s;/\*(.|\n)*\*/;;g;
            if (m;/\*;)
            {
                # handle multi-line comments properly.
                my $next_line = <INPUT_FILE>;
                die "$input_file: ends within C-style comment\n"
                  if !defined $next_line;
                $_ .= $next_line;
                redo;
            }

            # Strip useless whitespace and trailing semicolons.
            chomp;
            s/^\s+//;
            s/;\s*$//;
            s/\s+/ /g;

            # Push the data into the appropriate data structure.
            if (/^DATA\(insert(\s+OID\s+=\s+(\d+))?\s+\(\s*(.*)\s*\)\s*\)$/)
            {
                push @{ $catalog{data} }, {oid => $2, bki_values => $3};
            }
            elsif (/^DESCR\(\"(.*)\"\)$/)
            {
                $most_recent = $catalog{data}->[-1];
                # this tests if most recent line is not a DATA() statement
                if (ref $most_recent ne 'HASH')
                {
                    die "DESCR() does not apply to any catalog ($input_file)";
                }
                if (!defined $most_recent->{oid})
                {
                    die "DESCR() does not apply to any oid ($input_file)";
                }
                elsif ($1 ne '')
                {
                    $most_recent->{descr} = $1;
                }
            }
            elsif (/^SHDESCR\(\"(.*)\"\)$/)
            {
                $most_recent = $catalog{data}->[-1];
                # this tests if most recent line is not a DATA() statement
                if (ref $most_recent ne 'HASH')
                {
                    die "SHDESCR() does not apply to any catalog ($input_file)";
                }
                if (!defined $most_recent->{oid})
                {
                    die "SHDESCR() does not apply to any oid ($input_file)";
                }
                elsif ($1 ne '')
                {
                    $most_recent->{shdescr} = $1;
                }
            }
            elsif (/^DECLARE_TOAST\(\s*(\w+),\s*(\d+),\s*(\d+)\)/)
            {
                $catname = 'toasting';
                my ($toast_name, $toast_oid, $index_oid) = ($1, $2, $3);
                push @{ $catalog{data} }, "declare toast $toast_oid $index_oid on $toast_name\n";
            }
            elsif (/^DECLARE_(UNIQUE_)?INDEX\(\s*(\w+),\s*(\d+),\s*(.+)\)/)
            {
                $catname = 'indexing';
                my ($is_unique, $index_name, $index_oid, $using) = ($1, $2, $3, $4);
                push @{ $catalog{data} },
                  sprintf(
                    "declare %sindex %s %s %s\n",
                    $is_unique ? 'unique ' : '',
                    $index_name, $index_oid, $using
                  );
            }
            elsif (/^BUILD_INDICES/)
            {
                push @{ $catalog{data} }, "build indices\n";
            }
            elsif (/^CATALOG\(([^,]*),(\d+)\)/)
            {
                $catname = $1;
                $catalog{relation_oid} = $2;

                # Store pg_* catalog names in the same order we receive them
                push @{ $catalogs{names} }, $catname;

                $catalog{bootstrap}       = /BKI_BOOTSTRAP/            ? ' bootstrap'       : '';
                $catalog{shared_relation} = /BKI_SHARED_RELATION/      ? ' shared_relation' : '';
                $catalog{without_oids}    = /BKI_WITHOUT_OIDS/         ? ' without_oids'    : '';
                $catalog{rowtype_oid}     = /BKI_ROWTYPE_OID\((\d+)\)/ ? " rowtype_oid $1"  : '';
                $catalog{schema_macro}    = /BKI_SCHEMA_MACRO/         ? 'True'             : '';
                $declaring_attributes = 1;
            }
            elsif ($declaring_attributes)
            {
                next if (/^{|^$/);
                if (/^}/)
                {
                    undef $declaring_attributes;
                }
                else
                {
                    my ($atttype, $attname) = split /\s+/, $_;
                    if (exists $RENAME_ATTTYPE{$atttype})
                    {
                        $atttype = $RENAME_ATTTYPE{$atttype};
                    }
                    if ($attname =~ /(.*)\[.*\]/)        # array attribute
                    {
                        $attname = $1;
                        $atttype .= '[]';                # variable-length only
                    }
                    push @{ $catalog{columns} }, {$attname => $atttype};
                }
            }
        }
        $catalogs{$catname} = \%catalog;
        close INPUT_FILE;
    }
    return \%catalogs;
}

# Rename temporary files to final names, if anything has changed.
# Call this function with the final file name --- we append .tmp automatically
sub RenameTempFile
{
    my $final_name = shift;
    my $temp_name = $final_name . '.tmp';
    if (-e $final_name && -s $temp_name == -s $final_name)
    {
        open TN, '<', "$temp_name" || die "$temp_name: $!";
        if (open FN, '<', $final_name)
        {
            local $/ = undef;
            my $tn = <TN>;
            my $fn = <FN>;
            close FN;
            if ($tn eq $fn)
            {
                print "$final_name unchanged, not replacing\n";
                close TN;
                unlink($temp_name) || die "unlink: $temp_name: $!";
                return;
            }
        }
        close TN;
    }
    print "Writing $final_name\n";
    rename($temp_name, $final_name) || die "rename: $temp_name: $!";
}

1;
