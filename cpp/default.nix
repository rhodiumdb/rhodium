{ }:

let rev = "65d6153aec85c8cb46023f0a7248628f423ca4ee";
    url = "https://github.com/NixOS/nixpkgs/archive/${rev}.tar.gz";
    sha256 = "1cjd7253c4i0wl30vs6lisgvs947775684d79l03awafx7h12kh8";
    nixpkgs = fetchTarball { inherit url sha256; };
    pkgs = import nixpkgs { config = {}; overlays = []; };
in rec {
  inherit pkgs;
  rdss = pkgs.callPackage ./.nix/rdss.nix {};
}
