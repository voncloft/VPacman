# VPacman

Qt 6 graphical interface for Scratchpkg. Browse and search packages from the enabled repositories in
`/etc/scratchpkg.repo`, install selected packages, upgrade selections or
the system, and view outdated packages and command output.

The label below the output box shows your selected packages and the dependencies
Scratchpkg would install. It updates when you check or uncheck a package. Already
installed dependencies are omitted; shared dependencies appear once. Checking a
box only runs Scratch's read-only dependency lookup, without installing anything.

## Build and install

Requires CMake 3.21+, a C++17 compiler, Qt 6 Widgets, and Scratchpkg. Qt Test is
required when tests are enabled. The running app uses `/usr/bin/scratch` and
`sudo -n` for package changes; sudo authorization must already be configured.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_TESTING=ON
cmake --build build -j2
ctest --test-dir build --output-on-failure
sudo cmake --install build
```

The only executable installed is `/usr/bin/VPacman`. The desktop entry is
`/usr/share/applications/vpacman.desktop`; pin this application entry to your
Plasma panel. Its icon is installed in the hicolor icon theme and embedded in
the executable as a fallback. No binary or build directory belongs in Git.

## Upload changes

This checkout tracks `master` on `git@github.com:voncloft/VPacman.git`.

```sh
cd /home/von/Desktop/coding-projects/Vpacman
git status
git add -A
git commit -m "Update VPacman"
git push
```

## Changes

### 4.3.4 - 2026-10-29

- Show selected packages and their pending dependencies below the command output.
- Resolve dependencies with Scratchpkg asynchronously, using the configured repos.
- Keep the preview current across checkbox changes, searches, and reloads.

### 4.3.3 - 2026-10-29

- Read enabled repository paths from /etc/scratchpkg.repo, including extra and
  repositories outside /sources/Voncloft-OS-dev. Ignore blank/commented lines.
- Report unreadable repository configuration and unavailable repo directories.

### 4.3.2 - 2026-10-29

- Include the Qt 6 port and CMake build/install rules in the source repository.
- Bundle and install a VPacman icon; associate windows with vpacman.desktop.
- Install one executable at /usr/bin/VPacman, with no lowercase alias.
- Read the current Voncloft-OS-dev package tree and use Scratch's built-in
  outdated and upgrade commands; preserve exact selected package names.
- Remove committed binaries, generated build files, and obsolete Qt 5 project files.
