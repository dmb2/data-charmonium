#include <iostream>
#include <list>
#include <algorithm>
#include "simple-parser.hh"
#include "root-sugar.hh"

#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
void usage(const char* name){
  MSG("Usage: "<<name<<" cross_sections.conf n_requested [MC Files]");
  MSG("\t cross_sections.conf contains the cross sections for each 6 digit");
  MSG("\t DSID in fb, in the form DSID=xsec on each line. [MC Files] is a ");
  MSG("\t list of files that starts with the 6 digit DSID, ie DSID.foobar.root");
}
typedef struct {
  std::string region;
  double xsec;
  int nentries;
  double fraction;
  TFile* out_file;
  TTree* tree;
} sample_info;

void parse_xsecs(std::map<std::string,double>& xsecs, const char* fname){
  option_map str_xsecs;
  get_opts(fname,str_xsecs);
  for(option_map::const_iterator it=str_xsecs.begin(); it!=str_xsecs.end();
      ++it){
    const std::string& dsid=it->first;
    const std::string& xsec=it->second;
    xsecs[dsid]=atof(xsec.c_str());
  }
}
void copy_tree(sample_info& sample_meta_info,TTree* input_tree,const double xsec, const double n_requested){
  // TTree* tmp_tree = NULL;
  sample_meta_info.out_file->cd();
  if(sample_meta_info.tree==NULL){
    sample_meta_info.tree = input_tree->CloneTree(0);
  }
  sample_meta_info.tree->CopyAddresses(input_tree);
  sample_meta_info.nentries=(xsec/sample_meta_info.xsec)*(n_requested*sample_meta_info.fraction);
  // gROOT->cd();
  // tmp_tree = input_tree->CopyTree(sample_meta_info.region.c_str());
  sample_meta_info.out_file->cd();
  sample_meta_info.tree->CopyEntries(input_tree,sample_meta_info.nentries);
  // MSG_DEBUG("Nentries: "<<sample_meta_info.nentries<<" Entries in tmp_tree: "<<tmp_tree->GetEntries()<<" Output Tree "<<sample_meta_info.tree->GetEntries());

}
int main(const int argc, const char* argv[]){
  if(argc < 4){
    usage(argv[0]);
    return 0;
  }
  std::map<std::string,double> xsecs;
  parse_xsecs(xsecs,argv[1]);
  double n_requested=atof(argv[2]);

  std::string comb_dsid = "periodA"; //periodA.CombMu.mini.root
  const char* sig_dsids[] = {"208024","208025","208026","208027","208028"};
  const char* np_dsids[]  = {"208400","108606","208202","208207"};
  std::list<std::string> signal_dsids(sig_dsids,sig_dsids + LEN(sig_dsids));
  std::list<std::string> nonprompt_dsids(np_dsids,np_dsids + LEN(np_dsids));

  std::map<std::string,sample_info> sample_meta_info;
  sample_meta_info["signal"].fraction=0.310;
  sample_meta_info["combinatoric"].fraction=0.197;
  sample_meta_info["nonprompt"].fraction=0.493;
  std::map<std::string,TTree*> samples;
  for(int i=3; i < argc; i++ ){
    std::string fname(argv[i]);
    std::vector<std::string> parts = split_string(fname,'.');
    std::string dsid=parts[parts.size()-4];
    samples[dsid]=retrieve<TTree>(fname.c_str(),"mini");
    MSG_DEBUG("Processing "<<dsid);
    if (std::find(signal_dsids.begin(),signal_dsids.end(),dsid)!=signal_dsids.end()){
      sample_meta_info["signal"].xsec+=xsecs[dsid];
    }
    else if(dsid=="periodA"){
      sample_meta_info["combinatoric"].xsec=0.766;
    }
    else if(std::find(nonprompt_dsids.begin(),nonprompt_dsids.end(),dsid)!=nonprompt_dsids.end()){
      sample_meta_info["nonprompt"].xsec+=xsecs[dsid];
    }
  }
  const std::string jpsi_mass = "(2.957 < jpsi_m && jpsi_m < 3.236)";
  const std::string jpsi_prompt = "(-1 < jpsi_tau && jpsi_tau < 0.25)";
  const std::string jpsi_nonprompt = "(0.25 < jpsi_tau && jpsi_tau < 50)";
  const std::string jpsi_combinatoric = "((2.587 < jpsi_m && jpsi_m < 2.957) || (3.236 < jpsi_m && jpsi_m < 3.467))";
  sample_meta_info["signal"].region=jpsi_mass+" && "+jpsi_prompt;
  sample_meta_info["combinatoric"].region=jpsi_combinatoric+" && "+jpsi_prompt;
  sample_meta_info["nonprompt"].region=jpsi_mass+" && "+jpsi_nonprompt;
  
  // TTree* out_tree=samples.begin()->second->CloneTree(0);
  sample_meta_info["signal"].out_file = new TFile("signal.closure.mini.root","RECREATE");
  sample_meta_info["combinatoric"].out_file = new TFile("combinatoric.closure.mini.root","RECREATE");
  sample_meta_info["nonprompt"].out_file = new TFile("nonprompt.closure.mini.root","RECREATE");
  // TTree* tmp_tree=NULL;
  
  for(std::map<std::string,TTree*>::const_iterator it=samples.begin();
      it!=samples.end(); ++it){
    const std::string& dsid = it->first;
    const double& xsec = xsecs[dsid];

    if (std::find(signal_dsids.begin(),signal_dsids.end(),dsid)!=signal_dsids.end()){
      copy_tree(sample_meta_info["signal"],samples[dsid],xsec,n_requested);
    }
    else if(dsid=="periodA"){
      copy_tree(sample_meta_info["combinatoric"],samples[dsid],0.766,n_requested);
    }
    else if(std::find(nonprompt_dsids.begin(),nonprompt_dsids.end(),dsid)!=nonprompt_dsids.end()){
      copy_tree(sample_meta_info["nonprompt"],samples[dsid],xsec,n_requested);
    }
  }
  for(std::map<std::string,sample_info>::iterator it = sample_meta_info.begin();
      it != sample_meta_info.end(); ++it){
    if(it->second.tree && it->second.nentries != it->second.tree->GetEntries()){
      MSG_ERR("Warning: did not copy a full sample for "<<it->first<<" got: "<<it->second.tree->GetEntries()<<" expected: "<<it->second.nentries);
    }
    MSG_DEBUG("Sample: "<<it->first<<" composed of: "<<it->second.tree->GetEntries());
    it->second.out_file->Write();
    it->second.out_file->Close();
  }
  return 0;
}
