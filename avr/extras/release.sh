#!/bin/bash
# Tags a release and pushes the tag, which is what sets the release workflow
# going:
#
#     ./extras/release.sh 1.3.4        # the real thing
#     ./extras/release.sh 1.3.4-rc1    # one to try out first
#     ./extras/release.sh -n 1.3.4     # say what it would do, do nothing
#
# (./avr/extras/release.sh where extras/ sits in the core's folder, as in TinyCore.)
#
# A tag is the one thing in this whole business that cannot be taken back once
# it is out: the workflow runs, the release appears, the board manager index
# gets an entry, and people have it. So everything that can be looked at
# beforehand is looked at here.
set -u

DRY=0
case "${1:-}" in
    -n|--dry-run) DRY=1; shift ;;
    -h|--help) echo "Usage: $0 [-n] <version>"; exit 0 ;;
esac
if [[ -z "${1:-}" ]] || [[ -n "${2:-}" ]]; then
    echo "Usage: $0 [-n] <version>      e.g. 1.3.4 or 1.3.4-rc1"
    exit 1
fi

# Asking git rather than hardwiring a path also covers being outside a
# repository at all, where the left side comes back empty.
if [[ "$(git rev-parse --show-toplevel 2>/dev/null)" != "$(pwd)" ]]; then
    echo "Invoke the script in the root folder of the repository"
    exit 1
fi

TAG="v${1#v}"                  # a leading v is optional on the command line
VERSION="${TAG#v}"
REMOTE="${REMOTE:-origin}"

# A candidate belongs to the version it is a candidate for, and the two worlds
# spell that differently: a core counts 1.3.4-rc1, a Python package 1.7.1rc1,
# which is what PEP 440 makes of it. Both are attempts at a release whose
# number stays as it is, so the number is what everything below compares.
base() { printf '%s' "${1%%-*}" | sed -E 's/(rc|a|b)[0-9]*$//'; }
BASE=$(base "$VERSION")

# The version is stated in different places in different projects, and where it
# is stated is also what the release is built from.
# A core keeps its platform.txt at the top or in the folder of its architecture. A
# platform.txt may set version= more than once (megaTinyCore does, as a workaround
# for an IDE bug); the last one wins, as in the IDE.
WHERE=""
for candidate in platform.txt avr/platform.txt megaavr/platform.txt; do
    if [[ -f "$candidate" ]]; then
        WHERE="$candidate"
        break
    fi
done
if [[ -n "$WHERE" ]]; then
    STATED=$(sed -n 's/^version=//p' "$WHERE" | tail -1)
elif [[ -f pyproject.toml ]]; then
    STATED=$(sed -n 's/^version *= *"\(.*\)"/\1/p' pyproject.toml | head -1)
    WHERE=pyproject.toml
else
    echo "No platform.txt and no pyproject.toml here, so there is no version to"
    echo "check against. This script is meant for a core or a Python project."
    exit 1
fi

# A release is cut from the default branch. A tag on another branch would ship
# whatever that branch happens to contain, and nobody would see it in the diff.
# Which branch that is: what the remote says, and if the remote was never
# asked (git only records it on clone, and 'main' is not a safe guess in an
# older repository), whichever of the two usual names exists here.
MAIN=$(git symbolic-ref --quiet --short "refs/remotes/${REMOTE}/HEAD" 2>/dev/null)
MAIN="${MAIN#${REMOTE}/}"
if [[ -z "$MAIN" ]]; then
    for candidate in main master; do
        git rev-parse -q --verify "refs/heads/${candidate}" > /dev/null && MAIN="$candidate" && break
    done
fi
MAIN="${MAIN:-$(git rev-parse --abbrev-ref HEAD)}"
BRANCH=$(git rev-parse --abbrev-ref HEAD)
if [[ "$BRANCH" != "$MAIN" ]]; then
    echo "A release is built from ${MAIN}, but this is ${BRANCH}"
    exit 1
fi
if [[ -n $(git status -s -uno) ]]; then
    echo "There are uncommitted changes, and they would not be in the release"
    exit 1
fi
git fetch -q "$REMOTE" || echo "warning: could not reach ${REMOTE}, working with what is here"
if [[ -n $(git log "${REMOTE}/${MAIN}..HEAD" --oneline 2>/dev/null) ]]; then
    echo "There are unpushed commits:"
    git log "${REMOTE}/${MAIN}..HEAD" --oneline
    exit 1
fi

if [[ "$(base "$STATED")" != "$BASE" ]]; then
    echo "${WHERE} says ${STATED}, but the tag would say ${BASE}"
    echo "Set the version first; a candidate keeps the version it is a candidate for"
    exit 1
fi
# Where a version is written down for people to read, it has to be written down
# for this one, because that text ends up on the release page. Headings are
# spelled '## v1.3.4', '## 1.7.1 (13-Oct-2026)' and '## Release v1.7.1' in the
# three files this script meets, so all three forms count.
for notes in CHANGELOG.md docs/release-notes.md; do
    if [[ -f "$notes" ]] \
       && ! grep -Eq "^#+[[:space:]]*(Release[[:space:]]+)?v?${BASE//./\\.}([[:space:]]|$|\()" "$notes"; then
        echo "${notes} has no entry for ${BASE}"
        exit 1
    fi
done
if git rev-parse -q --verify "refs/tags/${TAG}" > /dev/null \
   || git ls-remote --exit-code --tags "$REMOTE" "$TAG" > /dev/null 2>&1; then
    echo "The tag ${TAG} exists already. A published tag is not moved: pick the"
    echo "next candidate, or delete the tag here and on ${REMOTE} if it never left."
    exit 1
fi

# The same checks the release workflow runs, so a failure costs a second here
# instead of a red run and a tag that has to be cleaned up afterwards.
# The checker sits beside this script, wherever that is (extras/ or avr/extras/),
# and is told where the core is: in the folder of its platform.txt.
HERE="$(cd "$(dirname "$0")" && pwd)"
if [[ -f "$HERE/check_core.py" ]] && command -v python3 > /dev/null; then
    python3 "$HERE/check_core.py" --root "$(dirname "$WHERE")" --tag "$TAG" || exit 1
fi

echo
echo "About to tag ${BRANCH} at $(git rev-parse --short HEAD) as ${TAG}"
echo "and push it to ${REMOTE}, which starts the release."
# A candidate is published, but not where people look for the current version.
case "$TAG" in
    *rc*)
        if [[ "$WHERE" == pyproject.toml ]]; then
            echo "As a candidate: a pre-release on GitHub, and nothing on PyPI."
        else
            echo "As a candidate: a pre-release, kept out of the stable board manager index."
        fi ;;
esac
if ! grep -rqs "^ *tags:" .github/workflows/; then
    echo "Note: no workflow here triggers on a tag, so the release itself still"
    echo "has to be created (gh release create '${TAG}' --generate-notes)."
fi
if [[ "$DRY" == 1 ]]; then
    echo
    echo "Dry run, so only what would happen:"
    echo "    git tag -a ${TAG} -m ${TAG}"
    echo "    git push ${REMOTE} ${TAG}"
    exit 0
fi
read -r -p "Go ahead? [y/N] " answer
case "$answer" in [yY]|[yY][eE][sS]) ;; *) echo "Nothing was tagged."; exit 1 ;; esac

git tag -a "$TAG" -m "$TAG" || exit 1
git push "$REMOTE" "$TAG" || { echo "The tag is here but did not reach ${REMOTE}."; exit 1; }
echo "Pushed ${TAG}. The run shows up under Actions within a few seconds."
