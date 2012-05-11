#!/bin/sh

set -vxe

create_orig () {
  local ver="$1" n=freeswitch-htdocs-slim
  local d=$n-$ver f=${n}_${ver}
  mkdir -p debbuild/$d
  find * -maxdepth 0 -type f | xargs tar -cvf - | tar -x -C debbuild/$d -vf -
  tar -c -C debbuild -vf debbuild/$f.orig.tar $d
  xz -9ev debbuild/$f.orig.tar
  mv debbuild/$f.orig.tar.xz ../
}

case "$1" in
  create-orig) create_orig "$2" ;;
esac

exit 0
