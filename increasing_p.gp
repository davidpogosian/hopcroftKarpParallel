# Gnuplot script for plotting data from a file

# Set the terminal to png and specify the output filename
set terminal png
set output 'increasing_p.png'

# Set the title of the plot
set title 'Sequential vs Parallel as p increases'

# Set the labels for the x and y axes
set xlabel 'p'
set ylabel 'time (s)'

# Plot the data from the file using lines
plot 'graph_data.txt' using 1:2 with points title 'Seq', \
     '' using 1:3 with points title 'Par'
