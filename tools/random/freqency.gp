#!/usr/bin/gnuplot -c

set terminal x11 enhanced persist

# Acknowledgments:
#	Neil Carter: "gnuplot Frequency Plot"
#	URL: http://psy.swansea.ac.uk/staff/carter/gnuplot/gnuplot_frequency.htm

if (0) {
	set terminal dumb 38 20
	set xtics 1
	set ytics 50
}

if (ARG1) {
	bin_width = ARG1
} else {
	bin_width = 0.1
}

# Each bar is half the (visual) width of its x-range.
set boxwidth bin_width * 0.5 absolute
set style fill solid 1.0 noborder


bin_number(x) = floor(x/bin_width)

rounded(x) = bin_width * (bin_number(x) + 0.5)

plot '/dev/stdin' using (rounded(column(1))):(1) smooth frequency with boxes
