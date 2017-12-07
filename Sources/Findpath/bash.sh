#!./bash
echo catting myself
cat $BASH_SOURCE
echo
echo catted and outputted...
echo outputting script results...
echo echoing args: $@
echo echoing args: $*
echo echoing args: $1 $2 $3 $4 $5
printf "You have executed ./bash.sh from $(pwd) succesfully.\n"
pwd
echo current directory contents
ls ./
echo parent directory contents
ls ../
sleep 3
./bash --help
