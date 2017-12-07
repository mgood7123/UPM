#i need to convert this https://paste.pound-python.org/raw/RqUji4bl0SK64XWS4Gkf/ into C wich produces relatively the same output
# ./perl/bin/cpan HTTP::Date inc::latest Module::Build::Tiny Perl::Lint strictures IO::Async::Loop IO::Async::OS IO::Async::Stream IO::Async::Handle IO::Async::Protocol::Stream IO::Async::Loop Net::Async::HTTP IO::Async::SSL HTML::LinkExtractor Future  Data::Dumper DBI Perl::Critic Test::Perl::Critic B::Lint ControlStructures::ProhibitUnreachableCode  Test::DependentModules Test::Output Test::Memory::Cycle Devel::PartialDump Algorithm::C3 DBM::Deep DateTime DateTime::Calendar::Mayan DateTime::Format::MySQL  Declare::Constraints::Simple Local::US Module::Refresh MooseX::NonMoose Params::Coerce Regexp::Common Moo SUPER Data::Visitor Taint::Runtime Test::Taint Readonly Test::Regexp B::Lint PAR::Dist Foo::Bar::Baz Test::LeakTrace Test::MockRandom Unicode::UTF8
use strictures;
use IO::Async::Loop;
use Net::Async::HTTP;
use IO::Async::SSL;
use HTML::LinkExtractor;
use Future;
use Data::Dumper;
print "starting...\n";
my $repos =
[
    {
        name => 'blackarch',
        url  => 'https://www.mirrorservice.org/sites/blackarch.org/blackarch/blackarch/os/x86_64/',
    },
    {
        name => 'archlinux',
        url  => 'http://mirrors.kernel.org/archlinux/pool/packages/',
    },
];

my $lx = HTML::LinkExtractor->new( );

my $input  = 'linux';

my $filter = sub
{
    $_[0] =~ m/\Q$input\E/
    and
    $_[0] =~ m/\.xz$/;
    #and
    #$_[0] !~ m/\.sig$/;
};

my $loop = IO::Async::Loop->new;
$loop->add(my $ua = Net::Async::HTTP->new(max_connections_per_host => 0+@{$repos}));

my @info = Future->needs_all(map {
    my ($repo) = $_;
    $ua->GET($_->{url})
        ->then(sub {
            my ($res) = @_;
            $lx->parse( \$res->content );
            my $links = $lx->links;
            return Future->done({
                %$repo,
                links => [ map { $_->{href} } grep { $_->{tag} eq 'a' and $filter->( $_->{href} ) } @$links ]
            })
        })
    } @$repos)->get;

for my $repo ( @info ) { print "\n$repo->{name} $_" for ( @{$repo->{links}} ); }
