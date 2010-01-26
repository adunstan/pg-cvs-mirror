
#  $PostgreSQL: pgsql/src/pl/plperl/plc_safe_bad.pl,v 1.2 2010/01/20 01:08:21 adunstan Exp $

# Minimal version of plc_safe_ok.pl
# that's used if Safe is too old or doesn't load for any reason

my $msg = 'trusted Perl functions disabled - please upgrade Perl Safe module';

sub mksafefunc {
	my ($name, $pragma, $prolog, $src) = @_;
	# replace $src with code to generate an error
	$src = qq{ ::elog(::ERROR,"$msg\n") };
	my $ret = eval(::mkfuncsrc($name, $pragma, '', $src));
	$@ =~ s/\(eval \d+\) //g if $@;
	return $ret;
}
