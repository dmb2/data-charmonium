#pragma once
//Inspired by Rivet Units.hh, renormalized to MeV (the standard ATLAS unit)
//http://rivet.hepforge.org/code/dev/a00793_source.html
namespace Units {
  //
  // Area [L^2]
  //

  // static const double barn = 1.e-28*meter2;
  // Barn-units in terms of the pb returned by AGILe
  static const double  picobarn = 1.0;
  static const double      barn = 1.0e+12* picobarn;
  static const double millibarn = 1.0e-3 * barn;
  static const double microbarn = 1.0e-6 * barn;
  static const double  nanobarn = 1.0e-9 * barn;
  static const double femtobarn = 1.0e-15 * barn;
  static const double attobarn  = 1.0e-18 * barn;
  /*  these may cause subtle bugs, use verbose ones above
  static const double pb = picobarn; 
  static const double nb = nanobarn;
  static const double fb = femtobarn;
  */
  //
  // Energy [E]
  //
  static const double megaelectronvolt = 1.;
  static const double     electronvolt = 1.e+6*megaelectronvolt;
  static const double kiloelectronvolt = 1.e+3*megaelectronvolt;
  static const double gigaelectronvolt = 1.e-3*megaelectronvolt;
  static const double teraelectronvolt = 1.e-6*megaelectronvolt;
  static const double petaelectronvolt = 1.e-9*megaelectronvolt;

  // symbols
  static const double  eV = electronvolt;
  static const double keV = kiloelectronvolt;
  static const double MeV = megaelectronvolt;
  static const double GeV = gigaelectronvolt;
  static const double TeV = teraelectronvolt;

  //
  // Bits and Bytes
  //
  static const unsigned int byte = 1;
  static const unsigned int kilobyte = 1e3*byte;
  static const unsigned int megabyte = 1e6*byte;
  static const unsigned int gigabyte = 1e9*byte;
  static const unsigned int bit = 1;
  static const unsigned int kilobit = 1024;
  static const unsigned int megabit = 1024*kilobit;
  static const unsigned int gigabit = 1024*megabit;
  // symboles
  static const unsigned int KB=kilobyte;
  static const unsigned int MB=megabyte;
  static const unsigned int GB=gigabyte;
}

