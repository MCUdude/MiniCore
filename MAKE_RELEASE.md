# Publishing a version of this core

Notes for whoever publishes this branch. Nothing here is read by the board
manager; the index is.

## A release

Normally the release workflow of the core does all of this: `./avr/extras/release.sh 3.2.0`
on `master` checks, tags and pushes, and the tag starts the workflow, which
runs the scripts below and commits the index here. By hand it is two scripts,
run here in this order. Each only adds what the index is missing and says so
when there is nothing to do, so calling one twice does no harm.

    bash Add_avrdude_release.sh        # the newest avrdude, if the index lacks it
    bash Boards_manager_release.sh     # the core itself

The platform entry the last script writes depends on the newest avrdude the
index knows, which is why the other one runs first. The debug tools are not a
dependency: since 3.2.0 they come from the separately installed AvrDebugger
package.
Without an argument it packs the newest release of the repository. Commit the
changed index together with the archive the script leaves behind.

Where the archives are downloaded from is the `AUTHOR` at the top of
`Boards_manager_release.sh`: the URL it writes into the index is
`https://<AUTHOR>.github.io/<REPOSITORY>/`. That is the fork which publishes
this branch, not necessarily the one the sources come from.

## A release candidate

The GitHub API never reports a pre-release as the latest release, so the tag has
to be named:

    bash Boards_manager_release.sh v3.2.0-rc1

A candidate has no business in the index that everybody's board manager reads.
Set `INDEX_FILE`, and both scripts write a second index instead:

    export INDEX_FILE=package_MCUdude_MiniCore_rc_index.json
    cp package_MCUdude_MiniCore_index.json "$INDEX_FILE"   # once, to create it
    bash Add_avrdude_release.sh
    bash Boards_manager_release.sh v3.2.0-rc1

Being a copy, that index offers the released versions as well, so a tester can
go back to one of those. Its name still matches `package_*_index.json`, which
the IDE demands of an index URL. Testers use that URL in place of the usual one.

When the candidate becomes the real thing, run the scripts again without
`INDEX_FILE`: the released index then gets the final version, and the candidates
stay where they are.

## Re-packing a version that is already published

Do not. The archive is not reproducible: the script creates the top directory
with `mkdir`, so its timestamp is the time of packing, and the compressed stream
differs even when every file in it is identical. A second run therefore yields a
different checksum from the one in the index. The scripts refuse a version that
the index already carries, which is what that check is for.
