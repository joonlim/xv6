# Displays the number of times each process ran
printf "BEST:  " ; grep BEST $1 | wc -l
printf "HIGH:  " ; grep HIGH $1 | wc -l
printf "LOW:   " ; grep LOW $1 | wc -l
printf "WORST: " ; grep WORST $1 | wc -l