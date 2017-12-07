print "starting...\n";
use strict;
use warnings FATAL => 'all';
use Thread::Pool;
use HTML::LinkExtractor;
use Data::Dumper;

my $repos =
[
    {
        name => 'blackarch',
        url  => 'https://www.mirrorservice.org/sites/blackarch.org/blackarch/blackarch/os/x86_64/',
    },
    {
        name => 'blackarch',
        url  => 'https://www.mirrorservice.org/sites/blackarch.org/blackarch/blackarch/os/x86_64/',
    },
    {
        name => 'blackarch',
        url  => 'https://www.mirrorservice.org/sites/blackarch.org/blackarch/blackarch/os/x86_64/',
    },
    {
        name => 'blackarch',
        url  => 'https://www.mirrorservice.org/sites/blackarch.org/blackarch/blackarch/os/x86_64/',
    },
    {
        name => 'blackarch',
        url  => 'https://www.mirrorservice.org/sites/blackarch.org/blackarch/blackarch/os/x86_64/',
    },
    {
        name => 'blackarch',
        url  => 'https://www.mirrorservice.org/sites/blackarch.org/blackarch/blackarch/os/x86_64/',
    },
    {
        name => 'blackarch',
        url  => 'https://www.mirrorservice.org/sites/blackarch.org/blackarch/blackarch/os/x86_64/',
    },
    {
        name => 'archlinux',
        url  => 'http://mirrors.kernel.org/archlinux/pool/packages/',
    },
    {
        name => 'archlinux',
        url  => 'http://mirrors.kernel.org/archlinux/pool/packages/',
    },
    {
        name => 'archlinux',
        url  => 'http://mirrors.kernel.org/archlinux/pool/packages/',
    },
    {
        name => 'archlinux',
        url  => 'http://mirrors.kernel.org/archlinux/pool/packages/',
    },
    {
        name => 'archlinux',
        url  => 'http://mirrors.kernel.org/archlinux/pool/packages/',
    },
];

my $lx = HTML::LinkExtractor->new( );

my $input  = 'bash';

my $filter = sub
{
    $_[0] =~ m/\Q$input\E/
    and
    $_[0] =~ m/\.xz$/;
    #and
    #$_[0] !~ m/\.sig$/;
};

my $ref = pool_run( $repos );

for my $id ( sort keys %$ref )
{
    my $repo = $ref->{$id};
    
    print "$repo->{name}\n";
    print "$_\n" for ( @{$repo->{links}} );
    print "\n";
}

sub pool_run
{
    my ( $jobs ) = @_;
    
    my ( %ids, @result, %results );
    
    my $pool = Thread::Pool->new
    (
        {
            workers => 200,
            do      => \&pool_do,
        }
    );

    for my $job ( @$jobs )
    {
        my $id = $pool->job( $job );
    
        $ids{$id} = 1;
    }

    while ( keys %ids )
    {
        for my $id ( keys %ids )
        {
            if ( @result = $pool->result_dontwait( $id ) )
            {
                $results{$id} =$result[0];
                
                delete $ids{$id};
            }
        }
    }
    
    return \%results;
}

sub pool_do
{
    my ( $repo ) = @_;
    
    my $cont = qx(aria2c $repo->{url});
    
    $lx->parse( \$cont );
    
    my $links = $lx->links;
    
    @$links = map { $_->{href} } grep { $_->{tag} eq 'a' and $filter->( $_->{href} ) } @$links;
    
    return { %$repo, links => $links };
}


