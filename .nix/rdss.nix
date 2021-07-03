{ stdenv, lib, cmake, pkgconfig
, abseil-cpp, gtest, highway, rapidcheck, tree-sitter, z3
}:

stdenv.mkDerivation {
  pname = "rdss";
  version = "0.1";

  src = lib.cleanSourceWith {
    filter = name: type: !(lib.hasPrefix "build" (baseNameOf (toString name)));
    src = lib.cleanSource ./..;
  };

  buildInputs = [ abseil-cpp gtest highway rapidcheck tree-sitter z3 ];

  nativeBuildInputs = [ cmake pkgconfig ];

  doCheck = true;

  checkTarget = "test";
}
