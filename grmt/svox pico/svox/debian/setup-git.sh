#!/bin/sh

set -e

echo '** Debian branches:'
for b in debian-lenny-backports debian-sid master upstream+patches; do
	git branch $b origin/$b ||:
done

echo '** Declaring upstream remote:'
git remote add upstream git://android.git.kernel.org/platform/external/svox.git ||:
git remote set-url --push upstream git+ssh://git.debian.org/git/collab-maint/svox.git

echo '** Upstream branch:'
git branch upstream upstream/master |:
git fetch upstream

echo '** List of branches:'
git branch
