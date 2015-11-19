local INFILES=$(echo "$1" | tr ',' ' ')
local CONFIG=$2
local XS=$3
local result_files=""
count = 0
for f in "$INFILES"
do
    out_file=$(basename "$f")
    out_file="${out_file%.*}.mini.root"
    echo ./bin/skim-tree "$CONFIG" "$f" "$out_file" "$XS"
    ./bin/skim-tree "$CONFIG" "$f" "$out_file" "$XS"
    result_files+=" $out_file"
done
if [ "$CONFIG" == "Systematics.conf" ]; then
    for syst in MuonSmeared{IDUp,MSUp,Low,Up,} TrackZ{Filtered,Smeared}Jets
    do
	hadd "total_${syst}.mini.root" *${syst}.mini.root
    done
else
    hadd output.mini.root "$result_files"
fi
   
