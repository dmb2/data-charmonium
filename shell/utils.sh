export PATH=${HOME}/data-charmonium/bin:$PATH
export LD_LIBRARY_PATH=${HOME}/data-charmonium/src:$LD_LIBRARY_PATH
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
    local JET_SYST=$(echo TrackZ{Filtered,Smeared,ScaledUp,RadialScaledUp}JPsiJets)
    declare -A SYST_PAIRS;
    
    for syst in $MUON_SYST $JET_SYST;
    do
	SYST_PAIRS["$syst"]="${INFILE}"
    done
    SYST_PAIRS["MuonEfficiencyUp"]="${DSID}-systematics/${DSID}.MuonEfficiencyDown.mini.root"
    SYST_PAIRS["MuonSmearedUp"]="${DSID}-systematics/${DSID}.MuonSmearedLow.mini.root"
    SYST_PAIRS["TrackZScaledUpJPsiJets"]="${DSID}-systematics/${DSID}.TrackZScaledDownJPsiJets.mini.root"
    SYST_PAIRS["TrackZRadialScaledUpJPsiJets"]="${DSID}-systematics/${DSID}.TrackZRadialScaledDownJPsiJets.mini.root"
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
    skim-tree -c Systematics.conf -i $FILE -o $(echo $(basename $FILE .root)).mini.root -x $XS
    mkdir -p "$DSID-systematics"
    mv $DSID.{M,T}*.mini.root "$DSID-systematics/"
    summarize_systematics $(echo $(basename $FILE .root)).mini.root
}

# usage: get_ami_xsec DSID.ami_dset/
# returns string (in fb): DSID=xsec
get_ami_xsec(){
    local dset_info=$(ami show dataset info $1)
    local xsec=$(echo $dset_info | grep -o "crossSection : [0-9\.E+-]*" | sed 's,crossSection : ,,' | sed 's/[eE]+\{0,1\}/*10^/g')
    local filt_eff=$(echo $dset_info | grep -o "GenFiltEff : [0-9\.E+-]*" | sed 's,GenFiltEff : ,,' | sed 's/[eE]+\{0,1\}/*10^/g')
    local XS=$(awk "BEGIN {print ${xsec}*${filt_eff}*1e6}")
    local DSID=$(basename $1 | awk -F '.' '{print $2}')
    echo "${DSID}=${XS}"
}
rename_files(){
    local name_map
    declare -A name_map
    
    name_map["208025"]="208025.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_1"
    name_map["208027"]="208027.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_1"
    name_map["208028"]="208028.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_8"
    name_map["208026"]="208026.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_8"
    name_map["208024"]="208024.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_1S0_8"
    name_map["non_prompt"]="non_prompt"
    name_map["full2012"]="full2012"
    for key in "${!name_map[@]}"
    do
	mv ${key}*.mini.root "${name_map[$key]}.mini.root"
    done
}
strip_slice_name(){
    local BRANCH_NAME="$1"
    for f in $(find . -name *.root | grep "${BRANCH_NAME}"); 
    do 
    	mv "$f" $(echo "$f" | sed s,"${BRANCH_NAME}_[0-9]*_[0-9]*.",,2); 
    done
}
organize_slice_files(){
    local BRANCH_NAME="$1"
    for dir in "${BRANCH_NAME}"*/; 
    do 
    	for dsid in 20802{4..8} total; 
    	do 
    	    mkdir -p "$dir/$dsid-systematics/"
    	    mv "$dir/${dsid}."{M,T}*.mini.root "$dir/$dsid-systematics/";
    	done;
    	mv "$dir/full2012.mini"{.mini,}.root
    	mv "$dir/non_prompt.mini"{.mini,}.root
    done
}
slice_file(){
    local DSID="$1"
    local BRANCH_NAME="$2"
    local INIT_VAL="$3"
    local BIN_VALS="$4"
    
    python/split_by_branch -a "${BIN_VALS}"\
			   ${DSID}*.mini.root\
			   -i "${INIT_VAL}" -b "${BRANCH_NAME}"
    for bin_label in $(ls ${DSID}*${BRANCH_NAME}*.root | awk -F '.' '{print $3}' | sort -u); do 
	mkdir -p "slices/${bin_label}"
	mv ${DSID}*${bin_label}.mini.root slices/${bin_label}/
    done
}
setup_slices(){
    local BRANCH_NAME="$1"
    local INIT_VAL="$2"
    local BIN_VALS="$3"
    # 20802{4..8} non_prompt.mini.root full2012.mini.root
    for dsid in  20802{4..8}-systematics/ 20802{4..8} non_prompt total full2012
    do
	slice_file "$dsid" "$BRANCH_NAME" "$INIT_VAL" "$BIN_VALS"
    done
    cd slices;
    strip_slice_name "${BRANCH_NAME}"
    organize_slice_files "${BRANCH_NAME}"
    cd ../;
}
publish_plots(){
    local BRANCH_NAME="$1"
    local NOTE_DIR="$2"
    local OLDPWD="$PWD"
    for dir in "slices/${BRANCH_NAME}"*/
    do
	cd "$dir"
	 mkdir -p "${NOTE_DIR}/plots/${dir}"{correlation,splot-bkg,syst-plots-20802{4..8}}/
	 mv *_corr.pdf "${NOTE_DIR}/plots/${dir}correlation/"
	 mv *_bkg.pdf "${NOTE_DIR}/plots/${dir}splot-bkg/"
	 mv *_splot.pdf "${NOTE_DIR}/plots/${dir}"
	 mv *_sbs_p8.pdf "${NOTE_DIR}/plots/${dir}"
	for dsid in 20802{4..8}
	do
	     summarize_systematics $dsid.*.mini.root
	     print-syst-plots $dsid-systematics/$dsid.{M,T}*.hist.root
	     mv *_syst.pdf "${NOTE_DIR}/plots/${dir}syst-plots-${dsid}/"
	done
	
	cd "$OLDPWD"
    done
}
analyze_slices(){
    local BRANCH_NAME="$1"
    local CMD="$2"
    local OLDPWD="$PWD"
    shift 2
    for dir in "slices/${BRANCH_NAME}"*/
    do
	cd "$dir"
	${CMD} $@
	cd "$OLDPWD"
    done
}
slice_to_tex(){
    local slice="$1"
    echo $slice | sed 's/n/-/g'|\
	awk -F '_' '{print $(NF-1)/100 " GeV < " $2 " " $1 " < " $(NF)/100 " GeV"}' |\
	sed 's/pt/$p_{T}(/g' |\
	sed 's/eta/$\\eta(/g'|\
	sed 's/rap/$y(/g'|\
	sed 's/jpsi/J\/\\\\psi)$/g'
}
process_data(){
      local f;
      local i;
      i=0;
      DSID=$(echo $1 | awk -F '.' '{print $3}')
      OUTDIR=$(dirname $1)
      for f in $@
      do
          skim-tree -c AnalysisCuts.conf -i $f -o "$OUTDIR/$DSID.$i.mini.root" -x -1
          (( i += 1 ))
      done
  }
