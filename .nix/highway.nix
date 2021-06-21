{ stdenv, fetchFromGitHub, cmake }:

stdenv.mkDerivation rec {
  pname = "highway";
  version = "0.12.2";

  src = fetchFromGitHub {
    owner  = "google";
    repo   = "highway";
    rev    = version;
    sha256 = "0w5y8qwnvqq78pgc11bg4a4ilraymv57b95pljf1cqxwd17wkp1y";
  };

  nativeBuildInputs = [ cmake ];
}
