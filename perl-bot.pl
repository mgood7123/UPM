# A simple IRC robot.
use strict;

# We will use a raw socket to connect to the IRC server.
use IO::Socket;

# The server to connect to and our details.
my $server = "irc.perl.org";
my $nick = "bash_bot_alpha";
my $login = "bash_bot_alpha";
# The channel which the bot will join.
my $channel = "#UPM";

# Connect to the IRC server.
my $sock = new IO::Socket::INET(PeerAddr => $server,
                                PeerPort => 6667,
                                Proto => 'tcp') or
                                    die "Can't connect\n";

# Log on to the server.
print $sock "NICK $nick\r\n";
print $sock "USER $login 8 * :Perl IRC Hacks Robot\r\n";

# Read lines from the server until it tells us we have connected.
while (my $input = <$sock>) {
    # Check the numerical responses from the server.
    if ($input =~ /004/) {
        # We are now logged in.
        last;
    }
    elsif ($input =~ /433/) {
        die "Nickname is already in use.";
    }
}

# Join the channel.
print $sock "JOIN $channel\r\n";

# Keep reading lines from the server.
while (my $input = <$sock>) {
    chop $input;
    if ($input =~ /^PING(.*)$/i) {
        # We must respond to PINGs to avoid being disconnected.
        print $sock "PONG $1\r\n";
    }
    elsif (($input =~ /PRIVMSG/) && ($input =~ (system "echo $input | grep kje ")))	#Regex to see if it was a message, way too simple so needs tweaking!
	{
        print $sock "PRIVMSG $channel (COMMAND_DETECTED)\r\n";
        print "[ COMMAND ] $input\n";
	}
    elsif (($input =~ /$nick/) && ($input =~ /JOIN/i))	#Regex to see if it was a message, way too simple so needs tweaking!
	{
        print "[ NOTICE ] Joined Channel: $channel\n";
	}
	else {
        # Print the raw line received by the bot.
        print "$input\n";
    }
}
