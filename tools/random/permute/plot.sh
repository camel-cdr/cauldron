#!/usr/bin/env bash

printbest=false
gpeval="set terminal wxt persist;"
cmd="./evalpow2"
while getopts '0bcn:q:o:' OPTION
do
	case $OPTION in
	0)	cmd="${cmd} -0"
		;;
	b)	printbest=true
		;;
	c)	cmd="${cmd} -c"
		;;
	n)	cmd="${cmd} -n $OPTARG"
		;;
	o)	gpeval="set terminal ${OPTARG##*.}; set output '$OPTARG';"
	#o)	gpeval="set terminal ${OPTARG##*.} size 400,300; set output '$OPTARG';"
		;;
	q)	cmd="${cmd} -q $OPTARG"
		;;
	?)	printf "Usage: %s: [-c] [-n bits] [-q quality] files" $(basename $0) >&2
		exit 2
		;;
	esac
done

gpeval="${gpeval}set title \"$0 ${@:3:$(($OPTIND - 3))}\" offset 0,-0.5;"
shift $(($OPTIND - 1))
gpeval="${gpeval}filenames=\""


DIR=`mktemp -d`
#trap '{ rm -rf -- "$DIR"; }' EXIT

if [ $printbest ]
then
	eval "$cmd -o $DIR/best.so.csv -b"
	gpeval="${gpeval}$DIR/best.so.csv "
fi

i=0
for so in "$@"; do
	csv="$DIR/$(basename $so).csv"
	eval "$cmd -o $csv -l $so"
	i=$((i+1))
	gpeval="${gpeval}$csv "
done
gpeval="${gpeval}\""


cat <<EOF > $DIR/plot.gp
set grid lt 1 lc rgb "grey" lw 0.5

set datafile separator ','
set logscale y

if (1) {
	set xlabel "bits"
	set ylabel "bias"
} else {
	set xlabel "bits" offset 0,1
	set ylabel "bias" offset 4,0
	set key spacing 0.6
	set yrange[-1:5000]

	set tmargin 2
	set lmargin 5
	set bmargin 2
}


plot for [file in filenames] file u 1:2 w l lw 2 t system(sprintf("basename %s .so.csv",file))
EOF

gnuplot -e "$gpeval" $DIR/plot.gp

