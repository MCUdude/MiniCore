#!/bin/bash 

##########################################################
##                                                      ##
## Shell script for generating a boards manager release ##
## Created by MCUdude                                   ##
## Requires wget, jq and a bash environment             ##
##                                                      ##
##########################################################

# Change these to match your repo
AUTHOR=MCUdude       # Github user name
REALAUTHOR=MCUdude   # real author
REPOSITORY=MiniCore  # Github repo name


# Which index to write. Set INDEX_FILE to put a release candidate into an index
# of its own, so the board manager of an ordinary user never offers it. Called
# without the variable, this is the released index, exactly as before.
INDEX="${INDEX_FILE:-package_${REALAUTHOR}_${REPOSITORY}_index.json}"

# The avrdude the platform depends on has to be one the index actually offers,
# so take the newest entry there rather than a number written down here. Run
# Add_avrdude_release.sh first when a newer avrdude should be used.
AVRDUDE_VERSION=$(jq -r '.packages[].tools[] | select(.name == "avrdude") | .version' \
                  $INDEX | sort -V | tail -1)

if [ -z "$AVRDUDE_VERSION" ]; then
    echo "The index contains no avrdude at all. Run Add_avrdude_release.sh first."
    exit 1
fi
echo "Platform will depend on avrdude ${AVRDUDE_VERSION}"

# Since 3.2.0 the debug tools are not a dependency any more: they come from the
# AvrDebugger package, which the user installs separately and the platform
# finds as runtime.tools.avr-debugger.path.

# A tag may be given as the first argument. Without one we take the latest
# release, as before; with one we take exactly that tag, which is what a
# pre-release needs: the "latest release" of the API never is one.
TAG="$1"

if [ -n "$TAG" ]; then
    DOWNLOAD_URL="https://api.github.com/repos/$AUTHOR/$REPOSITORY/tarball/$TAG"
    DOWNLOADED_FILE="$TAG"
else
    DOWNLOAD_URL=$(curl -s https://api.github.com/repos/$AUTHOR/$REPOSITORY/releases/latest | grep "tarball_url" | awk -F\" '{print $4}')
    DOWNLOADED_FILE=$(echo $DOWNLOAD_URL | awk -F/ '{print $8}')
fi

if [ -z "$DOWNLOADED_FILE" ]; then
    echo "Could not work out which version to package"
    exit 1
fi
echo "Packaging ${DOWNLOADED_FILE}"

# Check whether this exact version is already in the index. The archive name
# has to be matched in full: without the extension, "MiniCore-3.2.0" also
# matches "MiniCore-3.2.0-rc1.tar.bz2", and the final release would be
# refused because one of its own pre-releases is in the index.
if grep -q "${REPOSITORY}-${DOWNLOADED_FILE#"v"}.tar.bz2" $INDEX; then
    echo "Most recent board version is already in the index file. Nothing to do."
    exit 1
fi

# Download file
wget --no-verbose $DOWNLOAD_URL

# Add .tar.bz2 extension to downloaded file
mv $DOWNLOADED_FILE ${DOWNLOADED_FILE}.tar.bz2

# Extract downloaded file and place it in a folder
printf "\nExtracting folder ${DOWNLOADED_FILE}.tar.bz2 to $REPOSITORY-${DOWNLOADED_FILE#"v"}\n"
mkdir -p "$REPOSITORY-${DOWNLOADED_FILE#"v"}" && tar -xzf ${DOWNLOADED_FILE}.tar.bz2 -C "$REPOSITORY-${DOWNLOADED_FILE#"v"}" --strip-components=1
printf "Done!\n"

# Move files out of the avr folder
mv $REPOSITORY-${DOWNLOADED_FILE#"v"}/avr/* $REPOSITORY-${DOWNLOADED_FILE#"v"}

# Delete downloaded file and empty avr folder
rm -rf ${DOWNLOADED_FILE}.tar.bz2 $REPOSITORY-${DOWNLOADED_FILE#"v"}/avr

# Make sure there are no macOS related files added to the arching that's soon to be geneated
command -v dot_clean > /dev/null 2>&1 && dot_clean .

# Compress folder to tar.bz2
printf "\nCompressing folder $REPOSITORY-${DOWNLOADED_FILE#"v"} to $REPOSITORY-${DOWNLOADED_FILE#"v"}.tar.bz2\n"
tar -cjSf $REPOSITORY-${DOWNLOADED_FILE#"v"}.tar.bz2 $REPOSITORY-${DOWNLOADED_FILE#"v"}
printf "Done!\n"

# Get file size on bytes
FILE_SIZE=$(wc -c "$REPOSITORY-${DOWNLOADED_FILE#"v"}.tar.bz2" | awk '{print $1}')

# Get SHA256 hash
SHA256="SHA-256:$(shasum -a 256 "$REPOSITORY-${DOWNLOADED_FILE#"v"}.tar.bz2" | awk '{print $1}')"

# Create Github download URL
URL="https://${AUTHOR}.github.io/${REPOSITORY}/$REPOSITORY-${DOWNLOADED_FILE#"v"}.tar.bz2"

cp "$INDEX" "$INDEX.tmp"

# Add new boards release entry
jq -r                                    \
--arg repository  $REPOSITORY            \
--arg version     ${DOWNLOADED_FILE#"v"} \
--arg url         $URL                   \
--arg checksum    $SHA256                \
--arg file_size   $FILE_SIZE             \
--arg avrdude_ver $AVRDUDE_VERSION       \
--arg file_name   $REPOSITORY-${DOWNLOADED_FILE#"v"}.tar.bz2  \
'.packages[].platforms[.packages[].platforms | length] |= . +
{
  "name": $repository,
  "architecture": "avr",
  "version": $version,
  "category": "Contributed",
  "url": $url,
  "archiveFileName": $file_name,
  "checksum": $checksum,
  "size": $file_size,
  "boards": [
    {"name": "ATmega328/P/PA/A/PB"},
    {"name": "ATmega168/P/PA/A/PB"},
    {"name": "ATmega88/P/PA/A/PB"},
    {"name": "ATmega48/P/PA/A/PB"},
    {"name": "ATmega8"}
  ],
    "toolsDependencies": [
    {
      "packager": "arduino",
      "name": "avr-gcc",
      "version": "7.3.0-atmel3.6.1-arduino7"
    },
    {
      "packager": "MiniCore",
      "name": "avrdude",
      "version": $avrdude_ver
    },
    {
      "packager": "arduino",
      "name": "arduinoOTA",
      "version": "1.3.0"
    }
  ]
}' "$INDEX.tmp" > "$INDEX"

# Remove files that are no longer needed
rm -rf "$REPOSITORY-${DOWNLOADED_FILE#"v"}" "$INDEX.tmp"
