#!/usr/bin/gnuplot -c

set terminal x11 enhanced persist

if (0) {
	set terminal dumb 38 20
	set xtics 1
	set ytics 50
}

if (ARG1) {
	bw = ARG1
} else {
	bw = 0.1
}

# Adopted from "http://www.phyast.pitt.edu/~zov1/"

unset key
set boxwidth 0.7 relative
set style fill solid 1.0 noborder
bin(x, width) = width * floor(x / width)
plot '/dev/stdin' using (bin($1, bw)):(1.0) smooth frequency with boxes
