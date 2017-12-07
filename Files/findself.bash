HERE="$(dirname "$(readlink -f "${0}")")" # note pwd is unreliable as it returns the current working directory of the terminal, for example cd / ; ./git/UPM/UPM returns /
echo script directory = $HERE
