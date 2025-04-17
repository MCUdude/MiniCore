#!/bin/bash

AUTHOR=felias-fogg      # Github username
REALAUTHOR=MCUdude
REPOSITORY=MiniCore # Github repo name

AVRDUDE_VERSION="8.0-arduino.1"

OS_PLATFORM1="Linux_ARMv6"
OS_PLATFORM2="Linux_ARM64"
OS_PLATFORM3="macOS_64bit"
OS_PLATFORM4="Linux_64bit"
OS_PLATFORM5="Linux_32bit"
OS_PLATFORM6="Windows_32bit"

HOST1="arm-linux-gnueabihf"
HOST2="aarch64-linux-gnu"
HOST3="x86_64-apple-darwin12"
HOST4="x86_64-linux-gnu"
HOST5="i686-linux-gnu"
HOST6="i686-mingw32"

FILE1=avrdude_${AVRDUDE_VERSION}_${OS_PLATFORM1}.tar.gz
FILE2=avrdude_${AVRDUDE_VERSION}_${OS_PLATFORM2}.tar.gz
FILE3=avrdude_${AVRDUDE_VERSION}_${OS_PLATFORM3}.tar.gz
FILE4=avrdude_${AVRDUDE_VERSION}_${OS_PLATFORM4}.tar.gz
FILE5=avrdude_${AVRDUDE_VERSION}_${OS_PLATFORM5}.tar.gz
FILE6=avrdude_${AVRDUDE_VERSION}_${OS_PLATFORM6}.tar.gz

URL1=http://downloads.arduino.cc/tools/${FILE1}
URL2=http://downloads.arduino.cc/tools/${FILE2}
URL3=http://downloads.arduino.cc/tools/${FILE3}
URL4=http://downloads.arduino.cc/tools/${FILE4}
URL5=http://downloads.arduino.cc/tools/${FILE5}
URL6=http://downloads.arduino.cc/tools/${FILE6}

# Download files
wget --no-verbose $URL1
wget --no-verbose $URL2
wget --no-verbose $URL3
wget --no-verbose $URL4
wget --no-verbose $URL5
wget --no-verbose $URL6

SIZE1=$(wc -c $FILE1 | awk '{print $1}')
SIZE2=$(wc -c $FILE2 | awk '{print $1}')
SIZE3=$(wc -c $FILE3 | awk '{print $1}')
SIZE4=$(wc -c $FILE4 | awk '{print $1}')
SIZE5=$(wc -c $FILE5 | awk '{print $1}')
SIZE6=$(wc -c $FILE6 | awk '{print $1}')

SHASUM1=$(shasum -a 256 $FILE1 | awk '{print "SHA-256:"$1}')
SHASUM2=$(shasum -a 256 $FILE2 | awk '{print "SHA-256:"$1}')
SHASUM3=$(shasum -a 256 $FILE3 | awk '{print "SHA-256:"$1}')
SHASUM4=$(shasum -a 256 $FILE4 | awk '{print "SHA-256:"$1}')
SHASUM5=$(shasum -a 256 $FILE5 | awk '{print "SHA-256:"$1}')
SHASUM6=$(shasum -a 256 $FILE6 | awk '{print "SHA-256:"$1}')

printf "File1: ${FILE1}, Size: ${SIZE1}, SHA256: ${SHASUM1}, URL1: ${URL1}\n"
printf "File2: ${FILE2}, Size: ${SIZE2}, SHA256: ${SHASUM2}, URL2: ${URL2}\n"
printf "File3: ${FILE3}, Size: ${SIZE3}, SHA256: ${SHASUM3}, URL3: ${URL3}\n"
printf "File4: ${FILE4}, Size: ${SIZE4}, SHA256: ${SHASUM4}, URL4: ${URL4}\n"
printf "File5: ${FILE5}, Size: ${SIZE5}, SHA256: ${SHASUM5}, URL5: ${URL5}\n"
printf "File6: ${FILE6}, Size: ${SIZE6}, SHA256: ${SHASUM6}, URL6: ${URL6}\n"

cp "package_${REALAUTHOR}_${REPOSITORY}_index.json" "package_${REALAUTHOR}_${REPOSITORY}_index.json.tmp"

jq -r                                  \
--arg avrdude_version $AVRDUDE_VERSION \
--arg os_plaform1 $OS_PLATFORM1 \
--arg os_plaform2 $OS_PLATFORM2 \
--arg os_plaform3 $OS_PLATFORM3 \
--arg os_plaform4 $OS_PLATFORM5 \
--arg os_plaform5 $OS_PLATFORM6 \
--arg os_plaform6 $OS_PLATFORM6 \
--arg host1       $HOST1        \
--arg host2       $HOST2        \
--arg host3       $HOST3        \
--arg host4       $HOST4        \
--arg host5       $HOST5        \
--arg host6       $HOST6        \
--arg file1       $FILE1        \
--arg file2       $FILE2        \
--arg file3       $FILE3        \
--arg file4       $FILE4        \
--arg file5       $FILE5        \
--arg file6       $FILE6        \
--arg size1       $SIZE1        \
--arg size2       $SIZE2        \
--arg size3       $SIZE3        \
--arg size4       $SIZE4        \
--arg size5       $SIZE5        \
--arg size6       $SIZE6        \
--arg shasum1     $SHASUM1      \
--arg shasum2     $SHASUM2      \
--arg shasum3     $SHASUM3      \
--arg shasum4     $SHASUM4      \
--arg shasum5     $SHASUM5      \
--arg shasum6     $SHASUM6      \
--arg url1        $URL1         \
--arg url2        $URL2         \
--arg url3        $URL3         \
--arg url4        $URL4         \
--arg url5        $URL5         \
--arg url6        $URL6         \
'.packages[].tools[.packages[].tools | length] |= . +
{
  "name": "avrdude",
  "version": $avrdude_version,
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
    }
  ]
}' "package_${REALAUTHOR}_${REPOSITORY}_index.json.tmp" > "package_${REALAUTHOR}_${REPOSITORY}_index.json"

rm $FILE1
rm $FILE2
rm $FILE3
rm $FILE4
rm $FILE5
rm $FILE6
rm "package_${REALAUTHOR}_${REPOSITORY}_index.json.tmp"
