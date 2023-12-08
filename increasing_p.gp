# Gnuplot script for plotting data from a file

# Set the terminal to png and specify the output filename
set terminal png
set output 'increasing_p.png'

# Set the title of the plot
set title 'Sequential vs Parallel as p increases'

# Set the labels for the x and y axes
set xlabel 'p'
set ylabel 'time (s)'

# Set the style of the bars
set style data histograms
set style histogram cluster gap 1

# Set the position of the legend to the top left
set key left top

# Plot the data from the file using bars
plot 'graph_data.txt' using 1:2:xtic(1) with boxes title 'Seq', \
     '' using 1:3 with boxes title 'Par'
