# sed -i 's/" "/"0"/g' systemPerf.csv

set datafile separator ','
set terminal svg size 900,1500
set output 'systemPerf.svg'

set xdata time
set timefmt "%m/%d/%Y %H:%M:%S:%.3S"

set style line 1 \
    linecolor rgb '#0060ad' \
    pointtype 7 pointsize 0.4

set format x "%H:%M"
set multiplot layout 8,1

bytes_to_mbytes(x)=x/(1024**2)

set yrange [0:]
plot "systemPerf.csv" using 1:(bytes_to_mbytes($5)) with linespoints linestyle 1 title "Commit Limit (MBytes)"
plot "systemPerf.csv" using 1:4 with linespoints linestyle 1 title "% Committed Bytes In Use"
plot "systemPerf.csv" using 1:2 with linespoints linestyle 1 title "% Processor Time"
plot "systemPerf.csv" using 1:(bytes_to_mbytes($3)) with linespoints linestyle 1 title "Committed MBytes"
plot "systemPerf.csv" using 1:8 with linespoints linestyle 1 title "Available MBytes"
plot "systemPerf.csv" using 1:6 with linespoints linestyle 1 title "% Page File Usage"
plot "systemPerf.csv" using 1:9 with linespoints linestyle 1 title "% Free Space (Logical C:)"
