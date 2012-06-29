
# Before 'make install' is performed this script should be runnable with
# 'make test'. After 'make install' it should work as 'perl test.pl'

#########################

# Make warnings fatal
use warnings;
BEGIN {$SIG{__WARN__} = sub { die "Terminating test due to warning: $_[0]" } };

use Test::More;
BEGIN { plan tests => 10 };
use Search::Xapian qw(:standard);
use Config;

ok(1); # If we made it this far, we're ok.

#########################

my $query = Search::Xapian::Query->new(
	OP_AND,
	Search::Xapian::Query->new( "foo" ),
	"bar"
);
ok(defined $query, "AND query created");
is($query->get_description, 'Query((foo AND bar))', "AND query contains foo part");

$query = Search::Xapian::Query::MatchAll;
is($query->get_description, 'Query(<alldocuments>)');
$query = Search::Xapian::Query->new("");
is($query->get_description, 'Query(<alldocuments>)');
$query = Search::Xapian::Query::MatchNothing;
is($query->get_description, 'Query()');
$query = Search::Xapian::Query->new();
is($query->get_description, 'Query()');

eval {
    Search::Xapian::Query->new("hello", 1, 2, 3, 4);
};
ok(defined $@, "Bad query ctor threw exception");
like($@, qr!^USAGE: Search::Xapian::Query->new\('term'\) or Search::Xapian::Query->new\(OP, <args>\) at \S+/10query\.t line \d+$!);

ok(1);
