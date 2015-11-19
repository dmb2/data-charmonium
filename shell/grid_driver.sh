INFILES=$(echo "$1" | tr ',' ' ')
CONFIG=$2
XS=$3
result_files=""
count=0
for f in $INFILES
do
    out_file=$(basename "$f")
    out_file="${count}${out_file%.*}.mini.root"
    echo ./bin/skim-tree "$CONFIG" "$f" "$out_file" "$XS"
    ./bin/skim-tree "$CONFIG" "$f" "$out_file" "$XS"
    result_files+=" $out_file"
    (( count+=1 ))
done
if [ "$CONFIG" == "Systematics.conf" ]; then
    for syst in MuonSmeared{IDUp,MSUp,Low,Up,} TrackZ{Filtered,Smeared}Jets
    do
	echo hadd "total_${syst}.mini.root" *${syst}.mini.root
	hadd "total_${syst}.mini.root" *${syst}.mini.root
    done
else
    echo hadd output.mini.root "$result_files"
    hadd output.mini.root "$result_files"
fi
   
