{ clangStdenv, lib, cmake
, abseil-cpp, gtest, z3, highway, rapidcheck
}:

clangStdenv.mkDerivation {
  pname = "rdss";
  version = "0.1";

  src = lib.cleanSourceWith {
    filter = name: type: !(lib.hasPrefix "build" (baseNameOf (toString name)));
    src = lib.cleanSource ./..;
  };

  buildInputs = [ abseil-cpp gtest highway rapidcheck z3 ];

  nativeBuildInputs = [ cmake ];
}
