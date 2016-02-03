INFILES=$(echo "$1" | tr ',' ' ')
CONFIG=$2
XS=$3
result_files=""
count=0
export LD_LIBRARY_PATH=${PWD}/src:${LD_LIBRARY_PATH}
if [ "x${XS}" =="x" ]; then
    XS="-1"
fi
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
    local MUON_SYST=$(echo MuonSmeared{IDUp,MSUp,Up,});
    local JET_SYST=$(echo TrackZ{Filtered,Smeared,ScaledUp,RadialScaledUp}Jets)
    for syst in $MUON_SYST $JET_SYST;
    do
	echo hadd "total_${syst}.mini.root" *${syst}.mini.root
	if [ "$count" == "1" ]; then
	    cp *${syst}.mini.root "total_${syst}.mini.root"
	else
	    echo hadd "total_${syst}.mini.root" *${syst}.mini.root
	    hadd "total_${syst}.mini.root" *${syst}.mini.root
	fi
    done
fi
if [ "$count" == "1" ]; then
    cp $result_files output.mini.root 
else 
    echo hadd output.mini.root "$result_files"
    hadd output.mini.root "$result_files"
fi
   
