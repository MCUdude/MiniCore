#!/bin/bash

OWNER=felias-fogg  # Github username
AUTHOR=MCUdude
REPOSITORY=MiniCore

AVROCD_TOOLS_VERSION=$(curl -s https://api.github.com/repos/$OWNER/PyAvrOCD/releases/latest | grep "tag_name" |  awk -F\" '{print $4}' | awk -Fv '{print $2}')

if grep -q "avrocd-tools-${AVROCD_TOOLS_VERSION}" package_${AUTHOR}_${REPOSITORY}_index.json; then
    echo "Most recent version of PyAvrOCD is already in the index"
    exit 1
fi

OS_PLATFORM1="Linux_ARMv6"
OS_PLATFORM2="Linux_ARM64"
OS_PLATFORM3="macOS_64bit"
OS_PLATFORM4="Linux_64bit"
OS_PLATFORM5="Linux_32bit"
OS_PLATFORM6="Windows_64bit"
OS_PLATFORM7="macOS_ARM64"
OS_PLATFORM8="Windows_32bit"

HOST1="arm-linux-gnueabihf"
HOST2="aarch64-linux-gnu"
HOST3="x86_64-apple-darwin"
HOST4="x86_64-linux-gnu"
HOST5="i686-linux-gnu"
HOST6="x86_64-mingw32"
HOST7="arm64-apple-darwin"
HOST8="i686-mingw32"

FILE1=avrocd-tools-${AVROCD_TOOLS_VERSION}-${HOST1}.tar.gz
FILE2=avrocd-tools-${AVROCD_TOOLS_VERSION}-${HOST2}.tar.gz
FILE3=avrocd-tools-${AVROCD_TOOLS_VERSION}-${HOST3}.tar.gz
FILE4=avrocd-tools-${AVROCD_TOOLS_VERSION}-${HOST4}.tar.gz
FILE5=avrocd-tools-${AVROCD_TOOLS_VERSION}-${HOST5}.tar.gz
FILE6=avrocd-tools-${AVROCD_TOOLS_VERSION}-${HOST6}.tar.gz
FILE7=avrocd-tools-${AVROCD_TOOLS_VERSION}-${HOST7}.tar.gz
FILE8=avrocd-tools-${AVROCD_TOOLS_VERSION}-${HOST8}.tar.gz


URL1=https://github.com/felias-fogg/PyAvrOCD/releases/download/v${AVROCD_TOOLS_VERSION}/${FILE1}
URL2=https://github.com/felias-fogg/PyAvrOCD/releases/download/v${AVROCD_TOOLS_VERSION}/${FILE2}
URL3=https://github.com/felias-fogg/PyAvrOCD/releases/download/v${AVROCD_TOOLS_VERSION}/${FILE3}
URL4=https://github.com/felias-fogg/PyAvrOCD/releases/download/v${AVROCD_TOOLS_VERSION}/${FILE4}
URL5=https://github.com/felias-fogg/PyAvrOCD/releases/download/v${AVROCD_TOOLS_VERSION}/${FILE5}
URL6=https://github.com/felias-fogg/PyAvrOCD/releases/download/v${AVROCD_TOOLS_VERSION}/${FILE6}
URL7=https://github.com/felias-fogg/PyAvrOCD/releases/download/v${AVROCD_TOOLS_VERSION}/${FILE7}
URL8=https://github.com/felias-fogg/PyAvrOCD/releases/download/v${AVROCD_TOOLS_VERSION}/${FILE8}

# Download files
wget --no-verbose $URL1
wget --no-verbose $URL2
wget --no-verbose $URL3
wget --no-verbose $URL4
wget --no-verbose $URL5
wget --no-verbose $URL6
wget --no-verbose $URL7
wget --no-verbose $URL8

SIZE1=$(wc -c $FILE1 | awk '{print $1}')
SIZE2=$(wc -c $FILE2 | awk '{print $1}')
SIZE3=$(wc -c $FILE3 | awk '{print $1}')
SIZE4=$(wc -c $FILE4 | awk '{print $1}')
SIZE5=$(wc -c $FILE5 | awk '{print $1}')
SIZE6=$(wc -c $FILE6 | awk '{print $1}')
SIZE7=$(wc -c $FILE7 | awk '{print $1}')
SIZE8=$(wc -c $FILE8 | awk '{print $1}')

SHASUM1=$(shasum -a 256 $FILE1 | awk '{print "SHA-256:"$1}')
SHASUM2=$(shasum -a 256 $FILE2 | awk '{print "SHA-256:"$1}')
SHASUM3=$(shasum -a 256 $FILE3 | awk '{print "SHA-256:"$1}')
SHASUM4=$(shasum -a 256 $FILE4 | awk '{print "SHA-256:"$1}')
SHASUM5=$(shasum -a 256 $FILE5 | awk '{print "SHA-256:"$1}')
SHASUM6=$(shasum -a 256 $FILE6 | awk '{print "SHA-256:"$1}')
SHASUM7=$(shasum -a 256 $FILE7 | awk '{print "SHA-256:"$1}')
SHASUM8=$(shasum -a 256 $FILE8 | awk '{print "SHA-256:"$1}')

printf "File1: ${FILE1}, Size: ${SIZE1}, SHA256: ${SHASUM1}, URL1: ${URL1}\n"
printf "File2: ${FILE2}, Size: ${SIZE2}, SHA256: ${SHASUM2}, URL2: ${URL2}\n"
printf "File3: ${FILE3}, Size: ${SIZE3}, SHA256: ${SHASUM3}, URL3: ${URL3}\n"
printf "File4: ${FILE4}, Size: ${SIZE4}, SHA256: ${SHASUM4}, URL4: ${URL4}\n"
printf "File5: ${FILE5}, Size: ${SIZE5}, SHA256: ${SHASUM5}, URL5: ${URL5}\n"
printf "File6: ${FILE6}, Size: ${SIZE6}, SHA256: ${SHASUM6}, URL6: ${URL6}\n"
printf "File7: ${FILE7}, Size: ${SIZE7}, SHA256: ${SHASUM7}, URL7: ${URL7}\n"
printf "File8: ${FILE8}, Size: ${SIZE8}, SHA256: ${SHASUM8}, URL8: ${URL8}\n"

for ((p = 0 ; p < $(jq '.packages | length' package_${AUTHOR}_${REPOSITORY}_index.json); p++)); do
cp "package_${AUTHOR}_${REPOSITORY}_index.json" "package_${AUTHOR}_${REPOSITORY}_index.json.tmp"
jq -r                                  \
--arg ix                   $p \
--arg avrocd_tools_version $AVROCD_TOOLS_VERSION \
--arg os_plaform1          $OS_PLATFORM1 \
--arg os_plaform2          $OS_PLATFORM2 \
--arg os_plaform3          $OS_PLATFORM3 \
--arg os_plaform4          $OS_PLATFORM4 \
--arg os_plaform5          $OS_PLATFORM5 \
--arg os_plaform6          $OS_PLATFORM6 \
--arg os_plaform7          $OS_PLATFORM7 \
--arg os_plaform8          $OS_PLATFORM8 \
--arg host1                $HOST1        \
--arg host2                $HOST2        \
--arg host3                $HOST3        \
--arg host4                $HOST4        \
--arg host5                $HOST5        \
--arg host6                $HOST6        \
--arg host7                $HOST7        \
--arg host8                $HOST8        \
--arg file1                $FILE1        \
--arg file2                $FILE2        \
--arg file3                $FILE3        \
--arg file4                $FILE4        \
--arg file5                $FILE5        \
--arg file6                $FILE6        \
--arg file7                $FILE7        \
--arg file8                $FILE8        \
--arg size1                $SIZE1        \
--arg size2                $SIZE2        \
--arg size3                $SIZE3        \
--arg size4                $SIZE4        \
--arg size5                $SIZE5        \
--arg size6                $SIZE6        \
--arg size7                $SIZE7        \
--arg size8                $SIZE8        \
--arg shasum1              $SHASUM1      \
--arg shasum2              $SHASUM2      \
--arg shasum3              $SHASUM3      \
--arg shasum4              $SHASUM4      \
--arg shasum5              $SHASUM5      \
--arg shasum6              $SHASUM6      \
--arg shasum7              $SHASUM7      \
--arg shasum8              $SHASUM8      \
--arg url1                 $URL1         \
--arg url2                 $URL2         \
--arg url3                 $URL3         \
--arg url4                 $URL4         \
--arg url5                 $URL5         \
--arg url6                 $URL6         \
--arg url7                 $URL7         \
--arg url8                 $URL8         \
'.packages[ $ix | tonumber ].tools[.packages[ $ix | tonumber ].tools | length] |= . +
{
  "name": "avrocd-tools",
  "version": $avrocd_tools_version,
  "systems": [
    {
      "size": $size1,
      "checksum": $shasum1,
      "host": $host1,
      "archiveFileName": $file1,
      "url": $url1
    },
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
      "size": $size5,
      "checksum": $shasum5,
      "host": $host5,
      "archiveFileName": $file5,
      "url": $url5
    },
    {
      "size": $size6,
      "checksum": $shasum6,
      "host": $host6,
      "archiveFileName": $file6,
      "url": $url6
    },
    {
      "size": $size7,
      "checksum": $shasum7,
      "host": $host7,
      "archiveFileName": $file7,
      "url": $url7
    },
    {
      "size": $size8,
      "checksum": $shasum8,
      "host": $host8,
      "archiveFileName": $file8,
      "url": $url8
    }
  ]
}' "package_${AUTHOR}_${REPOSITORY}_index.json.tmp" > "package_${AUTHOR}_${REPOSITORY}_index.json"
rm "package_${AUTHOR}_${REPOSITORY}_index.json.tmp"
done

rm $FILE1
rm $FILE2
rm $FILE3
rm $FILE4
rm $FILE5
rm $FILE6
rm $FILE7
rm $FILE8


