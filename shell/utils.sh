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
    LABEL=$(echo $DSET | awk -F '.' '{print $5}' | awk -F '-' '{print $2 "-" $3}' | sed 's,_EXT0,,g')
    rucio download user.davidb:$DSET
    mkdir -p $DSET; 
    mv user.davidb/*.root* $DSET
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

#usage: make_config DSID.foobar.root
make_config(){
    local xsecs; local DSID;
    declare -A xsecs
    xsecs["208003"]=55717
    xsecs["208004"]=1361.5
    xsecs["208413"]=2.190e6
    xsecs["208022"]=1.682e7
    xsecs["208023"]=1.740e8
    xsecs["208400"]=6.191e8
    xsecs["208401"]=3.906e8
    xsecs["208432"]=7.407e9
    xsecs["108606"]=3.855e6
    xsecs["108601"]=5.042e6
    xsecs["208024"]=12.515716 
    xsecs["208025"]=586.61009 
    xsecs["208026"]=15.605898 
    xsecs["208027"]=0.30668153
    xsecs["208028"]=8256.2234 
    xsecs["singlet"]=586.9
    xsecs["octet"]=8284.3
    DSID=$(echo $1 | awk -F '.' '{print $1}')
    samp=$(echo $1 | sed 's/.root//g')
    echo ${samp}
    sed s/SAMP/${samp}/g AnalysisCutsMC_BKG.conf > ${samp}_config.conf
    sigma=${xsecs[${DSID}]}
    sed -i s/XSEC/$sigma/g ${samp}_config.conf 
}

#usage: make_mini DSID.foobar.root
make_mini(){
    samp=$(echo $1 | sed 's/.root//g')
    ./skim-tree ${samp}_config.conf
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
