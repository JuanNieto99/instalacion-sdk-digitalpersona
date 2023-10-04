#!/bin/sh

# Check root
if [ "`whoami`" != "root" ]; then
  echo "You must be root to install this product."
  echo ""
  exit
fi

# Determine the current path
curpath=`dirname $0`

echo ""
echo "===================================================================="
echo "=                                                                  ="
echo "= U.are.U SDK for Linux Installation.                              ="
echo "=                                                                  ="
echo "=                                                                  ="
echo "= Hit 'q<Enter>' to quit. Press <Enter> to continue.               ="
echo "=                                                                  ="
echo "===================================================================="
echo ""
echo ""

read REPLY
case $REPLY in 
  [qQ]|[qQ][uU][iI][tT])
    TYPE=1;
    echo "Install aborted.";
    exit 0;
    ;;
  *)
    ;;
esac  

KVER=`uname -r`

if [ "`uname -m`" = "x86_64" ]; then
  ARCH=64
else 
  ARCH=32
fi


TYPE="tar"

BIN=$curpath/installers/setup_${TYPE}_${ARCH}.bin
if [ -f $BIN ]; then
  $BIN
else
  echo "The install package, $BIN, is not found."
  echo ""
  exit 1
fi;

echo "Install finished. "
echo ""
exit 0

