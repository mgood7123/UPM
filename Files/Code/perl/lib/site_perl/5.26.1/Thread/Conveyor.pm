package Thread::Conveyor;

# Start the Thread::Tie thread now if not already started (as clean as possible)

use Thread::Tie ();

# Make sure we have version info for this module
# Make sure we do everything by the book from now on

$VERSION = '0.19';
use strict;

# Make sure we only load stuff when we actually need it

use load;

# Set default optimization

our $OPTIMIZE = $] > 5.008 ? 'cpu' : 'memory';

# Satisfy -require-

1;

#---------------------------------------------------------------------------

# The following subroutines are loaded only on demand

__END__

#---------------------------------------------------------------------------

# Class methods

#---------------------------------------------------------------------------
#  IN: 1 class with which to bless the object
#      2 parameter hash reference
# OUT: 1 instantiated object

sub new {

# Obtain the class
# Obtain the parameter hash, or create an empty one

    my $class = shift;
    my $self = shift || {};

# Obtain the optimization to be used
# Die now if unknown optimization

    my $optimize = $self->{'optimize'} || $OPTIMIZE;
    die "Don't know how to handle '$optimize' optimization"
     unless $optimize =~ m#^(?:cpu|memory)$#;

# Set maximum number of boxes if applicable
# Return now with an unthrottled array implementation if so required

    $self->{'maxboxes'} = 50 unless exists( $self->{'maxboxes'} );
    return $class->_new( $class.'::'.(qw(Tied Array)[$optimize eq 'cpu']) )
      if !$self->{'maxboxes'};

# Set minimum number of boxes if applicable
# Initialize a shared halted flag
# Safe a reference to it in the object
# Use the ::Throttled implementation (which will figure which optimization)

    $self->{'minboxes'} ||= $self->{'maxboxes'} >> 1;
    my $halted : shared = 0;
    $self->{'halted'} = \$halted;
    $class->_new( $class.'::Throttled',$self );
} #new

#---------------------------------------------------------------------------
#  IN: 1 class (ignored)
#      2 new default optimization type
# OUT: 1 current default optimization type

sub optimize {

# Set new optimized value if specified
# Return current optimized value

    $OPTIMIZE = $_[1] if @_ > 1;
    $OPTIMIZE;
} #optimize

#---------------------------------------------------------------------------

# Internal subroutines

#---------------------------------------------------------------------------
#  IN: 1 this class (ignored)
#      2 class for which to create object
#      3..N parameters to be passed to it
# OUT: 1 blessed object

sub _new {

# Ignore our own class
# Obtain the class
# Create module name
# Allow non-strict references
# Make sure the sub-module is available
# Return object created with give parameter

    shift;
    my $class = shift;
    (my $module = $class) =~ s#::#/#g;
    no strict 'refs';
    require $module.'.pm' unless defined( ${$class.'::VERSION'} );
    $class->new( @_ );
} #_new

#---------------------------------------------------------------------------

__END__

=head1 NAME

Thread::Conveyor - transport of any data-structure between threads

=head1 VERSION

This documentation describes version 0.19.

=head1 SYNOPSIS

    use Thread::Conveyor;
    my $belt = Thread::Conveyor->new(
     {
      maxboxes => 50,
      minboxes => 25,
      optimize => 'memory', # or 'cpu'
     }
    );

    $belt->put( "foo", ["bar"], {"zoo"} );
    my ($foo,$bar,$zoo) = $belt->take;
    my ($foo,$bar,$zoo) = $belt->take_dontwait;
    my ($foo,$bar,$zoo) = $belt->peek;
    my ($foo,$bar,$zoo) = $belt->peek_dontwait;
    my $onbelt = $belt->onbelt;

    my @box = $belt->clean;
    my @box = $belt->clean_dontwait;
    my ($foo,$bar,$zoo) = @{$box[0]};

    $belt->maxboxes( 100 );
    $belt->minboxes( 50 );

    $belt->shutdown;
    $belt->thread;
    $belt->tid;

=head1 DESCRIPTION

                  *** A note of CAUTION ***

 This module only functions on Perl versions 5.8.0 and later.
 And then only when threads are enabled with -Dusethreads.  It
 is of no use with any version of Perl before 5.8.0 or without
 threads enabled.

                  *************************

The Thread::Conveyor object is a thread-safe data structure that mimics the
behaviour of a conveyor belt.  One or more worker threads can put boxes with
frozen values and references on one end of the belt to be taken off by one
or more worker threads on the other end of the belt to be thawed and returned.

A box may consist of any combination of scalars and references to scalars,
arrays (lists) and hashes.  Freezing and thawing is currently done with the
L<Thread::Serialize> module, but that may change in the future.  Objects and
code references are currently B<not> allowed.

By default, the maximum number of boxes on the belt is limited to B<50>.
Putting of boxes on the belt is halted if the maximum number of boxes is
exceeded.  This throttling feature was added because it was found that
excessive memory usage could be caused by having the belt growing too large.
Throttling can be disabled if so desired.

=head1 CLASS METHODS

=head2 new

 $belt = Thread::Conveyor->new(
  {
   maxboxes => 50,
   minboxes => 25,
   optimize => 'memory', # or 'cpu'
  }
 );

The "new" function creates a new empty belt.  It returns the instantiated
Thread::Conveyor object.

The input parameter is a reference to a hash.  The following fields are
B<optional> in the hash reference:

=over 2

=item maxboxes

 maxboxes => 50,

 maxboxes => undef,  # disable throttling

The "maxboxes" field specifies the B<maximum> number of boxes that can be
sitting on the belt to be handled (throttling).  If a new L<put> would
exceed this amount, putting of boxes will be halted until the number of
boxes waiting to be handled has become at least as low as the amount
specified with the "minboxes" field.

Fifty boxes will be assumed for the "maxboxes" field if it is not specified.
If you do not want to have any throttling, you can specify the value "undef"
for the field.  But beware!  If you do not have throttling active, you may
wind up using excessive amounts of memory used for storing all of the boxes
that have not been handled yet.

The L<maxboxes> method can be called to change the throttling settings
during the lifetime of the object.

=item minboxes

 minboxes => 25, # default: maxboxes / 2

The "minboxes" field specifies the B<minimum> number of boxes that can be
waiting on the belt to be handled before the L<put>ting of boxes is allowed
again (throttling).

If throttling is active and the "minboxes" field is not specified, then
half of the "maxboxes" value will be assumed.

The L<minboxes> method can be called to change the throttling settings
during the lifetime of the object.

=item optimize

 optimize => 'cpu', # default: depends on Perl version

The "optimize" field specifies which implementation of the belt will be
selected.  Currently there are two choices: 'cpu' and 'memory'.  For Perl
5.8.0 the default is "memory".  For higher versions of perl, the default
optimization is "cpu".  The reason for this was that Perl 5.8.0 has a severe
memory leak with shared arrays, which is what is being used with the "cpu"
optimization.

You can call the class method L<optimize> to change the default optimization.

=back

=head2 optimize

 Thread::Conveyor->optimize( 'cpu' );

 $optimize = Thread::Conveyor->optimize;

The "optimize" class method allows you to specify the default optimization
type that will be used if no "optimize" field has been explicitely specified
with a call to L<new>.  It returns the current default type of optimization.

Currently two types of optimization can be selected:

=over 2

=item memory

Attempt to use as little memory as possible.  Currently, this is achieved by
starting a seperate thread which hosts an unshared array.  This uses the
"Thread::Conveyor::Thread" sub-class.

=item cpu

Attempt to use as little CPU as possible.  Currently, this is achieved by
using a shared array (using the "Thread::Conveyor::Array" sub-class),
encapsulated in a hash reference if throttling is activated (then also using
the "Thread::Conveyor::Throttled" sub-class).

=back

=head1 OBJECT METHODS

The following methods operate on the instantiated Thread::Conveyor object.

=head2 put

 $belt->put( 'string',$scalar,[],{} );

The "put" method freezes all the specified parameters together in a box and
puts the box on the beginning of the belt.

=head2 take

 ($string,$scalar,$listref,$hashref) = $belt->take;

The "take" method waits for a box to become available at the end of the
belt, removes that box from the belt, thaws the contents of the box and
returns the resulting values and references.

=head2 take_dontwait

 ($string,$scalar,$listref,$hashref) = $belt->take_dontwait;

The "take_dontwait" method, like the L<take> method, removes a box from the
end of the belt if there is a box waiting at the end of the belt.  If there
is B<no> box available, then the "take_dontwait" method will return
immediately with an empty list.  Otherwise the contents of the box will be
thawed and the resulting values and references will be returned.

=head2 clean

 @box = $belt->clean;
 ($string,$scalar,$listref,$hashref) = @{$box[0]};

The "clean" method waits for one or more boxes to become available at the
end of the belt, removes B<all> boxes from the belt, thaws the contents of
the boxes and returns the resulting values and references as an array
where each element is a reference to the original contents of each box.

=head2 clean_dontwait

 @box = $belt->clean_dontwait;
 ($string,$scalar,$listref,$hashref) = @{$box[0]};

The "clean_dontwait" method, like the L<clean> method, removes all boxes
from the end of the belt if there are any boxes waiting at the end of the
belt.  If there are B<no> boxes available, then the "clean_dontwait" method
will return immediately with an empty list.  Otherwise the contents of the
boxes will be thawed and the resulting values and references will be
returned an an array where each element is a reference to the original
contents of each box.

=head2 peek

 ($string,$scalar,$listref,$hashref) = $belt->peek;

 @lookahead = $belt->peek( $index );

The "peek" method waits for a box to become availabe at the end of the
belt, but does B<not> remove it from the belt like the L<take> method does.
It does however thaw the contents and returns the resulting values and
references.

For advanced, and mostly internal, usages, it is possible to specify the
ordinal number of the box in which to peek.

Please note that there is B<no> guarantee that "take" will give you the
same data as which is returned with this method, as any other thread can
have taken the boxes off of the belt in the meantime.

=head2 peek_dontwait

 ($string,$scalar,$listref,$hashref) = $belt->peek_dontwait;

 @lookahead = $belt->peek_dontwait( $index );

The "peek_dontwait" method is like the L<take_dontwait> method, but does
B<not> remove the box from the belt if there is one available.  If there
is a box available, then the contents of the box will be thawed and the
resulting values and references are returned.  An empty list will be
returned if there was no box available at the end of the belt.

For advanced, and mostly internal, usages, it is possible to specify the
ordinal number of the box in which to peek.

Please note that there is B<no> guarantee that "take" will give you the
same data as which is returned with this method, as any other thread can
have taken the boxes off of the belt in the meantime.

=head2 onbelt

 $onbelt = $belt->onbelt;

The "onbelt" method returns the number of boxes that are still in the belt.

=head2 maxboxes

 $belt->maxboxes( 100 );
 $maxboxes = $belt->maxboxes;

The "maxboxes" method returns the maximum number of boxes that can be on the
belt before throttling sets in.  The input value, if specified, specifies the
new maximum number of boxes that may be on the belt.  Throttling will be
switched off if the value B<undef> is specified.

Specifying the "maxboxes" field when creating the object with L<new> is
equivalent to calling this method.

The L<minboxes> method can be called to specify the minimum number of boxes
that must be on the belt before the putting of boxes is allowed again after
reaching the maximum number of boxes.  By default, half of the "maxboxes"
value is assumed.

=head2 minboxes

 $belt->minboxes( 50 );
 $minboxes = $belt->minboxes;

The "minboxes" method returns the minimum number of boxes that must be on the
belt before the putting of boxes is allowed again after reaching the maximum
number of boxes.  The input value, if specified, specifies the new minimum
number of boxes that must be on the belt.

Specifying the "minboxes" field when creating the object with L<new> is
equivalent to calling this method.

The L<maxboxes> method can be called to set the maximum number of boxes that
may be on the belt before the putting of boxes will be halted.

=head2 shutdown

 $belt->shutdown;

The "shutdown" method performs an orderly shutdown of the belt.  It waits
until all of the boxes on the belt have been removed before it returns.

=head2 thread

 $thread = $belt->thread;

The "thread" method returns the thread object that is being used for the belt.
It returns undef if no seperate thread is being used.

=head2 tid

 $tid = $belt->tid;

The "tid" method returns the thread id of the thread object that is being
used for the belt.  It returns undef if no seperate thread is being used.

=head1 REQUIRED MODULES

 load (any)
 Thread::Serialize (any)
 Thread::Tie (0.09)

=head1 OPTIMIZATIONS

This module uses L<load> to reduce memory and CPU usage. This causes
subroutines only to be compiled in a thread when they are actually needed at
the expense of more CPU when they need to be compiled.  Simple benchmarks
however revealed that the overhead of the compiling single routines is not
much more (and sometimes a lot less) than the overhead of cloning a Perl
interpreter with a lot of subroutines pre-loaded.

=head1 CAVEATS

Passing unshared values between threads is accomplished by serializing the
specified values using L<Thread::Serialize>.  Please see the CAVEATS section
there for an up-to-date status of what can be passed around between threads.

=head1 AUTHOR

Elizabeth Mattijsen, <liz@dijkmat.nl>.

Please report bugs to <perlbugs@dijkmat.nl>.

=head1 HISTORY

This module started life as L<Thread::Queue::Any> and as a sub-class of
L<Thread::Queue>.  Using the conveyor belt metaphore seemed more appropriate
and therefore the name was changed.  To cut the cord with Thread::Queue
completely, the belt mechanism was implemented from scratch.

Why would you use Thread::Conveyor over Thread::Queue::Any?  Well,
Thread::Conveyor has the following extra features:

=over 2

=item It works with Perl 5.8.0

Shared arrays leak memory very badly in Perl 5.8.0.  Therefore, you cannot
really use Thread::Queue in Perl 5.8.0, and consequently cannot use
Thread::Queue::Any in any type of production environment.

=item It provides throttling

A thread that enqueues very many values quickly, can cause a large amount of
memory to be used.  With throttling, any thread that enqueues will have to
wait until there is "room" on the belt again before continuing.  See methods
"minboxes" and "maxboxes".

=item You can check for a new value without removing it from the belt

Sometimes it can be nice to check whether there is a new value on the belt
without actually removing it from the belt.  See the "peek" and "peek_dontwait"
methods.

=item You can reset the entire belt

Sometimes you want to be able to reset the contents of the belt.  See the
"clean" and "clean_dontwait" methods for that.

=item You can get everything from the belt in one go

Sometimes you want everything that's on the belt in one go.  That can also
ba accomplished with the "clean" and "clean_dontwait" methods.

=back

=head1 COPYRIGHT

Copyright (c) 2002, 2003, 2004, 2007, 2010 Elizabeth Mattijsen <liz@dijkmat.nl>.
All rights reserved.  This program is free software; you can redistribute it
and/or modify it under the same terms as Perl itself.

=head1 SEE ALSO

L<threads>, L<threads::shared>, L<Thread::Queue>, L<Thread::Queue::Any>,
L<Thread::Serialize>.

=cut
