#!/bin/bash

AUTHOR=felias-fogg  # Github username
REALAUTHOR=MCUDude   # Real author name
REPOSITORY=MiniCore # Github repo name

DWTOOLS_VERSION="2.1.4"

OS_PLATFORM1="Linux_ARMv6"
OS_PLATFORM2="Linux_ARM64"
OS_PLATFORM3="macOS_64bit"
OS_PLATFORM4="Linux_64bit"
OS_PLATFORM5="Linux_32bit"
OS_PLATFORM6="Windows_64bit"

HOST1="arm-linux-gnueabihf"
HOST2="aarch64-linux-gnu"
HOST3="x86_64-apple-darwin"
HOST4="x86_64-linux-gnu"
HOST5="i686-linux-gnu"
HOST6="x86_64-mingw32"

FILE1=dw-tools-${DWTOOLS_VERSION}_${HOST1}.tar.bz2
FILE2=dw-tools-${DWTOOLS_VERSION}_${HOST2}.tar.bz2
FILE3=dw-tools-${DWTOOLS_VERSION}_${HOST3}.tar.bz2
FILE4=dw-tools-${DWTOOLS_VERSION}_${HOST4}.tar.bz2
FILE5=dw-tools-${DWTOOLS_VERSION}_${HOST5}.tar.bz2
FILE6=dw-tools-${DWTOOLS_VERSION}_${HOST6}.tar.bz2

URL1=https://felias-fogg.github.io/dw-tools/${FILE1}
URL2=https://felias-fogg.github.io/dw-tools/${FILE2}
URL3=https://felias-fogg.github.io/dw-tools/${FILE3}
URL4=https://felias-fogg.github.io/dw-tools/${FILE4}
URL5=https://felias-fogg.github.io/dw-tools/${FILE5}
URL6=https://felias-fogg.github.io/dw-tools/${FILE6}

# Download files
#wget --no-verbose $URL1
wget --no-verbose $URL2
wget --no-verbose $URL3
wget --no-verbose $URL4
#wget --no-verbose $URL5
wget --no-verbose $URL6

#SIZE1=$(wc -c $FILE1 | awk '{print $1}')
SIZE2=$(wc -c $FILE2 | awk '{print $1}')
SIZE3=$(wc -c $FILE3 | awk '{print $1}')
SIZE4=$(wc -c $FILE4 | awk '{print $1}')
#SIZE5=$(wc -c $FILE5 | awk '{print $1}')
SIZE6=$(wc -c $FILE6 | awk '{print $1}')

#SHASUM1=$(shasum -a 256 $FILE1 | awk '{print "SHA-256:"$1}')
SHASUM2=$(shasum -a 256 $FILE2 | awk '{print "SHA-256:"$1}')
SHASUM3=$(shasum -a 256 $FILE3 | awk '{print "SHA-256:"$1}')
SHASUM4=$(shasum -a 256 $FILE4 | awk '{print "SHA-256:"$1}')
#SHASUM5=$(shasum -a 256 $FILE5 | awk '{print "SHA-256:"$1}')
SHASUM6=$(shasum -a 256 $FILE6 | awk '{print "SHA-256:"$1}')

#printf "File1: ${FILE1}, Size: ${SIZE1}, SHA256: ${SHASUM1}, URL1: ${URL1}\n"
printf "File2: ${FILE2}, Size: ${SIZE2}, SHA256: ${SHASUM2}, URL2: ${URL2}\n"
printf "File3: ${FILE3}, Size: ${SIZE3}, SHA256: ${SHASUM3}, URL3: ${URL3}\n"
printf "File4: ${FILE4}, Size: ${SIZE4}, SHA256: ${SHASUM4}, URL4: ${URL4}\n"
#printf "File5: ${FILE5}, Size: ${SIZE5}, SHA256: ${SHASUM5}, URL5: ${URL5}\n"
printf "File6: ${FILE6}, Size: ${SIZE6}, SHA256: ${SHASUM6}, URL6: ${URL6}\n"

cp "package_${REALAUTHOR}_${REPOSITORY}_index.json" "package_${REALAUTHOR}_${REPOSITORY}_index.json.tmp"

### NOTE: OS platform 5 needs to be added!

jq -r                                  \
--arg dwtools_version $DWTOOLS_VERSION \
--arg os_plaform2 $OS_PLATFORM2 \
--arg os_plaform3 $OS_PLATFORM3 \
--arg os_plaform4 $OS_PLATFORM5 \
--arg os_plaform6 $OS_PLATFORM6 \
--arg host2       $HOST2        \
--arg host3       $HOST3        \
--arg host4       $HOST4        \
--arg host6       $HOST6        \
--arg file2       $FILE2        \
--arg file3       $FILE3        \
--arg file4       $FILE4        \
--arg file6       $FILE6        \
--arg size2       $SIZE2        \
--arg size3       $SIZE3        \
--arg size4       $SIZE4        \
--arg size6       $SIZE6        \
--arg shasum2     $SHASUM2      \
--arg shasum3     $SHASUM3      \
--arg shasum4     $SHASUM4      \
--arg shasum6     $SHASUM6      \
--arg url2        $URL2         \
--arg url3        $URL3         \
--arg url4        $URL4         \
--arg url6        $URL6         \
'.packages[].tools[.packages[].tools | length] |= . +
{
  "name": "dw-tools",
  "version": $dwtools_version,
  "systems": [
    {
      "size": $size2,
      "checksum": $shasum2,
      "host": $host2,
      "archiveFileName": $file2,
      "url": $url2
    },
    {
      "size": $size3,
      "checksum": $shasum3,
      "host": $host3,
      "archiveFileName": $file3,
      "url": $url3
    },
    {
      "size": $size4,
      "checksum": $shasum4,
      "host": $host4,
      "archiveFileName": $file4,
      "url": $url4
    },
    {
      "size": $size6,
      "checksum": $shasum6,
      "host": $host6,
      "archiveFileName": $file6,
      "url": $url6
    }
  ]
}' "package_${REALAUTHOR}_${REPOSITORY}_index.json.tmp" > "package_${REALAUTHOR}_${REPOSITORY}_index.json"

#rm $FILE1
rm $FILE2
rm $FILE3
rm $FILE4
#rm $FILE5
rm $FILE6
rm "package_${REALAUTHOR}_${REPOSITORY}_index.json.tmp"


