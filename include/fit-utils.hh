class RooAddPdf;
class RooArgSet;

RooAddPdf* build_model();
RooArgSet* Fit(RooAddPdf* model);
void print_fit_results(RooAddPdf* model);
