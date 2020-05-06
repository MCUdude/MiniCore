#!/bin/bash

##########################################################
##                                                      ##
## Shell script for generating a boards manager release ##
## Created by MCUdude                                   ##
## Requires wget, jq and a bash environment             ##
##                                                      ##
##########################################################

# Change these to match your repo
AUTHOR=MCUdude       # Github username
REPOSITORY=MiniCore # Github repo name

# Get the download URL for the latest release from Github
DOWNLOAD_URL=$(curl -s https://api.github.com/repos/$AUTHOR/$REPOSITORY/releases/latest | grep "tarball_url" | awk -F\" '{print $4}')

# Download file
wget --no-verbose $DOWNLOAD_URL

# Get filename
DOWNLOADED_FILE=$(echo $DOWNLOAD_URL | awk -F/ '{print $8}')

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

cp "package_${AUTHOR}_${REPOSITORY}_index.json" "package_${AUTHOR}_${REPOSITORY}_index.json.tmp"

# Add new boards release entry
jq -r                                   \
--arg repository $REPOSITORY            \
--arg version    ${DOWNLOADED_FILE#"v"} \
--arg url        $URL                   \
--arg checksum   $SHA256                \
--arg file_size  $FILE_SIZE             \
--arg file_name  $REPOSITORY-${DOWNLOADED_FILE#"v"}.tar.bz2  \
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
  "toolsDependencies": []
}' "package_${AUTHOR}_${REPOSITORY}_index.json.tmp" > "package_${AUTHOR}_${REPOSITORY}_index.json"

# Remove files that's no longer needed
rm -rf "$REPOSITORY-${DOWNLOADED_FILE#"v"}" "package_${AUTHOR}_${REPOSITORY}_index.json.tmp"
