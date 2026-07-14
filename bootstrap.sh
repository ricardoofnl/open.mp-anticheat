#!/usr/bin/env bash
#
# Fetch the open.mp SDK + network + pawn dependencies into lib/. Run once after
# cloning this repo (or instead of `git submodule update --init --recursive` if
# you didn't clone with submodules).
#
set -e
cd "$(dirname "$0")"
mkdir -p lib

clone() {
	url="$1"
	dst="$2"
	if [ -e "$dst/CMakeLists.txt" ] || [ -d "$dst/.git" ]; then
		echo "== $dst already present, skipping"
	else
		echo "== cloning $url -> $dst"
		git clone --recursive --depth 1 "$url" "$dst"
	fi
}

# --recursive matters for the SDK (it pulls glm / robin-hood / span-lite /
# string-view-lite as its own submodules).
clone https://github.com/openmultiplayer/open.mp-sdk      lib/sdk
clone https://github.com/openmultiplayer/open.mp-network  lib/network
clone https://github.com/openmultiplayer/compiler         lib/pawn
clone https://github.com/openmultiplayer/pawn-natives      lib/pawn-natives

echo
echo "Done. Configure & build with:"
echo "  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release"
echo "  cmake --build build --config Release"
