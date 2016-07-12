# Set up path to have ./bin on top, could have unintended consquences
# outside of this analysis if your working in a directory that has bin
# with binaries in it
export PATH=./bin:$PATH
export LD_LIBRARY_PATH=./src:$LD_LIBRARY_PATH
# Helper functions for bookkeeping, should be sourced not executed 

# usage: proc_file foo.txt echo [cmd opts]
proc_file(){
    local LINE; local FILE; local CMD;
    FILE=$1
    CMD=$2
    shift 2
    cat $FILE | \
	while read LINE; do
	    is_comment=$(echo $LINE | grep "^#.*")
            [ "x${is_comment}" != "x" ]  && continue
	    $CMD $LINE $@
	done
}

# usage: proc_file samples.txt get_and_manage
get_and_manage(){
    local DSET; local TAG; local LABEL;
    DSET=$(echo $1 | sed 's,/$,_EXT0,g')
    TAG=$(echo $DSET | awk -F '.' '{print $5}' | awk -F '-' '{print $1}')
    LABEL=$(echo $DSET | awk -F '.' '{print $5}' | awk -F '-' '{for(i=2;i<NF;i++) printf "%s",$i "-"; if(NF) printf "%s",$NF; printf ORS}' | sed 's,_EXT0,,g')
    rucio download user.davidb:$DSET
    dset-manage -r root://eosatlas.cern.ch//eos/atlas/user/d/davidb/charm/NTUP -t $TAG -l $LABEL $DSET
    rm -rf $DSET
}

# usage: submit_dset dset_name/ tag [--pathena-opts]
submit_dset(){
    local OUT_DS; local IN_DS; local TAG;
    IN_DS=$1
    TAG=$2
    OUT_DS=$(echo $1 | awk -F '.' '{ print "user.davidb." $2 "." $3 ".mini-"}')
    OUT_DS+="${TAG}/"
    shift 2
    pathena BPhysAnalysisMasterAuto.py \
            --inDS="${IN_DS}"\
            --outDS="${OUT_DS}"\
            --extOutFile="ntuple.root"\
            $@
}

#usage: make_config cross_sections.conf AnalysisCutsMC.conf DSID.file.root
py_mul(){
    python -c "print $1"
}
make_config(){
    local INFILE=$1;
    local XSEC_FILE=$2;
    local BASECONF=$3;
    local DSID; 
    local INPATH=../rucio/mini/mj-v5/;
    # local INPATH=root://eosatlas.cern.ch//eos/atlas/user/d/davidb/charm/NTUP/mini/mj-v5/;
    DSID=$(echo ${INFILE} | awk -F '.' '{print $1}')
    samp=$(echo ${INFILE} | sed 's/.root//g')
    xsec=$(grep "$DSID" "$XSEC_FILE" | awk -F '=' '{print $2}')
    sed s/OSAMP/${samp}/g $BASECONF > ${samp}_config.conf
    sed -i s,SAMP,${INPATH}${samp},g ${samp}_config.conf
    sed -i s/XSEC/${xsec}/g ${samp}_config.conf 
}

#usage: make_mini DSID.foobar.root
make_mini(){
    samp=$(echo $1 | sed 's/.root//g')
    skim-tree ${samp}_config.conf
}

#usage: proc_files dir cmd [cmd opts]
#usage: proc_files dir make_mini [opts]
proc_files(){
    local DIR; local CMD;
    DIR=$1
    CMD=$2
    shift 2
    for f in $(ls $DIR);
    do
	$CMD $f $@
    done
}
summarize_systematics(){
    local INFILE=$1;
    local DSID=$(echo ${INFILE} | awk -F '.' '{print $1}');
    local MUON_SYST=$(echo MuonSmeared{IDUp,MSUp,Up,});
    local JET_SYST=$(echo TrackZ{Filtered,Smeared,ScaledUp,RadialScaledUp}Jets)
    declare -A SYST_PAIRS;
    
    for syst in $MUON_SYST $JET_SYST;
    do
	SYST_PAIRS["$syst"]="${INFILE}"
    done
    SYST_PAIRS["MuonEfficiencyUp"]="${DSID}-systematics/${DSID}.MuonEfficiencyDown.mini.root"
    SYST_PAIRS["MuonSmearedUp"]="${DSID}-systematics/${DSID}.MuonSmearedLow.mini.root"
    SYST_PAIRS["TrackZScaledUpJets"]="${DSID}-systematics/${DSID}.TrackZScaledDownJets.mini.root"
    SYST_PAIRS["TrackZRadialScaledUpJets"]="${DSID}-systematics/${DSID}.TrackZRadialScaledDownJets.mini.root"
    for syst in "${!SYST_PAIRS[@]}"
    do
	# echo "$syst ${SYST_PAIRS[$syst]}" 
	make-systematic-plots "${DSID}-systematics/${DSID}.$syst.mini.root" "${SYST_PAIRS[$syst]}" "${INFILE}" "${DSID}-systematics/${DSID}.${syst}.hist.root"
    done
}
process_systematics(){
    local FILE=$1
    local DSID=$(echo $(basename $FILE) | awk -F '.' '{print $1}')
    local XS=$(grep "$DSID" cross_sections.conf | awk -F '=' '{print $2}')
    skim-tree Systematics.conf $FILE $(echo $(basename $FILE .root)).mini.root $XS
    mkdir -p "$DSID-systematics"
    mv $DSID.{M,T}*.mini.root "$DSID-systematics/"
    summarize_systematics $(echo $(basename $FILE .root)).mini.root
}

# usage: get_ami_exsec DSID.ami_dset/
# returns string (in fb): DSID=xsec
get_ami_xsec(){
    local dset_info=$(ami show dataset info $1)
    local xsec=$(echo $dset_info | grep -o "crossSection : [0-9\.E+-]*" | sed 's,crossSection : ,,' | sed 's/[eE]+\{0,1\}/*10^/g')
    local filt_eff=$(echo $dset_info | grep -o "GenFiltEff : [0-9\.E+-]*" | sed 's,GenFiltEff : ,,' | sed 's/[eE]+\{0,1\}/*10^/g')
    local XS=$(awk "BEGIN {print ${xsec}*${filt_eff}*1e6}")
    local DSID=$(basename $1 | awk -F '.' '{print $2}')
    echo "${DSID}=${XS}"
}
